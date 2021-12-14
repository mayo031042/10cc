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
    if (NULL == op)
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
    if (consume(TK_TYPE, "void"))
    {
        return VOID;
    }
    else if (consume(TK_TYPE, "char"))
    {
        return CHAR;
    }
    else if (consume(TK_TYPE, "int"))
    {
        return INT;
    }
    else if (consume(TK_TYPE, "bool"))
    {
        return INT;
    }

    error_at(tokens[token_pos]->str, "型宣言がありません");
}

// expectと同様にflase ならerrorを吐く　true ならtoken に数値を登録し読み進める
int expect_number()
{
    if (tokens[token_pos]->kind == TK_NUM)
    {
        int val = tokens[token_pos]->val;
        token_pos++;
        return val;
    }

    error_at(tokens[token_pos]->str, "数値ではありません\n");
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

// 引数のKind であるようなToken が出現するまで読み進める
// TK_EOF が引数に渡されたり at_eof() までToken が見つからなかったときはerror
Token *look_ahead(TokenKind kind)
{
    int now_pos = token_pos;

    while (TK_EOF != tokens[now_pos]->kind)
    {
        if (kind == tokens[now_pos]->kind)
        {
            return tokens[now_pos + 1];
        }

        now_pos++;
    }

    // 引数がEOF か見つからないKind の場合error
    error("無効な引数です -> look_ahead()");
}

bool look_ahead_function()
{
    Token *tok = look_ahead(TK_IDENT);
    // tok は関数の識別子の次のToken を指している
    return (TK_RESERVED == tok->kind && tok->len == 1 && !memcmp(tok->str, "(", 1));
}