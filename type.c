#include "parse.h"

Type *new_type(TypeKind kind)
{
    Type *type = calloc(1, sizeof(type));
    type->kind = kind;
    return type;
}

// 型宣言のみ終了している段階で行う　
// 0こ以上の* を解釈する
Type *add_type_ptr(Type *type)
{
    // * が続く限り処理する
    while (consume(TK_OPERATOR, "*"))
    {
        Type *next = new_type(PTR);
        next->ptr_to = type;
        type = next;
    }

    return type;
}

// [] があれば解釈して引数に付加する　なければそのままを返す
// 最後尾の配列（a[x][y][z] なら[z] ）がポインタを含む型宣言を直接つなぐので再帰的に探索する
// 引数のtype はいじらずに次の関数呼び出しに渡す
Type *add_type_array(Type *type)
{
    // [] がないならそのままを返す
    if (!consume(TK_RESERVED, "["))
    {
        return type;
    }

    Type *array = new_type(ARRAY);
    // 数値リテラル以外には未対応
    // Node *node = expr();
    array->array_size = expect_number();
    expect(TK_RESERVED, "]");

    array->ptr_to = add_type_array(type);

    return array;
}