# AVX512 Polyline

An implementation of Google's Encoded Polyline algorithm in AVX512 because why not.

## What? Why?

I've been looking for an excuse to experiment with SIMD instructions for a while. It occured to me that this could be an interesting problem to solve using AVX512. I also thought it'd be funny if I wrote the least portable polyline implementation ever.

**I haven't extensively tested this and it might fail on some edge cases I'm not aware of. It's fast, but it's not production ready!**

## Requirements
Your processor needs to support the AVX512 instruction set and some of its extensions. My test platform was `icelake-server` - this CPU platform (or possibly `icelake-client`) is the minimum. There is a `skylake` branch that is slower, but it works on Skylake processors. It also uses a BMI instruction because why the heck not?

## Encoding Performance
Running my `benchmark` tool on a dual core Icelake Server processor (`Intel(R) Xeon(R) CPU @ 2.60GHz`) running Ubuntu Server 22.04 I get the following results rather consistently (within a margin of error):

```
Encoded 4000000 random points 20 times in 0.357486 seconds
Effective rate: 223784987.38 points per second
```

## To Do
* Write a decoder

## License
MIT
