CXX := g++
CXXFLAGS := -O2 -Wall -Werror -std=c++0x
LIBS := -lpthread

TARGET := test-mythreadpool

.PHONY: all clean

all: $(TARGET)

test-mythreadpool: test.o mythreadpool.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(TARGET) *.o
