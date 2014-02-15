CXX := g++
CXXFLAGS := -O2 -Wall -Werror -std=c++0x

LIBS := -lpthread

OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

TARGET := test-mythreadpool

.PHONY: all clean

all: $(TARGET)

test-mythreadpool: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
