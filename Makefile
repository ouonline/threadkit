CXX := g++
CXXFLAGS := -Wall -g -std=c++0x
LIBS := -lpthread

TARGET := test-thread-pool

.PHONY: all clean

all: $(TARGET)

test-thread-pool: test.o threadpool.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(TARGET) *.o
