CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
HEADS=$(wildcard *.h)
# SRCS:=$(filter-out optimize.c,$(SRCS))
OBJS=$(SRCS:.c=.o)
CC = cc
TARDIR=bin
OBJROOT=obj
# VPATH = /home/mayo/Documents/codes/cpp/10cc/codegen

10cc: $(OBJS) 
	$(CC) -o $(TARDIR)/$@ $(OBJS) $(LDFLAGS)

$(OBJS): $(HEADS)

testculc: 10cc 
	./testCulc.sh

testfunc: 10cc
	./testFunc.sh

# 編集されたtmp.s を実行
s: tmp.s
	gcc -o tmp tmp.s
	./tmp

clean:
	rm -f 10cc *.o *~ tmp* optimize

.PHONY: test clean
