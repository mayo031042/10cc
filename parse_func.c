#include "10cc.h"

// 今見ているtoken と引数の文字列が一致していたら　true を返しtoken を読み進める
bool consume(TokenKind kind, char *op)
{
    if (tokens[pos]->kind != kind || tokens[pos]->len != strlen(op) || memcmp(tokens[pos]->str, op, tokens[pos]->len))
    {
        return false;
    }
    pos++;
    return true;
}

// consumeと同じ判定をするが　falseが返る場合は代わりにerror を吐く
bool expect(TokenKind kind, char *op)
{
    if (consume(kind, op))
    {
        return true;
    }
    error_at(tokens[pos]->str, "%cではありません\n", op);
}

// expectと同様にflase ならerrorを吐く　true ならtoken に数値を登録し読み進める
int expect_number()
{
    if (tokens[pos]->kind != TK_NUM)
    {
        error_at(tokens[pos]->str, "数値ではありません\n");
    }
    int val = tokens[pos]->val;
    pos++;
    return val;
}

bool consume_keyword(TokenKind kind)
{
    if (tokens[pos]->kind != kind)
    {
        return false;
    }
    if (kind == TK_IDENT)
    {
        ident_pos = pos;
    }
    pos++;
    return true;
}

// token 列の最後尾の次だったらtrue
bool at_eof()
{
    return (tokens[pos]->kind == TK_EOF);
}
