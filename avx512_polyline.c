#include <stdio.h>
#include <immintrin.h>
#include <string.h>
#include <stdint.h>

#define FIVE(x, i) (x >> (i*5)) & 0b00011111
#define FIVE_BIT_CHUNKS(x) FIVE(x, 0), FIVE(x, 1), FIVE(x, 2), FIVE(x, 3), FIVE(x, 4), FIVE(x, 5)
#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

// #define POLYLINE_DEBUG

#ifdef POLYLINE_DEBUG
#define debug(...) printf(__VA_ARGS__)
#define debug_m128i(x) print_m128i(x)
#define debug_m512(x) print_m512(x)
#define debug_m512i(x) print_m512i(x)

void print_m128i(__m128i v) {
    char *p = (char *)&v;
    printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
}

void print_m512(__m512d v) {
    double *p = (double *)&v;
    printf("%lf %lf %lf %lf %lf %lf %lf %lf\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}

void print_m512i(__m512i v) {
    int *p = (int *)&v;
    printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
}

#else
#define debug(...)
#define debug_m128i(x)
#define debug_m512(x)
#define debug_m512i(x)
#endif

void encode_polyline(double* a, int points, char* outbuf) {
    uint32_t *ibuf = (uint32_t*)malloc(sizeof(uint32_t)*ROUND_UP(points*2, 32));
    
    for(int i = 0; i < points*2; i+=16) {
        __mmask8 loadm1 = ((i+8-points*2) < 0) ? 0xFF : (0xFF >> (i+8-points*2)) & 0xFF;
        __mmask8 loadm2 = ((i+16-points*2) < 0) ? 0xFF : (0xFF >> (i+16-points*2)) & 0xFF;
        __m512d in1 = _mm512_maskz_loadu_pd(loadm1, a+i);
        __m512d in2 = _mm512_maskz_loadu_pd(loadm2, a+i+8);

        // calculate deltas
        __m512d sub1 = _mm512_maskz_permutexvar_pd(0xFC, _mm512_set_epi64(5, 4, 3, 2, 1, 0, 0, 0), in1);
        __m512d sub2 = _mm512_maskz_permutexvar_pd(0xFC, _mm512_set_epi64(5, 4, 3, 2, 1, 0, 0, 0), in2);
        if(i != 0) 
            sub1 = _mm512_maskz_add_pd(loadm1, sub1, _mm512_set_pd(0, 0, 0, 0, 0, 0, a[i-1], a[i-2]));
        else
            sub1 = _mm512_maskz_add_pd(loadm1, sub1, _mm512_setzero_pd());

        sub2 = _mm512_maskz_add_pd(loadm2, sub2, _mm512_set_pd(0, 0, 0, 0, 0, 0, a[i+7], a[i+6]));
        debug("sub1:\t"); debug_m512(sub1);
        debug("sub2:\t"); debug_m512(sub2);

        in1 = _mm512_sub_pd(in1, sub1);
        in2 = _mm512_sub_pd(in2, sub2);

        debug("in1:\t"); debug_m512(in1);
        debug("in2:\t"); debug_m512(in2);

        // multiply in by 1e5
        __m256i mul1 = _mm512_cvtpd_epi32(_mm512_mul_pd(in1, _mm512_set1_pd(1e5)));
        __m256i mul2 = _mm512_cvtpd_epi32(_mm512_mul_pd(in2, _mm512_set1_pd(1e5)));

        __m512i mul = _mm512_inserti32x8(_mm512_setzero_epi32(), mul1, 0);
        mul = _mm512_inserti32x8(mul, mul2, 1);
        debug("mul:\t"); debug_m512i(mul);

        // shift left by 1 bit
        __m512i out = _mm512_slli_epi32(mul, 1);
        debug("slli:\t"); debug_m512i(out);

        // make a mask of what original values were negative
        __mmask16 mask = _mm512_cmp_epi32_mask(mul, _mm512_setzero_epi32(), _MM_CMPINT_LT);
        debug("mask:\t %X\n", mask);

        // use the mask to negate the values
        out = _mm512_mask_xor_epi32(out, mask, out, _mm512_set1_epi32(-1));
        debug("neg:\t"); debug_m512i(out);

        _mm512_storeu_epi32(ibuf + i, out);
        debug("\n\n");
    }

    uint32_t out_idx = 0;

    for(int i = 0; i < points*2; i+=2) {
        // break the binary value out into 5-bit chunks
        __m512i x = _mm512_set_epi32(FIVE_BIT_CHUNKS(ibuf[i]), 0, 0, FIVE_BIT_CHUNKS(ibuf[i+1]), 0, 0);
        debug_m512i(x);

        // make mask if greater than zero
        __mmask16 mask1 = _mm512_cmpgt_epi32_mask(x, _mm512_setzero_epi32());

        // fix mask1 by patching holes in the 5-bit chunk masks
        uint8_t top = 0xFF << __builtin_ctz(mask1 >> 8);
        uint8_t bottom = 0xFF << __builtin_ctz(mask1 & 0xFF);
        mask1 = ((uint16_t)top << 8) | bottom;
        debug("mask1:\t %X\n", mask1);

        // or with 0x20 according to mask1 shifted (all but last byte in sequence)
        x = _mm512_mask_or_epi32(x, mask1 << 1, x, _mm512_set1_epi32(0x20));
        debug("or\t"); debug_m512i(x);

        // add 63 according to all bytes (and zero out everything else)
        x = _mm512_maskz_add_epi32(mask1, x, _mm512_set1_epi32(63));
        debug("add\t"); debug_m512i(x);

        // permute to get the right order
        x = _mm512_permutexvar_epi32(_mm512_set_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15), x);
        debug("perm\t"); debug_m512i(x);

        // compress the array to one end
        __mmask16 mask2 = _mm512_cmpgt_epi32_mask(x, _mm512_setzero_epi32());
        x = _mm512_maskz_compress_epi32(mask2, x);
        debug("comp\t"); debug_m512i(x);
        debug("to write: %d\n", __builtin_popcount(mask2));

        // convert everything into 8-bit numbers
        __m128i y = _mm512_maskz_cvtepi32_epi8(0xFFFF, x);
        debug("write:\t"); debug_m128i(y);
        debug("mask2:\t %X\n", mask2);

        _mm_storeu_epi8(outbuf + out_idx, y);
        out_idx += __builtin_popcount(mask2);

        debug("rendered: %s\n", outbuf);
        debug("\n\n");
    }

    free(ibuf);
}