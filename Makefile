CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
SRCS:=$(filter-out optimize.c,$(SRCS))
OBJS=$(SRCS:.c=.o)

10cc: $(OBJS) optimize.c
	$(CC) -o 10cc $(OBJS) $(LDFLAGS)
	$(CC) -o optimize optimize.c

$(OBJS): 10cc.h

test: 10cc
	./test.sh

clean:
	rm -f 10cc *.o *~ tmp*

.PHONY: test clean
