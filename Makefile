CC = cc
CFLAGS=-std=c11 -g -static

# ディレクトリを指定
OBJDIR=./obj
TARDIR=./bin

# カレントディレクトリ内から 末尾が.c であるようなすべてのファイルを配列で持つ？
SRCS=$(wildcard *.c)
# 上記のような配列のすべての要素に対して 共通の接頭語を据える
OBJS=$(addprefix $(OBJDIR)/,$(SRCS:.c=.o))
HEADS=$(wildcard *.h)

# 10cc はカレントディレクトリに存在するすべてのCファイルと同名の./obj/*.o ファイルに依存している
10cc: $(OBJS)
	$(CC) -o $(TARDIR)/$@ $(OBJS) $(LDFLAGS)

# ディレクトリ名と拡張子を取り除いて.c 拡張子を新しくつける　他にもやり方はあるはず、、、
%.o: 
	$(CC) -c $(CFLAGS) -o $@ $(addsuffix .c,$(basename $(notdir $@))) 

test: 10cc 
	./testsh/testType.sh

# 編集されたtmp.s を実行
s: tmp.s
	gcc -o tmp tmp.s
	./tmp

clean:
	rm -f ./bin/10cc *.o *~ tmp* optimize ./obj/*

.PHONY: test clean
