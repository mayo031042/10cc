CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
SRCS:=$(filter-out optimize.c,$(SRCS))
OBJS=$(SRCS:.c=.o)

10cc: $(OBJS) optimize.c
	$(CC) -o 10cc $(OBJS) $(LDFLAGS)
	$(CC) -o optimize optimize.c

$(OBJS): 10cc.h

test1: 10cc optimize
	./test1.sh

test2: 10cc optimize
	./test2.sh


tmp2.s: 
	touch tmp2.s

# 編集されたtmp.s をtmp2.s において実行
s: tmp2.s
	gcc -o tmp2 tmp2.s
	./tmp2

clean:
	rm -f 10cc *.o *~ tmp* optimize

.PHONY: test clean
