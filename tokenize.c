#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "10cc.h"

bool consume(char *op)
{
    if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
        return 0;
    token = token->next;
    return 1;
}

void expect(char *op)
{
    if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
        error("%cではありません\n", op);
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
        error("数値ではありません\n");
    int val = token->val;
    token = token->next;
    return val;
}

// 今見ているtokenがidentでないならば−１　
// identならばtokenを読み勧めて　offsetを返す
Token *consume_ident()
{
    if (token->kind != TK_IDENT)
        return NULL;

    Token *tok = token;
    token = token->next;
    return tok;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

// 変数に使える文字か？
int is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

void *tokenize()
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    char *p = user_input;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (!strncmp(p, "==", 2) || !strncmp(p, "!=", 2) || !strncmp(p, "<=", 2) || !strncmp(p, ">=", 2))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        else if (strchr("+-*/()<>=;", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        else if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            continue;
        }
        // 変数判定は　数値判定よりも　後に持ってくる必要がある　
        else if (is_alnum(*p))
        {
            char *q = p;
            // p,qはこの時　どちらも変数文字列の先頭である
            for (; is_alnum(*q); q++)
                ;
            // while (is_alnum(*++q)); でも可

            // この段階で　qのみが変数の最後尾の次を指している
            // よって変数の長さは　ちょうどq-p　になる
            cur = new_token(TK_IDENT, cur, p, q - p);
            // 次に見たいpは　ちょうどqの位置にある
            p = q;
            continue;
        }

        error("%s tokenizeできません\n", p);
    }

    new_token(TK_EOF, cur, p, 0);
    token = head.next;
}
