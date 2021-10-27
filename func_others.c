#include "10cc.h"

#define Queue_size 100

int Queue[Queue_size];
int Queue_front = 0;

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

void err(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

int count()
{
    static int cnt = 2;
    return cnt++;
}

void clear_semicolon()
{
    while (consume(TK_RESERVED, ";"))
        ;
}

int qfront()
{
    if (!Queue_front)
        error("ループ構文内ではありません");
    return Queue[Queue_front];
}

void qpush(int val)
{
    Queue_front++;
    if (Queue_size <= Queue_front)
        error("Queue size overflowed.");
    Queue[Queue_front] = val;
}
void qpop()
{
    if (!Queue_front)
        error("Queue size underflowed.");
    Queue_front--;
}