# Compiler
CXX = g++

# Flags
CXXFLAGS = -Wall -Wextra -std=c++14
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
	perf stat ./$(TARGET)

perf_cache: $(TARGET)
	perf stat -e cache-references,cache-misses ./$(TARGET)

force: clean $(TARGET)

clang: 
	make $(TARGET) CXX=clang++

.PHONY : perf perf_cache force clang