CXX := g++
AR := ar

ifeq ($(debug), y)
    CXXFLAGS := -g
else
    CXXFLAGS := -O2 -DNDEBUG
endif

CXXFLAGS := $(CXXFLAGS) -Wall -Werror -Wextra -fPIC

LIBS := -lpthread

OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

TARGET := libthreadpool.a libthreadpool.so

.PHONY: all clean

all: $(TARGET)

libthreadpool.so: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -shared $(LIBS)

libthreadpool.a: $(OBJS)
	$(AR) -rc $@ $^

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
