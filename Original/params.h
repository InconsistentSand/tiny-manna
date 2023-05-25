#pragma once

#include <ctime>

// number of sites
#ifndef N
#define N 32000
#endif

// number of sites
#ifndef DENSITY
#define DENSITY 0.8924
#endif

// number of temporal steps
#ifndef NSTEPS
#define NSTEPS 1000000
#endif

#ifndef SEED
#define SEED time(nullptr)
#endif

#define METRIC
