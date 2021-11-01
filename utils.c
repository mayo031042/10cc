#include "10cc.h"

#define MAX_STACK_SIZE 100

int Stack[MAX_STACK_SIZE];
int StackTop = 0;

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

// void clear_semicolon()
// {
//     while (consume(TK_RESERVED, ";"))
//     ;
// }

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

void aaa()
{
    fprintf(stderr, "\n\tnow-> %s\n", tokens[token_pos]->str);
}