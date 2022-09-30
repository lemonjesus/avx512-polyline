#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "avx512_polyline.h"

double random_coord() {
    return -180.0 + ((double)rand() / RAND_MAX) * (189.0 - -180.0);
}

#define POINTS 4000000
#define ROUNDS 20

int main() {
    double* data = (double*)malloc(sizeof(double)*POINTS*2);
    char* out = (char*)malloc(16*POINTS);

    // fill data with random data
    for(int i = 0; i < POINTS*2; i++) data[i] = random_coord();

    printf("starting test\n");

    clock_t start_time = clock();

    for(int i = 0; i < ROUNDS; i++) encode_polyline(data, POINTS, out);

    double elapsed_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    printf("Encoded %d random points %d times in %lf seconds\n", POINTS, ROUNDS, elapsed_time);
    printf("Effective rate: %.02lf points per second\n", POINTS*ROUNDS/elapsed_time);

    free(data);
    free(out);
}