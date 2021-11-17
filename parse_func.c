#include "parse.h"
// tokens[] やtoken_pos を扱い parse を補助する関数など
// また node やfuncs[] の作成を補助する関数など

// 最後に確認した識別子のtoken_pos
int ident_pos = 0;

// token を読み進めずに判定のみ行う current 系

// op == NULL の時　種類が一致すればtrue
// op != NULL の時　種類と文字列のどちらも一致すればtrue
bool current_token_is(TokenKind kind, char *op)
{
    if (op == NULL)
    {
        return (tokens[token_pos]->kind == kind);
    }
    else
    {
        return (tokens[token_pos]->kind == kind && tokens[token_pos]->len == strlen(op) && !memcmp(tokens[token_pos]->str, op, tokens[token_pos]->len));
    }
}

// 引数と一致したらtoken を読み進める consume 系
// consume ,consume_keyword, consume_ident

// 今見ているtoken と引数の文字列が一致していたならtrue を返しtoken を読み進める
bool consume(TokenKind kind, char *op)
{
    if (!current_token_is(kind, op))
    {
        return false;
    }

    token_pos++;
    return true;
}

// 今のtoken が引数通りなら読み進める　更に識別子のときはident_pos を更新する
bool consume_keyword(TokenKind kind)
{
    if (tokens[token_pos]->kind != kind)
    {
        return false;
    }

    token_pos++;
    return true;
}

// 識別子専用の関数　consume と同時にident_pos のセットも行う
bool consume_ident()
{
    if (tokens[token_pos]->kind != TK_IDENT)
    {
        return false;
    }

    ident_pos = token_pos;
    token_pos++;
    return true;
}

// consume できなければエラーを吐く expect 系
// expect, ex_ident, ex_vartype, ex_number

// consumeと同じ判定をするが　falseが返る場合は代わりにerror を吐く
bool expect(TokenKind kind, char *op)
{
    if (consume(kind, op))
    {
        return true;
    }

    error_at(tokens[token_pos]->str, "%cではありません\n", op);
}

bool expect_ident()
{
    if (consume_ident())
    {
        return true;
    }

    error_at(tokens[token_pos]->str, "識別子ではありません");
}

// int 型宣言を期待　拡張可能
// 型宣言を処理し TypeKind で返す
TypeKind expect_vartype()
{
    if (consume_keyword(TK_TYPE))
    {
        return INT;
    }

    error_at(tokens[token_pos]->str, "型宣言がありません");
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

// token 列の最後尾の次だったらtrue
bool at_eof()
{
    return (tokens[token_pos]->kind == TK_EOF);
}

int val_of_ident_pos()
{
    return ident_pos;
}
