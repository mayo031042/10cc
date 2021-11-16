#include "parse.h"

// 有効変数列内を新規作成順に探索する　最初に発見した変数のoffset を返す
int culc_offset()
{
    for (int depth = val_of_block_nest(); 0 <= depth; depth--)
    {
        if (funcs[func_pos]->locals[depth])
        {
            return funcs[func_pos]->locals[depth]->offset;
        }
        // locals[depth] がNULL なら上位のブロック深度を探索
    }

    return 0;
}

// 有効変数列の中で最大のoofset から必要とするメモリサイズ分下げたoffset を登録したLVar を作成する
// さらに該当関数のmax_offset も更新し　locals[] の最後尾を変更する
// つまりfuncs[]->locals[] は常に　該当関数の該当ネスト部分で作成された最新の変数を保持している（offset がそのネストの中で最大）
// name, len, offset, next が登録された変数を作成する
// 関数のmax_offset が常に最新の登録変数のoffset を指しているとは限らない
// 変数の型を登録　とりあえずINTのみ
LVar *new_lvar(Type *type)
{
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->type = type;
    lvar->name = tokens[val_of_ident_pos()]->str;
    lvar->len = tokens[val_of_ident_pos()]->len;

    // 8 は自分の型に合わせて　変更
    lvar->offset = culc_offset() + size_of(type);

    if (funcs[func_pos]->max_offset < lvar->offset)
    {
        funcs[func_pos]->max_offset = lvar->offset;
    }

    lvar->next = funcs[func_pos]->locals[val_of_block_nest()];
    funcs[func_pos]->locals[val_of_block_nest()] = lvar;

    return lvar;
}

// func_pos を参照しつつ引数のブロック深度内のみから 直前識別子名に合致する変数を探す なければNULL
LVar *find_lvar_within_block(int depth)
{
    for (LVar *lvar = funcs[func_pos]->locals[depth]; lvar; lvar = lvar->next)
    {
        // 指定ブロック深度内に　条件に合致する変数を発見したときは その変数を返す
        if (lvar->len == tokens[val_of_ident_pos()]->len && !memcmp(lvar->name, tokens[val_of_ident_pos()]->str, lvar->len))
        {
            return lvar;
        }
    }

    return NULL;
}

// 既出変数から直前識別子名に一致するものを探す -> 変数呼び出しの際にのみ使用
// 合致する変数が見つかればその変数を　なければerror
LVar *find_lvar()
{
    LVar *lvar;
    // 有効変数列を全探索する
    for (int depth = val_of_block_nest(); 0 <= depth; depth--)
    {
        lvar = find_lvar_within_block(depth);
        // 変数が帰ってきた場合　条件に合致しているので返す
        if (lvar)
        {
            return lvar;
        }
    }

    // 宣言されていた変数列から見つからなかったのでエラー
    error_at(tokens[token_pos]->str, "宣言されていない変数です");
}

// stmt() 内での変数の宣言について扱う　型部分だけ既に読み勧めている
// token が識別子であることを確認し　初出変数であることを確認し　作成する
// funcs[]->locals[]にlvar を登録する 多重定義はエラー
Node *declare_lvar()
{
    // 変数宣言専用のノードを作成
    Node *node = create_node(ND_DECLARE);

    // 型宣言を処理
    Type *type = new_type(expect_vartype());
    type = add_type_ptr(type);
    expect_ident();
    type = add_type_array(type);

    // 現在のスコープの中を探索する
    LVar *lvar = find_lvar_within_block(val_of_block_nest());

    // 同一変数が見つかれば多重定義としてエラー
    if (lvar)
    {
        error_at(tokens[token_pos]->str, "既に宣言されている変数です");
    }

    // 変数宣言なので上位ブロック深度に 合致する変数が存在するしないに関わらず　必ず新規登録する
    new_lvar(type);

    return node;
}
