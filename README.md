# AVX512 Polyline

An implementation of Google's Encoded Polyline algorithm in AVX512 because why not. This is the Skylake variant, which used to be the main version but I picked the Icelake version to be main because I wanted the fastest and least portable implementation possible for the memes.

## What? Why?

I've been looking for an excuse to experiment with SIMD instructions for a while. It occured to me that this could be an interesting problem to solve using AVX512. I also thought it'd be funny if I wrote the least portable polyline implementation ever.

**I haven't extensively tested this and it might fail on some edge cases I'm not aware of. It's fast, but it's not production ready!**

## Requirements
Your processor needs to support the AVX512 instruction set and some of its extensions. My test platform was `skylake-avx512` - I believe this CPU platform is the minimum.

## Encoding Performance
Running my `benchmark` tool on a dual core Icelake Server processor running Ubuntu Server 22.04 I get the following results rather consistently (within a margin of error):

```
Encoded 4000000 random points 20 times in 1.406822 seconds
Effective rate: 79884806.11 points per second
```

## To Do
* Write a decoder

## License
MIT
