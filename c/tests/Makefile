CC := gcc

ifeq ($(debug), y)
    CFLAGS := -g
else
    CFLAGS := -O2 -DNDEBUG
endif

CFLAGS := $(CFLAGS) -Wall -Werror

ifndef DEPSDIR
    DEPSDIR := $(shell pwd)/../../..
endif

INCLUDE := -I$(DEPSDIR) -I..
LIBS := ../libthreadpool.a -lpthread

OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

TARGET := test_threadpool

.PHONY: all clean pre-process post-clean

all: $(TARGET)

$(OBJS): pre-process

pre-process:
	$(MAKE) DEPSDIR=$(DEPSDIR) -C ..

post-clean:
	$(MAKE) clean DEPSDIR=$(DEPSDIR) -C ..

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean: | post-clean
	rm -f $(TARGET) $(OBJS)
