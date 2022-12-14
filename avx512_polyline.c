#include <stdio.h>
#include <immintrin.h>
#include <string.h>
#include <stdint.h>

#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

// #define POLYLINE_DEBUG

#ifdef POLYLINE_DEBUG
#define debug(...) printf(__VA_ARGS__)
#define debug_m128i(x) print_m128i(x)
#define debug_m128i16(x) print_m128i16(x)
#define debug_m512(x) print_m512(x)
#define debug_m512i(x) print_m512i(x)
#define debug_m512i8(x) print_m512i8(x)

void print_m128i(__m128i v) {
    char *p = (char *)&v;
    printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
}

void print_m128i16(__m128i v) {
    short *p = (short *)&v;
    printf("%d %d %d %d %d %d %d %d\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}

void print_m512(__m512d v) {
    double *p = (double *)&v;
    printf("%lf %lf %lf %lf %lf %lf %lf %lf\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
}

void print_m512i(__m512i v) {
    int *p = (int *)&v;
    printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
}

void print_m512i8(__m512i v) {
    char *p = (char *)&v;
    printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", 
    p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
    p[16], p[17], p[18], p[19], p[20], p[21], p[22], p[23], p[24], p[25], p[26], p[27], p[28], p[29], p[30], p[31],
    p[32], p[33], p[34], p[35], p[36], p[37], p[38], p[39], p[40], p[41], p[42], p[43], p[44], p[45], p[46], p[47],
    p[48], p[49], p[50], p[51], p[52], p[53], p[54], p[55], p[56], p[57], p[58], p[59], p[60], p[61], p[62], p[63]);
}

#else
#define debug(...)
#define debug_m128i(x)
#define debug_m128i16(x)
#define debug_m512(x)
#define debug_m512i(x)
#define debug_m512i8(x)
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

        __m512i mul = _mm512_inserti32x8(_mm512_setzero_si512(), mul1, 0);
        mul = _mm512_inserti32x8(mul, mul2, 1);
        debug("mul:\t"); debug_m512i(mul);

        // shift left by 1 bit
        __m512i out = _mm512_slli_epi32(mul, 1);
        debug("slli:\t"); debug_m512i(out);

        // make a mask of what original values were negative
        __mmask16 mask = _mm512_cmp_epi32_mask(mul, _mm512_setzero_si512(), _MM_CMPINT_LT);
        debug("mask:\t %X\n", mask);

        // use the mask to negate the values
        out = _mm512_mask_xor_epi32(out, mask, out, _mm512_set1_epi32(-1));
        debug("neg:\t"); debug_m512i(out);

        _mm512_storeu_epi32(ibuf + i, out);
        debug("\n\n");
    }

    uint32_t out_idx = 0;

    for(int i = 0; i < points*2; i+=8) {
        // break the binary value out into 5-bit chunks
        __m512i x = _mm512_set_epi64(__builtin_bswap64(_pdep_u64((uint64_t)ibuf[i+0], 0x1F1F1F1F1F1F)), __builtin_bswap64(_pdep_u64((uint64_t)ibuf[i+1], 0x1F1F1F1F1F1F)),
                                     __builtin_bswap64(_pdep_u64((uint64_t)ibuf[i+2], 0x1F1F1F1F1F1F)), __builtin_bswap64(_pdep_u64((uint64_t)ibuf[i+3], 0x1F1F1F1F1F1F)),
                                     __builtin_bswap64(_pdep_u64((uint64_t)ibuf[i+4], 0x1F1F1F1F1F1F)), __builtin_bswap64(_pdep_u64((uint64_t)ibuf[i+5], 0x1F1F1F1F1F1F)),
                                     __builtin_bswap64(_pdep_u64((uint64_t)ibuf[i+6], 0x1F1F1F1F1F1F)), __builtin_bswap64(_pdep_u64((uint64_t)ibuf[i+7], 0x1F1F1F1F1F1F)));
        debug_m512i8(x);

        // make mask if greater than zero
        __mmask64 mask1 = _mm512_cmpgt_epi8_mask(x, _mm512_setzero_si512());

        // patch the holes in the mask to prevent deletion of valid zeros later
        const __m128i nibble_mask = _mm_set1_epi8(0x0F);
        __m128i v = _mm_set_epi64x(mask1, mask1);
        __m128i t;

        t = _mm_and_si128(nibble_mask, v);
        v = _mm_and_si128(_mm_srli_epi16(v, 4), nibble_mask);
        t = _mm_shuffle_epi8(_mm_set_epi64((__m64)0x0001000200010003, (__m64)0x0001000200010008), t);
        v = _mm_shuffle_epi8(_mm_set_epi64((__m64)0x0405040604050407, (__m64)0x0405040604050408), v);
        v = _mm_min_epu8(v, t);
        v = _mm_maskz_unpackhi_epi8(0x5555, v, v);
        
        debug_m128i16(v);

        __m128i mask = _mm_sllv_epi16(_mm_set1_epi16(-1), v);
        __m128i ormaskv = _mm_slli_epi16(mask, 1);

        mask = _mm_xor_epi32(mask, _mm_set1_epi32(-1));
        ormaskv = _mm_xor_epi32(ormaskv, _mm_set1_epi32(-1));
        mask = _mm_packus_epi16(mask, _mm_setzero_si128());
        ormaskv = _mm_packus_epi16(ormaskv, _mm_setzero_si128());
        mask = _mm_xor_epi32(mask, _mm_set1_epi32(-1));
        ormaskv = _mm_xor_epi32(ormaskv, _mm_set1_epi32(-1));

        mask1 = _mm_extract_epi64(mask, 0);
        __mmask64 ormask = _mm_extract_epi64(ormaskv, 0);
        debug("mask1:\t %lld\n", mask1);

        // or with 0x20 and then reset the last byte in each sequence (due to a lack of _mm512_mask_or_epi8)
        __m512i y = _mm512_or_epi64(x, _mm512_set1_epi8(0x20));
        debug("or\t"); debug_m512i8(y);
        x = _mm512_mask_add_epi8(x, ormask, y, _mm512_setzero_si512());
        debug("or\t"); debug_m512i8(x);

        // add 63 according to all bytes (and zero out everything else)
        x = _mm512_maskz_add_epi8(mask1, x, _mm512_set1_epi8(63));
        debug("add\t"); debug_m512i8(x);

        // permute to get the right order
        x = _mm512_permutexvar_epi8(_mm512_set_epi64(0x0001020304050607, 0x08090A0B0C0D0E0F, 0x1011121314151617, 0x18191A1B1C1D1E1F, 0x2021222324252627, 0x28292A2B2C2D2E2F, 0x3031323334353637, 0x38393A3B3C3D3E3F), x);
        debug("perm\t"); debug_m512i8(x);

        // compress the array to one end
        __mmask64 mask2 = _mm512_cmpgt_epi8_mask(x, _mm512_setzero_si512());
        x = _mm512_maskz_compress_epi8(mask2, x);
        debug("comp\t"); debug_m512i8(x);
        debug("to write: %d\n", _mm_popcnt_u64(mask2));

        _mm512_storeu_epi64(outbuf + out_idx, x);
        out_idx += _mm_popcnt_u64(mask2);

        debug("rendered: %s\n", outbuf);
        debug("\n\n");
    }

    free(ibuf);
}