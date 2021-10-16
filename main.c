#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "10cc.h"

Token *token;

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の数が間違っています\n");
        return 1;
    }

    char *user_input = argv[1];
    token = tokenize(user_input);

    // Node *node = program();
    program();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    int i = 0;
    while (code[i] != NULL)
        gen(code[i++]);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}