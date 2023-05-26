# Tiny Manna

A miniaturized Sandpile Model, used as an subject of experiments to work on compilation, HPC, SIMD, MIMD improvements.

This is currently a work in progress for parallel computation subject with @inescipullo and @ZimmSebas

The Original folder just has experiments with flags a couple of improvements over the code. 

The SIMD folder has an intrinsic approach with 256bits vector instructions (using a pack of 16 shorts)

## How to run

For a *naive* run, you can use: 

```bash
make
./tiny_manna
```

But for more efficiency, you can specify compilator and flags like:

```bash
make CXX=g++ CXXFLAGS="-std=c++17 -O3 -march=native"
./tiny_manna
```

See the Makefile for more options

## Original version 

The original code of this project is from [here](https://github.com/computacionparalela/tiny_manna)