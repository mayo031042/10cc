#include "10cc.h"

#define MAX_STACK_SIZE 100

int Stack[MAX_STACK_SIZE];
int StackTop = 0;
int block_nest = 0;

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

void aaa()
{
    fprintf(stderr, "\n\tnow-> %s\n", tokens[token_pos]->str);
}

int size_of_lvar(Node *node)
{
    if (node->lvar)
    {
        return size_of(node->lvar->type);
    }

    else if (node->kind == ND_DEREF)
    {
        return 8;
    }

    // error();
}

int size_of(Type *type)
{
    switch (type->type)
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
    switch (type->type)
    {
    case CHAR:
        return "BYTE\0";
    case INT:
        return "DWORD\0";
    case PTR:
        return "QWORD\0";
    }
}
