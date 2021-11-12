#include "parse.h"

// nodekind が引数であるようなnode をメモリ確保とint type 指定だけする
Node *create_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->lhs = NULL;
    node->rhs = NULL;
    node->kind = kind;
    node->type = new_type(INT);
    return node;
}

// node に種類と それにつながる左辺、右辺を登録する
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = create_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 数値を指すnode を作成する　常に葉であるような終端記号になるので左右に辺は持たない
Node *new_node_num(int val)
{
    Node *node = create_node(ND_NUM);
    node->val = val;
    return node;
}

// LVarを参照する
Node *new_node_lvar(LVar *lvar)
{
    Node *node = create_node(ND_LVAR);
    node->offset = lvar->offset;
    node->lvar = lvar;
    node->type = lvar->type;
    return node;
}

// stmt を期待する位置の直後に；が来ている場合　1push 保証の観点から 代わりにcreate_node() する
Node *create_or_stmt(NodeKind kind)
{
    Node *node;
    if (consume(TK_RESERVED, ";"))
    {
        node = create_node(kind);
    }
    else
    {
        node = stmt();
    }

    return node;
}

// 完全に独立してif node を完成させる
Node *new_node_if()
{
    Node *node = create_node(ND_IF);
    expect(TK_RESERVED, "(");
    node->lhs = expr();
    expect(TK_RESERVED, ")");
    node->rhs = create_or_stmt(ND_PUSH_0);
    return node;
}

// if の時点で else を作る　else node　の左辺にif を配置 右辺にはNULLかstmt()
// 条件分岐の終了はif かelse -> if で終了するときはelse 0; の省略と解釈する
Node *new_node_else()
{
    // 既にif があることがわかっていて消費されている
    Node *node = create_node(ND_ELSE);
    node->lhs = new_node_if();

    // 条件分岐制御構文がまだ続くなら
    if (consume_keyword(TK_ELSE))
    {
        // else if として続くなら
        if (consume_keyword(TK_IF))
        {
            node->rhs = new_node_else();
        }
        // else だけで終わるなら
        else
        {
            node->rhs = create_or_stmt(ND_PUSH_0);
        }
    }
    // if で終了するならelse 0; の省略として処理
    else
    {
        node->rhs = create_node(ND_PUSH_0);
    }

    return node;
}

// 孫node に引数順のnode を持つnode を作成して返す
Node *new_grand_node(NodeKind kind, Node *l_l, Node *l_r, Node *r_l, Node *r_r)
{
    Node *lhs = new_node(ND_NOP, l_l, l_r);
    Node *rhs = new_node(ND_NOP, r_l, r_r);
    return new_node(kind, lhs, rhs);
}

Node *new_node_for()
{
    expect(TK_RESERVED, "(");

    Node *nodes[3];
    char *op[] = {";", ";", ")"};
    for (int i = 0; i < 3; i++)
    {
        if (current_token_is(TK_RESERVED, op[i]))
        {
            // 条件式が空欄な時は恒真式なので　１が入っているとしてparseする
            nodes[i] = create_node(ND_PUSH_1);
        }
        else
        {
            nodes[i] = expr();
        }

        expect(TK_RESERVED, op[i]);
    }

    return new_grand_node(ND_FOR_WHILE, nodes[0], nodes[2], create_or_stmt(ND_PUSH_0), nodes[1]);
}

Node *new_node_while()
{
    expect(TK_RESERVED, "(");
    Node *node_B = expr(); // 空欄を許さない
    expect(TK_RESERVED, ")");

    // A式, C式はfor の空欄時に従って　ND_PUSH_1 を入れておく
    return new_grand_node(ND_FOR_WHILE, create_node(ND_PUSH_1), create_node(ND_PUSH_1), create_or_stmt(ND_PUSH_0), node_B);
}

Node *new_node_do()
{
    Node *lhs = create_or_stmt(ND_PUSH_0);

    expect(TK_WHILE, "while");
    expect(TK_RESERVED, "(");
    Node *rhs = expr();
    expect(TK_RESERVED, ")");
    expect(TK_RESERVED, ";");

    return new_node(ND_DO, lhs, rhs);
}
// : }が出現するまでnextつなぎにnode を登録していく　
// 全体として繋がれたnode の先頭を返す 終端はNULL

// sizeof に続くunary() をパースする　得られた結果のnode のサイズのnd_num を登録する
// parce されたunary() はサイズ計算にのみ用いられて破棄されるため　gen() で実行されることはない
Node *new_node_sizeof()
{
    Node *node = unary();
    return new_node_num(size_of_node(node));
}

Node *new_node_block()
{
    // 意味のある; はstmt() 内で処理をする
    while (consume(TK_RESERVED, ";"))
        ;

    if (consume_keyword(TK_BLOCK_END))
    {
        return NULL;
    }

    Node *node = stmt();
    node->next = new_node_block();
    return node;
}

// build_block() とgen_block() でのみ block_nest の値をいじる
Node *build_block()
{
    add_block_nest();

    Node *node = new_node(ND_BLOCK, new_node_block(), NULL);

    sub_block_nest();
    return node;
}

// node が担当するサイズを返す
int size_of_node(Node *node)
{
    return size_of(type_of_node(node));
}

// node を基準に型を走査する　基本的にはINT なType を返すことになる
// 四則演算では大きい型にキャストされるため　型を選択する
// 変数や関数では登録された型を採用する
// 代入式では最左の変数の型を返すことになるがパースされているのは右辺なのでrhs を返す
// deref では再帰的に探索する　帰ってきた型をptr_to で参照し一つ進めたものの型が結果である
// addr では再帰的に探索する　帰ってきた型にptr_to でつながる型を返す
// 計算過程でnode のtype が変化することを暗黙的なキャストに対応させている
// すべてのnode はcreate されたタイミングでINT型のType を割り当てられている
// 四則演算やderef でもINTのままであるため　それらを変更する
Type *type_of_node(Node *node)
{
    NodeKind kind = node->kind;

    // if(node->type->kind!=INT)return node->type;

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
    else if (kind == ND_ASSIGN)
    {
        // 代入先の変数の型が優先される
        node->type = type_of_node(node->rhs);
    }
    else if (kind == ND_FUNC_CALL)
    {
        node->type = funcs[node->func_num]->type;
    }
    else if (kind == ND_LVAR)
    {
        node->type = node->lvar->type;
    }
    else if (kind == ND_ADDR)
    {
        node->type->kind = PTR;
        node->type->ptr_to = type_of_node(node->lhs);
    }
    else if (kind == ND_DEREF)
    {
        Type *deref = type_of_node(node->lhs);

        if (deref->kind != PTR)
        {
            error("非ポインタ型を参照しています");
        }

        node->type = deref->ptr_to;
    }
    else
    {
        node->type = new_type(INT);
    }

    return node->type;
}

