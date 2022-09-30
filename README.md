# AVX512 Polyline

An implementation of Google's Encoded Polyline algorithm in AVX512 because why not.

## What? Why?

I've been looking for an excuse to experiment with SIMD instructions for a while. It occured to me that this could be an interesting problem to solve using AVX512. I also thought it'd be funny if I wrote the least portable polyline implementation ever.

**I haven't extensively tested this and it might fail on some edge cases I'm not aware of. It's fast, but it's not production ready!**

## Requirements
Your processor needs to support the AVX512 instruction set and some of its extensions. My test platform was `skylake-avx512` - I believe this CPU platform is the minimum.

## Encoding Performance
Running my `benchmark` tool on a single core Skylake Server processor (`Intel(R) Xeon(R) CPU @ 2.00GHz`) running Ubuntu Server 22.04 I get the following results rather consistently (within a margin of error):

```
Encoded 4000000 random points 20 times in 1.406822 seconds
Effective rate: 56865758.43 points per second
```

Using these test parameters, it's consistently able to get ~55 million point pairs processed per second (that's 110 million numbers per second!)

## To Do
* Write a decoder
* Optimize for Icelake instead of Skylake so it's even _less_ portable. We can (theoretically) operate on 8 items at once instead of 2 using 8-bit numbers instead of 32-bit numbers in the second loop. The addition of `_mm512_maskz_compress_epi8` in `avx512_vbmi2` makes this easier. 8-bit variants of `or` and `and` don't exist, though. I haven't put much though into how to get around that.

## License
MIT