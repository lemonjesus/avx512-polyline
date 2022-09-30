#include <stdio.h>
#include <stdlib.h>

#include "avx512_polyline.h"

int main() {
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

    // allocate an appropriate amount of space for the output. I think worst case is 6 characters per double, but I like my allocations on the safe side
    char* out = (char*)calloc(1, 0x100);
    encode_polyline(a, 17, out);
    printf("%s\n", out); // expect q}a_G`aflPx{sFyvfF__Yk~hRu|tRrbnKr{wBbnjZxwfJ|}dd@txsXw}re@_y~Ryg_z@owgi@hivIwux@lra{@hmsR~aqiAh_t_@z~eB`yeQ_ikSz}gGgroaAcuvc@cw|nAg}ss@`tqG}t{G~q}gA
    free(out);
}