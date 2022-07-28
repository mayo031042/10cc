#include "parse.h"

bool match_with(Token *tok, char *name, int len)
{
    return (tok->len == len && !memcmp(tok->str, name, len));
}

// 既出関数名から直前識別子名に一致するものを探す
// 既出ならそのfuncs を指すポインタfuncs[i] のアドレス を返し そうでないならNULLを返す
Func *find_func()
{
    for (int i = 0; funcs[i]; i++)
    {
        if (match_with(tokens[val_of_ident_pos()], funcs[i]->name, funcs[i]->len))
        {
            return funcs[i];
        }
    }

    return NULL;
}

// 新しいfuncに len, max_offset, defined, name, typeを設定
Func *new_func(Token *tok, Type *type)
{
    Func *func = calloc(1, sizeof(Func));
    func->type = type;

    strncpy(func->label, tok->str, tok->len);
    func->name = tok->str;
    func->len = tok->len;
    func->max_offset = 0;
    func->defined = false;
    return func;
}

// func_pos で指定された関数の引数の最大offset を返す
// int offset_arg()
// {
//     if (func_pos_ptr->locals[0])
//     {
//         return func_pos_ptr->locals[0]->offset;
//     }

//     return 0;
// }

// locals[0] に引数を登録する 登録された引数は関数呼び出しの際に ブロック０で宣言された変数として振る舞う
// 関数宣言、定義の際 引数を解釈する 既に（ は消費されている
void declare_arg()
{
    expect(TK_RESERVED, "(");
    // 引数なしならNULL を返す
    if (consume(TK_RESERVED, ")"))
    {
        return;
    }

    // global変数を除き その関数内ではじめて作成される変数
    declare_lvar();

    // ")" が出現するまで ", 変数" を０回以上解析する
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

// 関数呼び出しの際の引数渡しを解釈する 順番はlocals[] 等と同様最新を末尾に持つ
// 引数の型と個数を呼び出し先関数の引数と比較する 違っていたらエラー -> スルー
// 数値リテラルの場合は？ 整数からキャスト、、、-> とりあえずスルー

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

// 関数の定義部分完成をtry する
// 宣言のみの場合はそのままreturn
void try_build_definition()
{
    if (consume(TK_RESERVED, ";"))
    {
        return;
    }

    // 定義がくるので 多重定義されていないか確認する
    if (true == func_pos_ptr->defined)
    {
        error_at(tokens[token_pos]->str, "関数が多重定義されています");
    }

    func_pos_ptr->defined = true;
    func_pos_ptr->definition = program();

    // 各関数のmax offset を、それを超えるような最小の16の倍数で改める
    func_pos_ptr->max_offset = (func_pos_ptr->max_offset + 16 - 1) / 16 * 16;
}

Node *new_function()
{
    static int i = 0;

    Type *type = create_type(expect_vartype());
    type = add_type_ptr(type);
    expect_ident();

    // 識別子から 今までに登録されている関数列を全探索する
    func_pos_ptr = find_func();

    if (NULL == func_pos_ptr)
    {
        // 関数がはじめて宣言、定義されるので funcs[] と引数リストの登録を行う
        funcs[i] = new_func(tokens[val_of_ident_pos()], type);
        funcs[i + 1] = NULL;
        func_pos_ptr = funcs[i];
        i++;
        declare_arg();
    }
    else
    {
        // 既に登録済みの関数なので func_pos のセットだけ行い 引数リストは読み飛ばす
        consume_arg();
    }

    try_build_definition();
}