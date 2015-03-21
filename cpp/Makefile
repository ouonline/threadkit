CXX := g++

ifeq ($(release), y)
    CXXFLAGS := -O2 -DNDEBUG
else
    CXXFLAGS := -g
endif

CXXFLAGS := $(CXXFLAGS) -Wall -Werror -std=c++11

LIBS := -lpthread

OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

TARGET := test_mythreadpool

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
