CC := gcc

ifeq ($(release), y)
    CFLAGS := -O2 -DNDEBUG
else
    CFLAGS := -g
endif

CFLAGS := $(CFLAGS) -Wall -Werror

LIBS := -lpthread

OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

TARGET := test_threadpool

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
