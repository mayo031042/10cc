#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenの種類を分ける
typedef enum
{
    TK_reserved,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

// 連結リストで持ちたいので　自分のトークン種類、数値?値:記号をもち　次のトークンのアドレスを保持する
struct Token
{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

Token *token;

void error(char *fmt, ...)
{
}

bool consume()
{
}

void expect()
{
}

bool at_eof()
{
}

Token *new_token()
{
}

Token *tokenize()
{
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の数が間違っています\n");
        return 0;
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("    mov rax, %d\n", strtol(p, &p, 10));

    while (*p)
    {
        if (*p == '+')
        {
            // 数値と想定されるものを指すポインタに進む
            p++;
            printf("    add rax, %d\n", strtol(p, &p, 10));
            continue;
        }
        else if (*p == '-')
        {
            p++;
            printf("    sub rax, %d\n", strtol(p, &p, 10));
            continue;
        }
        else
        {
            fprintf(stderr, "入力が間違っています\n");
            return 1;
        }
    }

    printf("    ret\n");
    return 0;
}