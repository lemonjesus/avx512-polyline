all: benchmark example test

benchmark: avx512_polyline.o benchmark.c
	gcc -march=native -O2 avx512_polyline.o benchmark.c -o benchmark

example: avx512_polyline.o example.c
	gcc -march=native -O2 -g avx512_polyline.o example.c -o example

test: avx512_polyline.o test.c
	gcc -march=native -O2 avx512_polyline.o test.c -o test

avx512_polyline.o: avx512_polyline.c avx512_polyline.h
	gcc -march=native -O2 -c avx512_polyline.c -o avx512_polyline.o

clean:
	rm *.o benchmark example test