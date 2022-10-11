#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avx512_polyline.h"

void basic_test() {
    // given an array of doubles
    double a[] = {
        41.95817, -91.29505,
        40.70428,-90.10852,
        40.83740,-86.94446,
        44.06263,-88.98792,
        43.44365,-93.47034,
        41.60160,-99.56297,
        37.39909,-93.23485,
        40.67493,-83.56688,
        47.60109,-85.32469,
        47.89657,-95.16844,
        44.67892,-107.38524,
        39.32847,-107.91258,
        36.34446,-104.57274,
        34.98784,-93.67430,
        41.00738,-80.57860,
        49.63430,-81.98485,
        51.09189,-93.93797
    };

    const char* expected = "q}a_G`aflPx{sFyvfF__Yk~hRu|tRrbnKr{wBbnjZxwfJ|}dd@txsXw}re@_y~Ryg_z@owgi@hivIwux@lra{@hmsR~aqiAh_t_@z~eB`yeQ_ikSz}gGgroaAcuvc@cw|nAg}ss@`tqG}t{G~q}gA";

    // allocate an appropriate amount of space for the output. I think worst case is 6 characters per double, but I like my allocations on the safe side
    char* out = (char*)calloc(1, 0x100);

    for(int i = 1; i <= 16; i++) {
        memset(out, 0x100, 0);
        encode_polyline(a, i, out);
        if(strncmp(expected, out, strlen(out)) != 0) {
            printf("Basic Test Failed at point %d\n", i);
            printf("  Expected portion of %s\n", expected);
            printf("  Actually got        %s\n", out);
            exit(1);
        }
    }

    encode_polyline(a, 17, out);
    if(strncmp(expected, out, strlen(expected)) != 0) {
        printf("Basic Test Failed at point 17\n");
        printf("  Expected portion of %s\n", expected);
        printf("  Actually got        %s\n", out);
        exit(1);
    }
    free(out);
}

void threaded_test() {
    double a[] = {
        41.95817, -91.29505,
        40.70428,-90.10852,
        40.83740,-86.94446,
        44.06263,-88.98792,
        43.44365,-93.47034,
        41.60160,-99.56297,
        37.39909,-93.23485,
        40.67493,-83.56688,
        47.60109,-85.32469,
        47.89657,-95.16844,
        44.67892,-107.38524,
        39.32847,-107.91258,
        36.34446,-104.57274,
        34.98784,-93.67430,
        41.00738,-80.57860,
        49.63430,-81.98485,
        51.09189,-93.93797
    };

    const char* expected = "q}a_G`aflPx{sFyvfF__Yk~hRu|tRrbnKr{wBbnjZxwfJ|}dd@txsXw}re@_y~Ryg_z@owgi@hivIwux@lra{@hmsR~aqiAh_t_@z~eB`yeQ_ikSz}gGgroaAcuvc@cw|nAg}ss@`tqG}t{G~q}gA";

    // allocate an appropriate amount of space for the output. I think worst case is 6 characters per double, but I like my allocations on the safe side
    char* out = (char*)calloc(1, 0x100);

    encode_polyline_threaded(a, 17, 2, out);
    if(strncmp(expected, out, strlen(expected)) != 0) {
        printf("Threaded Test Failed at point 17\n");
        printf("  Expected portion of %s\n", expected);
        printf("  Actually got        %s\n", out);
        exit(1);
    }
    free(out);
}

void zero_test() {
    double a[2];
    char* out = (char*)calloc(1, 0x100);
    encode_polyline(a, 0, out);
    if(strlen(out) != 0) {
        printf("Zero Length Test Failed\n");
        printf("  Expected length of zero\n");
        printf("  Actually got       %ld\n", strlen(out));
        exit(1);
    }
}

int main() {
    basic_test();
    threaded_test();
    zero_test();
    printf("All test cases passed successfully!\n");
    exit(0);
}
