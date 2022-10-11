all: benchmark example test threaded_benchmark

benchmark: avx512_polyline.o benchmark.c
	gcc -march=native -O2 avx512_polyline.o benchmark.c -o benchmark -lpthread

example: avx512_polyline.o example.c
	gcc -march=native -O2 -g avx512_polyline.o example.c -o example -lpthread

test: avx512_polyline.o test.c
	gcc -march=native -O2 avx512_polyline.o test.c -o test -lpthread

threaded_benchmark: avx512_polyline.o threaded_benchmark.c
	gcc -march=native -O2 avx512_polyline.o threaded_benchmark.c -o threaded_benchmark -lpthread -g

avx512_polyline.o: avx512_polyline.c avx512_polyline.h
	gcc -march=native -O2 -c avx512_polyline.c -o avx512_polyline.o -lpthread -g

clean:
	rm *.o benchmark example test threaded_benchmark
