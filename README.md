# Tiny Manna

A miniaturized Sandpile Model was used as a subject of experiments on optimization, including compilation, HPC, SIMD, and MIMD improvements.

This is currently a work in progress for the parallel computing elective subject by @inescipullo and @ZimmSebas.

The Original folder only has experiments with flags and a couple of improvements over the code. 

The SIMD folder has an intrinsic approach using 256 bits vector instructions (a pack of 16 shorts).

The MIMD folder has a combination of SIMD with intrinsics and OpenMP parallelization.

## How to run

For a *naive* run, you can use: 

```bash
make
./tiny_manna
```

But for more efficiency, you can specify compilator and flags, something like:

```bash
make CXX=g++ CXXFLAGS="-std=c++17 -O3 -march=native -mavx2"
./tiny_manna
```

See the Makefile for more options

## Original version 

The original code for this project comes from [here](https://github.com/computacionparalela/tiny_manna)
