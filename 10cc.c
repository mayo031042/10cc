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

// printfと同じ引数をとるらしい
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// consume, expect, expect_numberはtokenの解析に用いられる　
// 期待されるtokenの種類を決め打つことでerrorの判定が楽である

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて真を返す。
// それ以外の場合には偽を返す。
bool consume(char op)
{
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op)
{
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number()
{
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token()
{
}

// 入力文字列pをトークナイズしてそれを返す
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