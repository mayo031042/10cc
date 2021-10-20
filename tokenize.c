#include "10cc.h"

// 今見ているtoken と引数の文字列が一致していたら　true を返しtoken を読み進める
bool consume(char *op)
{
    if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

// consumeと同じ判定をするが　falseが返る場合は代わりにerror を吐く
void expect(char *op)
{
    if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
        error("%cではありません\n", op);
    token = token->next;
}

// expectと同様にflase ならerrorを吐く　true ならtoken に数値を登録し読み進める
int expect_number()
{
    if (token->kind != TK_NUM)
        error("数値ではありません\n");
    int val = token->val;
    token = token->next;
    return val;
}

// token が識別子である時　そのtoken のコピーを返し　token を読み進める
Token *consume_ident()
{
    if (token->kind != TK_IDENT)
        return NULL;
    Token *tok = token;
    token = token->next;
    return tok;
}

bool consume_keyword(TokenKind kind)
{
    if (token->kind != kind)
        return false;
    token = token->next;
    return true;
}

// token 列の最後尾の次だったらtrue
bool at_eof()
{
    return token->kind == TK_EOF;
}

// 新しいtoken に{種類、文字列、長さ} を登録し　今のtoken のnext としてつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

// 変数に使える文字か否かを返す -> token 構成文字
int is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

// 入力されたプログラムを先頭からtokenize していく　
// error がなかった場合は　ｇ変数token をtoken 列の先頭にセットして終了
void *tokenize()
{
    // 先頭を指すためだけの　空のtoken
    Token head;
    // 入力を区切る　意味のあるtoken はhead.next から順につながっている
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
        if (!strncmp(p, "return", 6) && !is_alnum(p[6]))
        {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }
        else if (*(p + 1) == '=' && strchr("+-*/!=<>", *p))
        {
            if (strchr("!=<>", *p))
                cur = new_token(TK_RESERVED, cur, p, 2);
            else
                cur = new_token(TK_ASSIGN_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        else if (strchr("+-*/()=<>;", *p))
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
        else if (is_alnum(*p))
        {
            char *q = p;
            for (; is_alnum(*q); q++)
                ;
            cur = new_token(TK_IDENT, cur, p, q - p);
            p = q;
            continue;
        }

        error("%s tokenizeできません\n", p);
    }

    new_token(TK_EOF, cur, p, 0);
    token = head.next;
}
