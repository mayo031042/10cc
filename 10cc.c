#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenの種類
typedef enum
{
    TK_RESERVED,
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

// consume, expect, expect_numberは出来上がったtokenの利用にmain内で用いられる　

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて真を返す。
// それ以外の場合には偽を返す。
// expectと合わせて+か-のどちらであるかを判定する
bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return 0;
    token = token->next;
    return 1;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error("%cではありません\n", op);
    token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number()
{
    if (token->kind != TK_NUM)
        error("数値ではありません\n");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-')
        {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("tokenizeできません\n");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の数が間違っています\n");
        return 1;
    }

    // 正しい入力の時　この段階でtokenは　式の先頭にある記号か数値のどちらかである
    token = tokenize(argv[1]);
    // consume,expectを適切に利用できた場合　tokenは自動で次のTokenをさす

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("    mov rax, %d\n", expect_number());

    while (!at_eof())
    {
        // error処理はtoken確認系が行っている　以下は１処理で記号と数値をセットにしている　
        // これに矛盾する場合は全て確認系のerrorが呼ばれるので　このwhile内にerrorは不要
        if (consume('+'))
        {
            printf("    add rax, %d\n", expect_number());
            continue;
        }
        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}