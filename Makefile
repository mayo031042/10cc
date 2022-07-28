CC = gcc
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
# 10cc: $(OBJS)
# 	$(CC) -o $(TARDIR)/$@ $(OBJS) $(LDFLAGS)

# $(OBJS): $(*F:=.c)
# 	@echo $(*F)
# 	@echo $(*F:=.c)
# 	$(CC) -c $(CFLAGS) -o $@ $(addsuffix .c,$(basename $(notdir $@))) 


# # ディレクトリ名と拡張子を取り除いて.c 拡張子を新しくつける 他にもやり方はあるはず、、、
# %.o: $(addsuffix .c,$(basename $(notdir )))
# 	echo $^
# 	$(CC) -c $(CFLAGS) -o $@ $(addsuffix .c,$(basename $(notdir $@))) 

# 以下を変数等用いて書きたい
10cc: obj/main.o obj/utils.o obj/tokenize.o obj/tokenize_func.o obj/parse.o obj/parse_func.o obj/node.o obj/lvar.o obj/func.o obj/type.o obj/codegen.o obj/codegen_func.o
	$(CC) -o $(TARDIR)/$@ $^ $(LDFLAGS)

# 問題としては obj/ の接頭語が正しく各単語に展開されない 
# 接頭語付きで展開されたものを 別の接頭語を付けて依存ファイルを指定したい 
# フォルダが３つ位できるので結局いちいち書くほうが簡単になりそうな。。。？
# 依存ファイルを配列で渡すと １つ書き換えるとすべて再生製され 分割の意味がない 

$(OBJS): $(HEADS)

obj/main.o: main.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/utils.o: utils.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/tokenize.o: tokenize.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/tokenize_func.o: tokenize_func.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/parse.o: parse.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/parse_func.o: parse_func.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/node.o: node.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/lvar.o: lvar.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/func.o: func.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/type.o: type.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/codegen.o: codegen.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/codegen_func.o: codegen_func.c
	$(CC) -c $(CFLAGS) -o $@ $<

# obj/test.o: test.c
# 	$(CC) -c $(CFLAGS) -o $@ $<

test: 10cc 
	./test.sh

# 編集されたtmp.s を実行
s: tmp/tmp.s
	gcc tmp/tmp.s -o tmp/tmp
	./tmp/tmp

opt: optimize.c 
	$(CC) -c $(CFLAGS) -o obj/opt $<

srt: create_sort.cpp
	g++ -o bin/srt create_sort.cpp
	./bin/srt

clean:
	rm -f ./bin/10cc *.o *~ tmp/* optimize ./obj/* srt

.PHONY: test clean
