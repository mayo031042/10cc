#include "parse.h"

Type *new_type(TypeKind kind)
{
    Type *type = calloc(1, sizeof(type));
    type->kind = kind;
    return type;
}

// 型宣言が来ていることがわかっている状態で行う
// 型、参照、配列のすべての指定を解釈し適切なtype を返す
Type *type_and_ptr()
{
    Type *type = new_type(expect_vartype());

    // * が続く限り処理する
    while (consume(TK_OPERATOR, "*"))
    {
        Type *next = new_type(PTR);
        next->ptr_to = type;
        type = next;
    }

    return type;
}

// // [] があれば解釈して引数に付加する　なければそのままを返す
// Type *add_type_array(Type *type)
// {
//     if (!consume(TK_RESERVED, "["))
//     {
//         return type;
//     }

//     Node *node = expr();
//     Type *array=new_type(ARRAY);
//     array->ptr_to=type;
//     array->array_size=

//     expect(TK_RESERVED, "]");
// }