# AVX512-Polyline

An implementation of Google's Encoded Polyline algorithm in AVX512 because why not.

## What? Why?

I've been looking for an excuse to experiment with SIMD instructions for a while. It occured to me that this could be an interesting problem to solve using AVX512. I also thought it'd be funny if I wrote the least portable polyline implementation ever.

## Requirements
Your processor only needs to support the AVX512 Fundamental instruction set (`/proc/cpuinfo` lists this as `avx512f`). Further optimizations (`-O2`) will benefit from newer platforms. For example, my test platform was `skylake-avx512` and with `-O2` it wrote a `vpternlogd` which requires `avx512vl`.

## Encoding Performance
Running my `benchmark` tool on a single core Skylake Server processor (`Intel(R) Xeon(R) CPU @ 2.00GHz`) running Ubuntu Server 22.04 I get the following results rather consistently (within a margin of error):

```
Encoded 4000000 random points 20 times in 4.115627 seconds
Effective rate: 19438107.49 points per second
```

## To Do
* Write a decoder
* Further optimizations:
  * The first step - converting points to deltas - can probably be vectorized.
