CC := gcc
CXX := g++
CFLAGS := -O2 -Wall -Werror
CXXFLAGS := $(CFLAGS) -std=c++0x
LIBS := -lpthread

TARGET := test-mythreadpool test-threadpool

.PHONY: all clean

all: $(TARGET)

test-mythreadpool: test-mythreadpool.o mythreadpool.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

test-threadpool: test-threadpool.o threadpool.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o
