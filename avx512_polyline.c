#include <stdio.h>
#include <immintrin.h>
#include <string.h>
#include <stdint.h>

// #define POLYLINE_DEBUG

#ifdef POLYLINE_DEBUG
#define debug(...) printf(__VA_ARGS__)
#define debug_m128i(x) print_m128i(x)
#define debug_m512(x) print_m512(x)
#define debug_m512i(x) print_m512i(x)
#else
#define debug(...)
#define debug_m128i(x)
#define debug_m512(x)
#define debug_m512i(x)
#endif

#define FIVE(x, i) (x >> (i*5)) & 0b00011111
#define FIVE_BIT_CHUNKS(x) FIVE(x, 0), FIVE(x, 1), FIVE(x, 2), FIVE(x, 3), FIVE(x, 4), FIVE(x, 5)
#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

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

void encode_polyline(double* a, int points, char* outbuf) {
    double* buf = (double*)malloc(sizeof(double)*points*2);
    uint32_t* ibuf = (uint32_t*)malloc(sizeof(uint32_t)*ROUND_UP(points*2, 32));
    
    // convert coordinates to deltas
    buf[0] = a[0];
    buf[1] = a[1];
    
    for (int i = 2; i < points * 2; i++) {
        buf[i] = a[i] - a[i-2];
    }

    for(int i = 0; i < points*2; i+=16) {
        __m512d in1 = _mm512_loadu_pd(buf+i);
        __m512d in2 = _mm512_loadu_pd(buf+i+8);
        debug("in1:\t"); debug_m512(in1);
        debug("in2:\t"); debug_m512(in2);

        // multiply in by 1e5
        __m256i mul1 = _mm512_cvtpd_epi32(_mm512_mul_pd(in1, _mm512_set1_pd(1e5)));
        __m256i mul2 = _mm512_cvtpd_epi32(_mm512_mul_pd(in2, _mm512_set1_pd(1e5)));

        __m512i mul = _mm512_maskz_broadcast_i64x4(0xF, mul1);
        mul = _mm512_mask_broadcast_i64x4(mul, 0xF0, mul2);
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

        _mm512_storeu_epi32(ibuf+i, out);
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

        // or with 0x20 according to mask2
        x = _mm512_mask_or_epi32(x, mask1 << 1, x, _mm512_set1_epi32(0x20));
        debug("or\t"); debug_m512i(x);

        // add 63 according to mask1 
        x = _mm512_mask_add_epi32(x, mask1, x, _mm512_set1_epi32(63));
        debug("add\t"); debug_m512i(x);
        
        // AND with zero according to mask3
        x = _mm512_mask_and_epi32(x, ~mask1, x, _mm512_setzero_epi32());
        debug("and\t"); debug_m512i(x);

        // permute to get the right order
        x = _mm512_permutexvar_epi32(_mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15), x);
        debug("perm\t"); debug_m512i(x);

        // compress the array to one end
        __mmask16 mask2 = _mm512_cmpgt_epi32_mask(x, _mm512_setzero_epi32());
        x = _mm512_maskz_compress_epi32(mask2, x);
        debug("comp\t"); debug_m512i(x);
        debug("to write: %d\n", __builtin_popcount(mask2));

        // convert everything into 8-bit numbers
        __m128i y = _mm512_maskz_cvtepi32_epi8(0xFFFF, x);
        debug_m128i(y);

        _mm_mask_storeu_epi8(outbuf+out_idx, mask2, y);
        out_idx += __builtin_popcount(mask2);

        debug("\n\n");
    }

    free(buf);
    free(ibuf);
}