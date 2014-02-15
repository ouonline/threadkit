CC := gcc
CFLAGS := -O2 -Wall -Werror

INCLUDE := -I../../kernel-utils
LIBS := -lpthread

OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

TARGET := test-threadpool

.PHONY: all clean

all: $(TARGET)

test-threadpool: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
