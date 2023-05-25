set -e

truncate -s 0 performance.txt

echo "Performance with g++\n" >> performance.txt
echo "g++ -O3" >> performance.txt
make perf_file CXX=g++ CXXFLAGS="-std=c++17 -O3 -march=native" >> performance.txt

echo "Performance with clang++\n" >> performance.txt
echo "clang++ -O3" >> performance.txt
make perf_file CXX=clang++ CXXFLAGS="-std=c++17 -O3 -march=native" >> performance.txt
