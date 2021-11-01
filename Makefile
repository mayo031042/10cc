CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
SRCS:=$(filter-out optimize.c,$(SRCS))
OBJS=$(SRCS:.c=.o)

10cc: $(OBJS) optimize.c
	$(CC) -o 10cc $(OBJS) $(LDFLAGS)
	$(CC) -o optimize optimize.c

$(OBJS): 10cc.h tokenize.h

testculc: 10cc optimize
	./testCulc.sh

# 一度無視
testfunc: 10cc optimize
	./testFunc.sh

tmp.s: 
	touch tmp.s

# 編集されたtmp.s を実行
s: tmp.s
	gcc -o tmp tmp.s
	./tmp

clean:
	rm -f 10cc *.o *~ tmp* optimize

.PHONY: test clean
