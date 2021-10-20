CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
SRCS:=$(filter-out experiment.c,$(SRCS))
OBJS=$(SRCS:.c=.o)

ex: experiment.s
	./ex.sh
experiment.s:
	$(CC) -s experiment.c

10cc: $(OBJS)
	$(CC) -o 10cc $(OBJS) $(LDFLAGS)

$(OBJS): 10cc.h

test: 10cc
	./test.sh

clean:
	rm -f 10cc *.o *~ tmp*

.PHONY: test clean
