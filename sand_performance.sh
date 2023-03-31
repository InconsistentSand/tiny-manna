#!/bin/sh
set -e

truncate -s 0 performance.txt

echo "Performance with g++\n" >> performance.txt
echo "Plain g++" >> performance.txt
make perf_file CXX=g++
echo "Cache misses" >> performance.txt
make perf_cache_file CXX=g++
echo "g++ -O1" >> performance.txt
make perf_file CXX=g++ LDFLAGS=-O1
echo "g++ -O2" >> performance.txt
make perf_file CXX=g++ LDFLAGS=-O2
echo "g++ -O3" >> performance.txt
make perf_file CXX=g++ LDFLAGS=-O3
echo "g++ -Ofast" >> performance.txt
make perf_file CXX=g++ LDFLAGS=-Ofast

echo "Performance with clang++\n" >> performance.txt
echo "Plain clang++" >> performance.txt
make perf_file CXX=clang++
echo "Cache misses" >> performance.txt
make perf_cache_file CXX=clang++
echo "clang++ -O1" >> performance.txt
make perf_file CXX=clang++ LDFLAGS=-O1
echo "clang++ -O2" >> performance.txt
make perf_file CXX=clang++ LDFLAGS=-O2
echo "clang++ -O3" >> performance.txt
make perf_file CXX=clang++ LDFLAGS=-O3
echo "clang++ -Ofast" >> performance.txt
make perf_file CXX=clang++ LDFLAGS=-Ofast
