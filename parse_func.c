#include "10cc.h"

// 最後に確認した識別子のtoken_pos
int ident_pos = 0;

bool current_token_is(TokenKind kind, char *op)
{
    return (tokens[token_pos]->kind == kind && tokens[token_pos]->len == strlen(op) && !memcmp(tokens[token_pos]->str, op, tokens[token_pos]->len));
}

// 今見ているtoken と引数の文字列が一致していたら　true を返しtoken を読み進める
bool consume(TokenKind kind, char *op)
{
    if (!current_token_is(kind, op))
    {
        return false;
    }
    token_pos++;
    return true;
}

// consumeと同じ判定をするが　falseが返る場合は代わりにerror を吐く
bool expect(TokenKind kind, char *op)
{
    if (consume(kind, op))
    {
        return true;
    }
    error_at(tokens[token_pos]->str, "%cではありません\n", op);
}

// expectと同様にflase ならerrorを吐く　true ならtoken に数値を登録し読み進める
int expect_number()
{
    if (tokens[token_pos]->kind != TK_NUM)
    {
        error_at(tokens[token_pos]->str, "数値ではありません\n");
    }
    int val = tokens[token_pos]->val;
    token_pos++;
    return val;
}

bool consume_keyword(TokenKind kind)
{
    if (tokens[token_pos]->kind != kind)
    {
        return false;
    }
    if (kind == TK_IDENT)
    {
        ident_pos = token_pos;
    }
    token_pos++;
    return true;
}

// token 列の最後尾の次だったらtrue
bool at_eof()
{
    return (tokens[token_pos]->kind == TK_EOF);
}

int val_of_ident_pos()
{
    return ident_pos;
}
