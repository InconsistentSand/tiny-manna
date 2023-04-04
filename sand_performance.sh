#!/bin/sh
set -e

truncate -s 0 performance.txt

echo "Performance with g++\n" >> performance.txt
echo "Plain g++" >> performance.txt
make perf_file CXX=g++ >> performance.txt
echo "g++ -O1" >> performance.txt
make perf_file CXX=g++ LDFLAGS="-O1 -march=native" >> performance.txt
echo "g++ -O2" >> performance.txt
make perf_file CXX=g++ LDFLAGS="-O2 -march=native" >> performance.txt
echo "g++ -O3" >> performance.txt
make perf_file CXX=g++ LDFLAGS="-O3 -march=native" >> performance.txt
echo "g++ -Ofast" >> performance.txt
make perf_file CXX=g++ LDFLAGS="-Ofast -march=native" >> performance.txt

echo "Performance with clang++\n" >> performance.txt
echo "Plain clang++" >> performance.txt
make perf_file CXX=clang++ >> performance.txt
echo "clang++ -O1" >> performance.txt
make perf_file CXX=clang++ LDFLAGS="-O1 -march=native" >> performance.txt
echo "clang++ -O2" >> performance.txt
make perf_file CXX=clang++ LDFLAGS="-O2 -march=native" >> performance.txt
echo "clang++ -O3" >> performance.txt
make perf_file CXX=clang++ LDFLAGS="-O3 -march=native" >> performance.txt
echo "clang++ -Ofast" >> performance.txt
make perf_file CXX=clang++ LDFLAGS="-Ofast -march=native" >> performance.txt
