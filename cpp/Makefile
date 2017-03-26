CXX := g++

ifeq ($(debug), y)
    CXXFLAGS := -g
else
    CXXFLAGS := -O2 -DNDEBUG
endif

CXXFLAGS := $(CXXFLAGS) -Wall -Werror -std=c++0x

LIBS := -lpthread

OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

TARGET := test

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
