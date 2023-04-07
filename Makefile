# Compiler
CXX = g++

# Flags
CXXFLAGS = -Wall -Wextra -std=c++17
CPPFLAGS =
LDFLAGS =

# Binary file
TARGET = tiny_manna

# Files
SOURCES = tiny_manna.cpp
OBJS = $(patsubst %.cpp, %.o, $(SOURCES))

# Rules
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

perf: $(TARGET)
	perf stat -r 13 -e cache-references,cache-misses,instructions,cycles,task-clock ./$(TARGET)

force: clean $(TARGET)

clang: 
	make $(TARGET) CXX=clang++

perf_file: force
	perf stat -r 13 -o performance.txt --append -e cache-references,cache-misses,instructions,cycles,task-clock ./$(TARGET)

.PHONY : clean perf perf_cache force clang perf_file perf_cache_file
