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

// node->type はこの関数でのみ決定される
// 四則演算では大きい型にキャストされるため　型を選択する
// 比較演算子と数値node はINT 型を登録される
// 変数や関数では登録された型を採用する
// 代入式では最左の変数の型を返すことになるがパースされているのは右辺なのでrhs を返す
// deref のtype はnode->lhs のtype->ptr_to に等しい
// addr のtype はnode->lhs をptr_to でつなぐtype に等しい
// void 型はキャストされない
Type *type_of_node(Node *node)
{
    // node がNULL であったり、既にtype が決定している場合は探索済みとする
    if (NULL == node)
    {
        return NULL;
    }
    else if (node->type)
    {
        return node->type;
    }

    // 左右辺に対してnode->type の決定を行う
    type_of_node(node->lhs);
    type_of_node(node->rhs);

    NodeKind kind = node->kind;

    if (kind == ND_ADD || kind == ND_SUB || kind == ND_MUL || kind == ND_DIV || kind == ND_DIV_REM)
    {
        // 左右展開されるnode なのでキャストされる可能性がある
        if (cmp_node_size(node) == -1)
        {
            node->type = node->rhs->type;
        }
        else
        {
            node->type = node->lhs->type;
        }
    }
    else if (kind == ND_EQ || kind == ND_NE || kind == ND_LT || kind == ND_LE || kind == ND_NUM)
    {
        node->type = create_type(INT);
    }
    else if (kind == ND_ASSIGN)
    {
        // assign の右辺にはND_ADDR であるため　更にその左辺を参照する
        node->type = type_of_node(node->rhs->lhs);
    }
    else if (kind == ND_FUNC_CALL)
    {
        node->type = node->func->type;
    }
    else if (kind == ND_LVAR)
    {
        node->type = node->lvar->type;
    }
    else if (kind == ND_ADDR)
    {
        node->type = new_type(PTR, type_of_node(node->lhs));
    }
    else if (kind == ND_DEREF)
    {
        Type *deref = type_of_node(node->lhs);

        if (false == has_ptr_to(deref))
        {
            error("非ポインタ型を参照しています");
        }

        node->type = deref->ptr_to;
    }
    else
    {
        node->type = NULL;
    }

    return node->type;
}