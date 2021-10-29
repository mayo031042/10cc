CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
SRCS:=$(filter-out optimize.c,$(SRCS))
OBJS=$(SRCS:.c=.o)

10cc: $(OBJS) optimize.c
	$(CC) -o 10cc $(OBJS) $(LDFLAGS)
	$(CC) -o optimize optimize.c

$(OBJS): 10cc.h

test: 10cc optimize
	./test.sh

# 編集されたtmp.s をtmp2.s において実行
s: tmp2.s
	gcc -o tmp2 tmp2.s
	./tmp2

clean:
	rm -f 10cc *.o *~ tmp* optimize

.PHONY: test clean
