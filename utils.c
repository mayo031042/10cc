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

void sub_block_nest()
{
    block_nest--;
}

// 暗黙的なキャスト

// 左右辺を持つnode に対してどちらのnode のサイズが大きいかを返す
int cmp_node_size(Node *node)
{
    if (node->lhs == NULL || node->rhs == NULL)
    {
        error("終端node を展開しています");
    }

    int l_sz = size_of_node(node->lhs);
    int r_sz = size_of_node(node->rhs);

    if (l_sz < r_sz)
    {
        return -1;
    }
    else if (l_sz == r_sz)
    {
        return 0;
    }
    else if (l_sz > r_sz)
    {
        return 1;
    }
}

int size_of(Type *type)
{
    switch (type->kind)
    {
    case CHAR:
        return 1;
    case INT:
        return 4;
    case PTR:
        return 8;
    }
}

char *char_of(Type *type)
{
    switch (type->kind)
    {
    case CHAR:
        return "BYTE\0";
    case INT:
        return "DWORD\0";
    case PTR:
        return "QWORD\0";
    }
}
