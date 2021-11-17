#include "parse.h"

// nodekind が引数通りであるようなnode をメモリ確保だけする
Node *create_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->lhs = NULL;
    node->rhs = NULL;
    node->kind = kind;
    return node;
}

// create_node し　それにつながる左辺、右辺を登録する
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = create_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// create_node(ND_NUM) し数値を登録する　常に終端記号になるので左右辺はない
Node *new_node_num(int val)
{
    Node *node = create_node(ND_NUM);
    node->val = val;
    return node;
}

// create_node(ND_LVAR) し対応する変数のポインタを登録する
Node *new_node_lvar(LVar *lvar)
{
    Node *node = create_node(ND_LVAR);
    node->lvar = lvar;
    return node;
}

// 空白を許すstmt に1push を保証する
// stmt 解釈をする　空白であるならば　０の省略と解釈する
Node *create_or_stmt()
{
    Node *node;
    if (consume(TK_RESERVED, ";"))
    {
        node = new_node_num(0);
    }
    else
    {
        node = stmt();
    }

    return node;
}

//       if
// (expr)  {stmt}
// else node とは独立してif node を完成させる
// 先頭のif は読み進められた状態で呼ばれる
Node *new_node_if()
{
    Node *node = create_node(ND_IF);
    expect(TK_RESERVED, "(");
    node->lhs = expr();
    expect(TK_RESERVED, ")");
    node->rhs = create_or_stmt();
    return node;
}

//   else
// if    else/{stmt}
// if の時点で else を作る　右辺にはelse かstmt()
// 必要に応じてelse 0; を付加することで 常に丁度１つを実行し1push を保証する
// つまりelse はif の連結である
Node *new_node_else()
{
    // 既にif があることがわかっていて消費されている
    Node *node = new_node(ND_ELSE, new_node_if(), NULL);

    // 条件分岐制御構文がまだ続くなら
    // else if() / else {stmt}
    if (consume_keyword(TK_ELSE))
    {
        // else if()
        if (consume_keyword(TK_IF))
        {
            node->rhs = new_node_else();
        }
        // else {stmt}
        else
        {
            node->rhs = create_or_stmt();
        }
    }
    // [if()] -> 既に処理されている
    else
    {
        node->rhs = new_node_num(0);
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

// 条件式の仕様に合わせて空白は1 の省略と解釈して1push を保証する
Node *new_node_for()
{
    expect(TK_RESERVED, "(");

    Node *nodes[3];
    char *op[] = {";", ";", ")"};
    for (int i = 0; i < 3; i++)
    {
        if (current_token_is(TK_RESERVED, op[i]))
        {
            nodes[i] = new_node_num(1);
        }
        else
        {
            nodes[i] = expr();
        }

        expect(TK_RESERVED, op[i]);
    }

    return new_grand_node(ND_FOR_WHILE, nodes[0], nodes[2], create_or_stmt(), nodes[1]);
}

Node *new_node_while()
{
    expect(TK_RESERVED, "(");
    Node *node_B = expr(); // 空欄を許さない
    expect(TK_RESERVED, ")");

    // A式, C式はfor の空欄時に従って1　を入れておく
    return new_grand_node(ND_FOR_WHILE, new_node_num(1), new_node_num(1), create_or_stmt(), node_B);
}

Node *new_node_do()
{
    Node *lhs = create_or_stmt();

    expect(TK_WHILE, "while");
    expect(TK_RESERVED, "(");
    Node *rhs = expr();
    expect(TK_RESERVED, ")");
    expect(TK_RESERVED, ";");

    return new_node(ND_DO, lhs, rhs);
}

// sizeof に続くunary() をパースする　得られた結果のnode のサイズのnd_num を登録する
// parce されたunary() はサイズ計算にのみ用いられて破棄されるため　gen() で実行されることはない
Node *new_node_sizeof()
{
    Node *node = unary();
    return new_node_num(size_of_node(node));
}

// : }が出現するまでnext つなぎに ; で区切られた１文ずつを解釈しnode を登録していく　
// 全体として繋がれたnode の先頭を返す 終端はNULL
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

// 暗黙的なキャスト

// 左右辺を持つnode に対してどちらのnode のサイズが大きいかを返す
int cmp_node_size(Node *node)
{
    if (node->lhs == NULL || node->rhs == NULL)
    {
        error("終端node を展開しています");
    }

    int l_sz = size_of_node(node->lhs);
    int r_sz = size_of_node(node->rhs);

    if (l_sz < r_sz)
    {
        return -1;
    }
    else if (l_sz == r_sz)
    {
        return 0;
    }
    else if (l_sz > r_sz)
    {
        return 1;
    }
}

// node が担当するサイズを返す
int size_of_node(Node *node)
{
    return size_of(type_of_node(node));
}

// 加減算の際　キャストだけでなく値の乗算も必要かどうかを判定する
bool has_ptr_to(Type *type)
{
    if (type->kind == PTR || type->kind == ARRAY)
    {
        return true;
    }

    return false;
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
    if (node == NULL)
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
        node->type = type_of_node(node->rhs);
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

        if (has_ptr_to(deref) == false)
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