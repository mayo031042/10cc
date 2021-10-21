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
LVar *new_lvar(int my_pos)
{
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tokens[my_pos]->str;
    lvar->len = tokens[my_pos]->len;
    // 一番初めのlocalsはNULLである
    if (locals)
        lvar->offset = locals->offset + 8;
    else
        lvar->offset = 0;
    return lvar;
}

// なかったらNULL あったらLVar を返す
LVar *find_lvar(int my_pos)
{
    for (LVar *var = locals; var; var = var->next)
    {
        if (var->len == tokens[my_pos]->len && !memcmp(var->name, tokens[my_pos]->str, var->len))
            return var;
    }
    return NULL;
}

// 構文解析のための関数
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
    if (consume(TK_RESERVED, "("))
    {
        Node *node = expr();
        expect(TK_RESERVED, ")");
        return node;
    }

    int my_pos = consume_ident();
    if (my_pos != -1)
    {
        // この場合tokは変数である　既出か否か
        LVar *lvar = find_lvar(my_pos);
        if (lvar == NULL)
        {
            // 未出である
            lvar = new_lvar(my_pos);
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
    if (consume(TK_RESERVED, "+"))
    {
        return primary();
    }
    else if (consume(TK_RESERVED, "-"))
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
        if (consume(TK_RESERVED, "*"))
        {
            node = new_node(ND_MUL, node, unary());
        }
        else if (consume(TK_RESERVED, "/"))
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
        if (consume(TK_RESERVED, "+"))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if (TK_RESERVED, consume(TK_RESERVED, "-"))
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
        if (consume(TK_RESERVED, "<="))
        {
            node = new_node(ND_LE, node, add());
            continue;
        }
        else if (consume(TK_RESERVED, ">="))
        {
            node = new_node(ND_LE, add(), node);
            continue;
        }
        else if (consume(TK_RESERVED, "<"))
        {
            node = new_node(ND_LT, node, add());
            continue;
        }
        else if (consume(TK_RESERVED, ">"))
        {
            node = new_node(ND_LT, add(), node);
            continue;
        }
        else
            return node;
    }
}

// 数式を等号か等号否定で繋がれた式の連続であると解釈する
Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume(TK_RESERVED, "=="))
        {
            node = new_node(ND_EQ, node, relational());
            continue;
        }
        else if (consume(TK_RESERVED, "!="))
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
    if (consume(TK_RESERVED, "="))
        node = new_node(ND_ASSIGN, assign(), node);

    else if (tokens[pos]->kind == TK_ASSIGN_RESERVED)
    {
        if (consume(TK_ASSIGN_RESERVED, "+="))
            node = new_node(ND_ASSIGN, new_node(ND_ADD, node, assign()), node);
        else if (consume(TK_ASSIGN_RESERVED, "-="))
            node = new_node(ND_ASSIGN, new_node(ND_SUB, node, assign()), node);
        else if (consume(TK_ASSIGN_RESERVED, "*="))
            node = new_node(ND_ASSIGN, new_node(ND_MUL, node, assign()), node);
        else
        {
            expect(TK_ASSIGN_RESERVED, "/=");
            node = new_node(ND_ASSIGN, new_node(ND_DIV, node, assign()), node);
        }
    }

    return node;
}
// expr は予約語以下の全ての１文のコードに対応する
Node *expr()
{
    return assign();
}
// ;　で区切る
// return が来たら　return したい式が 左辺==lhsに展開されるような　return node を作成する
Node *stmt()
{
    Node *node;
    if (consume_keyword(TK_RETURN))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    }
    else if (consume_keyword(TK_IF))
    {
        expect(TK_RESERVED, "(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->lhs=stmt();
        expect(TK_RESERVED, ")");
        

    }
    else
        node = expr();
    expect(TK_RESERVED, ";");
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