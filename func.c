#include "parse.h"

// 既出関数名から直前識別子名に一致するものを探す　
// 既出ならそのfuncs[] のpos を返し　そうでないなら−１を返す
int find_func()
{
    for (int i = 0; funcs[i]; i++)
    {
        if (!memcmp(tokens[val_of_ident_pos()]->str, funcs[i]->name, funcs[i]->len))
        {
            return i;
        }
    }

    return -1;
}

// 新しいfuncに len, max_offset, defined, name を設定
// 型の登録はとりあえずint のみ
Func *new_func(Token *tok, Type *type)
{
    Func *func = calloc(1, sizeof(Func));
    func->type = type; // 要　変更
    func->defined = false;
    func->len = tok->len;
    func->max_offset = 0;
    strncpy(func->name, tok->str, tok->len);
    return func;
}

// func_pos で指定された関数の引数の最大offset を返す
int offset_arg(int func_pos)
{
    if (funcs[func_pos]->locals[0])
    {
        return funcs[func_pos]->locals[0]->offset;
    }
    else
    {
        return 0;
    }
}

// locals[0] に引数を登録する 登録された引数は関数呼び出しの際に　ブロック０で宣言された変数として振る舞う
// 関数宣言、定義の際　引数を解釈する　既に（　は消費されている
void declare_arg()
{
    expect(TK_RESERVED, "(");
    // 引数なしならNULL を返す
    if (consume(TK_RESERVED, ")"))
    {
        return;
    }

    // global変数を除き　その関数内ではじめて作成される変数
    declare_lvar();

    // ")" が出現するまで ", 変数"　を０回以上解析する
    while (!consume(TK_RESERVED, ")"))
    {
        expect(TK_RESERVED, ",");
        declare_lvar();
    }
}

// 引数リストを読み飛ばす
void consume_arg()
{
    expect(TK_RESERVED, "(");
    while (!consume(TK_RESERVED, ")"))
    {
        token_pos++;
    }
}

// 関数呼び出しの際の引数渡しを解釈する　順番はlocals[] 等と同様最新を末尾に持つ
// 引数の型と個数を呼び出し先関数の引数と比較する　違っていたらエラー -> スルー
// 数値リテラルの場合は？　整数からキャスト、、、-> とりあえずスルー

//                func_call
// ... <- arg3 <- (lhs:arg4)
//                (lhs:expr, val:regi)
Node *build_arg()
{
    expect(TK_RESERVED, "(");

    // 引数が１つもない場合 NULL を返す
    if (consume(TK_RESERVED, ")"))
    {
        return NULL;
    }

    // 最初の引数をexpr() 解釈する
    Node *node = new_node(ND_NOP, expr(), NULL);
    node->next = NULL;

    while (!consume(TK_RESERVED, ")"))
    {
        expect(TK_RESERVED, ",");

        Node *arg = new_node(ND_NOP, expr(), NULL);
        arg->next = node;
        node = arg;
    }

    return node;
}