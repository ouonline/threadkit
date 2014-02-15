CC := gcc
CFLAGS := -O2 -Wall -Werror

LIBS := -lpthread

OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

TARGET := test-threadpool

.PHONY: all clean

all: $(TARGET)

test-threadpool: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
