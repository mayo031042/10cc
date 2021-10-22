#include "10cc.h"

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

int code_pos;
int small_if;

Node *create_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

// node の種類を登録し　それがつなぐ左辺、右辺を指すようにする
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
Node *new_node_ident(LVar *lvar)
{
    Node *node = create_node(ND_LVAR);
    node->offset = lvar->offset;
    return node;
}

// if node を作る　同一code_pos内で　if node が作られるたびに　small_ifは増加
Node *new_node_if()
{
    Node *node = create_node(ND_IF);
    node->next_label = small_if++;
    node->end_label = code_pos;
    return node;
}
// if node を受け取り　それを参照しつつ左辺に配置したelse node を作成する
Node *new_node_else(Node *node_if)
{
    Node *node = create_node(ND_ELSE);
    node->end_label = node_if->end_label;
    node->lhs = node_if;
    return node;
}

Node *new_node_if_else()
{
    Node *node = new_node_if();

    expect(TK_RESERVED, "(");
    node->lhs = expr();
    expect(TK_RESERVED, ")");

    node->rhs = stmt();
    // ただのifなら　ここでreturn node につながるだけ
    if (consume_keyword(TK_ELSE))
    {
        node = new_node_else(node);
        // if, else, その他　何が来てもstmtで対処できる
        node->rhs = stmt();
    }
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

// programの中の最小単位 (expr)か数値か変数しかありえない　
// 演算子は処理されているので　残るは数値等　のみである
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

// 単項演算子で区切る
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

// 乗除算で区切る
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

// 加減算で区切る
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

// 不等号で区切る
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

// 等号　等号否定で区切る　
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

// 計算式を代入式　代入演算子で区切る -> 区切られた後は代入式等は出現しない
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

// 予約語のない純粋な計算式として解釈する
Node *expr()
{
    return assign();
}

// 予約語の解釈を行う
Node *stmt()
{
    Node *node;
    if (consume_keyword(TK_RETURN))
    {
        node = create_node(ND_RETURN);
        node->lhs = expr();
        expect(TK_RESERVED, ";");
    }
    else if (consume_keyword(TK_IF))
    {
        node = new_node_if_else();
    }
    else
    {
        node = expr();
        expect(TK_RESERVED, ";");
    }

    return node;
}

// code全体を　;　で区切る
Node *program()
{
    while (!at_eof())
    {
        small_if = 0;
        codes[code_pos++] = stmt();
    }
    codes[code_pos] = NULL;
}