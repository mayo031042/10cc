#include "10cc.h"

// node の種類を登録し　それがつなぐ左辺、右辺を指すようにする
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 数値を指すnode を作成する　常に葉であるような終端記号になるので左右に辺は持たない
Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// LVarを参照する
Node *new_node_ident(LVar *lvar)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = lvar->offset;
    return node;
}

// tokを参照して　新しいlvarを作成する つまり未出のlvarに対してのみ行う処理
LVar *new_lvar(Token *tok)
{
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    // 一番初めのlocalsはNULLである
    if (locals)
        lvar->offset = locals->offset + 8;
    else
        lvar->offset = 0;
    return lvar;
}

// なかったらNULL あったらLVar を返す
LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var; var = var->next)
    {
        if (var->len == tok->len && !memcmp(var->name, tok->str, var->len))
            return var;
    }
    return NULL;
}

// 構文解析のための関数
Node *ident();
Node *num();
Node *primary();
Node *unary();
Node *mul();
Node *add();
Node *relational();
Node *equality();
Node *assign();
Node *expr();
Node *stmt();
// Node *program();

// programの中の最小単位 (expr)か数値か変数しかありえない　
Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    // 変数か数値が残るはず
    // 変数の場合 tok != NULL
    Token *tok = consume_ident();
    if (tok)
    {
        // この場合tokは変数である　既出か否か
        LVar *lvar = find_lvar(tok);
        if (lvar == NULL)
        {
            // 未出である
            lvar = new_lvar(tok);
            // localsは常に最後尾を指すことでoffsetの計算が容易に
            locals = lvar;
        }
        // この段階でlvarは適切なLVarのオブジェクトを指しているので　nodeに反映する
        return new_node_ident(lvar);
    }

    return new_node_num(expect_number());
}

Node *unary()
{
    if (consume("+"))
    {
        return primary();
    }
    else if (consume("-"))
    {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    else
        return primary();
}

Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
        {
            node = new_node(ND_MUL, node, unary());
        }
        else if (consume("/"))
        {
            node = new_node(ND_DIV, node, unary());
        }
        else
            return node;
    }
}

Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume("+"))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if (consume("-"))
        {
            node = new_node(ND_SUB, node, mul());
        }
        else
            return node;
    }
}

// 数式を不等号で繋がれた単位であると見る
Node *relational()
{
    Node *node = add();

    for (;;)
    {
        // 2文字の不等号を先に処理する
        if (consume("<="))
        {
            node = new_node(ND_LE, node, add());
            continue;
        }
        else if (consume(">="))
        {
            node = new_node(ND_LE, add(), node);
            continue;
        }
        else if (consume("<"))
        {
            node = new_node(ND_LT, node, add());
            continue;
        }
        else if (consume(">"))
        {
            node = new_node(ND_LT, add(), node);
            continue;
        }
        else
            return node;
    }
}

// 数式を等号で繋がれた式の連続であると解釈する
Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume("=="))
        {
            node = new_node(ND_EQ, node, relational());
            continue;
        }
        else if (consume("!="))
        {
            node = new_node(ND_NE, node, relational());
            continue;
        }
        else
            return node;
    }
}

// 数式を代入式の連続であると解釈する
Node *assign()
{
    // == ,!=, < 等はequality()内で優先的に処理されている
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());

    return node;
}

Node *expr()
{
    return assign();
}
// ;　で区切る
// return が来たら　return したい式が 左辺==lhsに展開されるような　return node を作成する
Node *stmt()
{
    Node *node;
    if (consume_return())
    {
// error("     error!!\n\n");
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    }
    else
        node = expr();
    expect(";");
    return node;
}
// ；　で区切られたコード全体を　parseする
Node *program()
{
    int i = 0;
    while (!at_eof())
        code[i++] = stmt();
    code[i] = NULL;
}