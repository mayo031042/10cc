#include "10cc.h"

#define MAX_STACK_SIZE 100

int Stack[MAX_STACK_SIZE];
int StackTop = 0;
int block_nest = 0;

// エラー処理

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void aaa()
{
    fprintf(stderr, "\n\tnow-> %s\n", tokens[token_pos]->str);
}

// 重要な変数の操作

int count()
{
    static int cnt = 2;
    return cnt++;
}

int stack_front()
{
    if (!StackTop)
        error("ループ構文内ではありません");
    return Stack[StackTop];
}

void stack_push(int val)
{
    StackTop++;
    if (MAX_STACK_SIZE <= StackTop)
        error("Stack size overflowed.");
    Stack[StackTop] = val;
}
void stack_pop()
{
    if (!StackTop)
        error("Stack size underflowed.");
    StackTop--;
}

int val_of_block_nest()
{
    return block_nest;
}

void add_block_nest()
{
    block_nest++;
}

// ブロックを抜ける　該当関数のmax_offset を更新
void sub_block_nest()
{
    if (func_pos_ptr->max_offset < culc_offset())
    {
        func_pos_ptr->max_offset = culc_offset();
    }

    block_nest--;
}

int size_of(Type *type)
{
    switch (type->kind)
    {
    case VOID:
    case CHAR:
        return 1;
    case INT:
        return 4;
    case PTR:
        return 8;
    case ARRAY:
        return type->array_size * size_of(type->ptr_to);
    }
}

// 指定されたファイルの内容をuser_input に渡す
void read_file(char *path)
{
    fprintf(stderr, "%s: ", path);

    FILE *test_file_ptr = fopen(path, "r");

    if (NULL == test_file_ptr)
        error("cannot open %s", path);

    // ファイルの長さを調べる
    if (fseek(test_file_ptr, 0, SEEK_END) == -1)
        error("%s: fseek ", path);

    size_t size = ftell(test_file_ptr);
    if (fseek(test_file_ptr, 0, SEEK_SET) == -1)
        error("%s: fseek ", path);

    // ファイル内容を読み込む
    user_input = calloc(1, size + 2);
    fread(user_input, size, 1, test_file_ptr);

    // ファイルが必ず"\n\0"で終わっているようにする
    if (size == 0 || user_input[size - 1] != '\n')
    {
        user_input[size++] = '\n';
    }

    user_input[size] = '\0';
    fclose(test_file_ptr);
}
