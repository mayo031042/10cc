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
    node = new_node(ND_ADDR, node, NULL);
    node = new_node(ND_DEREF, node, NULL);
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

// ND_ADDR, ND_DEREF を作成する際にはこれらの関数をかならず使用する　右辺にNULL が来ることを保証する
// 引数のnode がderef 型だった場合　そのnode の左辺を返す そうでない時はND_LVAR である必要がある
Node *new_node_addr(Node *node)
{
    if (ND_DEREF == node->kind)
    {
        return node->lhs;
    }

    if (ND_LVAR != node->kind)
    {
        error("非変数のアドレスを計算しています");
    }

    return new_node(ND_ADDR, node, NULL);
}

// 参照先のType を見て左辺として適切かを判定
Node *new_node_deref(Node *node)
{
    if (false == has_ptr_to(type_of_node(node)))
    {
        error("非ポインタ型を参照しています");
    }

    return new_node(ND_DEREF, node, NULL);
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

// lhs を左辺に、[rhs を左辺に持つようなND_ADDR] を右辺にもつようなND_ASSIGN node を作成する
Node *new_node_assign(Node *lhs, Node *rhs)
{
    // return new_node(ND_ASSIGN, lhs, rhs);
    return new_node(ND_ASSIGN, lhs, new_node(ND_ADDR, rhs, NULL));
}

// 関数呼び出しnode を作成する　関数出ない買った場合NULL を返す
Node *try_node_func_call()
{
    if (false == current_token_is(TK_RESERVED, "("))
    {
        return NULL;
    }

    // 関数なので　関数呼び出しである
    Node *node = create_node(ND_FUNC_CALL);

    // 既存の関数であれば関数実体へのポインタを保持する配列の要素のアドレスが返る
    node->func = find_func();

    if (NULL == node->func)
    {
        error_at(tokens[val_of_ident_pos()]->str, "未定義な関数です");
    }

    // node のtype をfunc のtype に揃える
    node->type = node->func->type;

    // 引数を解釈する
    node->lhs = build_arg();

    return node;
}

// 暗黙的なキャスト

// 左右辺を持つnode に対してどちらのnode のサイズが大きいかを返す
int cmp_node_size(Node *node)
{
    if (NULL == node->lhs || NULL == node->rhs)
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
