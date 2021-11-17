#include "parse.h"

// 引数をTypeKind として持つような新しいType を作成する
Type *create_type(TypeKind kind)
{
    Type *type = calloc(1, sizeof(type));
    type->kind = kind;
    return type;
}

// 引数をptr_to としてもつような新しいTypeKind なType を作成する
Type *new_type(TypeKind kind, Type *ptr_to)
{
    Type *type = create_type(kind);
    type->ptr_to = ptr_to;
    return type;
}

// 型宣言のみ終了している段階で行う　
// 0こ以上の* を解釈する
Type *add_type_ptr(Type *type)
{
    // * が続く限り処理する
    while (consume(TK_OPERATOR, "*"))
    {
        type = new_type(PTR, type);
    }

    return type;
}

// [] があれば解釈して引数に付加する　なければそのままを返す
// 最後尾の配列（a[x][y][z] なら[z] ）がポインタを含む型宣言を直接つなぐので再帰的に探索する
// 引数のtype はいじらずに次の関数呼び出しに渡す
Type *add_type_array(Type *type)
{
    // [] がないならそのままを返す
    if (consume(TK_RESERVED, "["))
    {
        // 数値リテラル以外には未対応
        // Node *node = expr();
        int array_size = expect_number();
        expect(TK_RESERVED, "]");

        type = new_type(ARRAY, add_type_array(type));
        type->array_size = array_size;
    }

    return type;
}