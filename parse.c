#include "10cc.h"

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

// 完全に独立してif node を完成させる
Node *new_node_if()
{
    Node *node = create_node(ND_IF);
    expect(TK_RESERVED, "(");
    node->lhs = expr();
    expect(TK_RESERVED, ")");
    node->rhs = stmt();
    return node;
}

// if の時点で else を作る　else node　の左辺にif を配置 右辺にはNULLかstmt()
Node *new_node_else()
{
    // 既にif があることがわかっていて消費されている
    Node *node = create_node(ND_ELSE);
    node->lhs = new_node_if();
    // if, else if で終了しないなら
    if (consume_keyword(TK_ELSE))
    {
        // else if なら
        if (consume_keyword(TK_IF))
            node->rhs = new_node_else();
        else
            node->rhs = stmt();
    }
    else
        node->rhs = create_node(ND_NULL);
    return node;
}

Node *new_node_block()
{
    if (consume_keyword(TK_BLOCK_END))
        return NULL;
    Node *node = stmt();
    node->next = new_node_block();
    return node;
}

// lvar name,len
LVar *create_lvar(int now_pos)
{
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->name = tokens[now_pos]->str;
    lvar->len = tokens[now_pos]->len;
    return lvar;
}

// なかったらNULL あったらLVar を返す
LVar *find_lvar()
{
    int now_pos = pos - 1;
    LVar *lvar;

    for (lvar = locals; lvar; lvar = lvar->next)
    {
        if (lvar->len == tokens[now_pos]->len && !memcmp(lvar->name, tokens[now_pos]->str, lvar->len))
            return lvar;
    }

    lvar = create_lvar(now_pos);

    // 変数がまだ何も登録されていない時　offsetを初期化する
    if (locals == NULL)
    {
        lvar->offset = 0;
    }
    // localsは既出変数配列の末尾変数を指しているのでlvar はlocals を参照すれば良い
    else
    {
        lvar->offset = locals->offset + 8;
    }

    lvar->next = locals;
    locals = lvar;

    return lvar;
}

// programの中の最小単位 (expr)か数値か変数しかありえない　
// 演算子は処理されているので　残るは数値等　のみである
Node *primary()
{
    Node *node;
    // : ()
    if (consume(TK_RESERVED, "("))
    {
        node = expr();
        expect(TK_RESERVED, ")");
    }
    // 変数
    else if (consume_keyword(TK_IDENT))
        node = new_node_ident(find_lvar());
    // 数値
    else
        node = new_node_num(expect_number());

    return node;
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
        else if (consume(TK_ASSIGN_RESERVED, "/="))
            node = new_node(ND_ASSIGN, new_node(ND_DIV, node, assign()), node);
        else
            error_at(tokens[pos]->str, "代入演算子ではありません\n");
    }

    return node;
}

// 予約語のない純粋な計算式として解釈する 三項間演算子もここ？
Node *expr()
{
    return assign();
}

// 予約語,{} の解釈を行う
Node *stmt()
{
    Node *node;

    // : return,
    if (consume_keyword(TK_RETURN))
    {
        node = create_node(ND_RETURN);
        node->lhs = expr();
        expect(TK_RESERVED, ";");
    }
    // : if
    else if (consume_keyword(TK_IF))
    {
        node = new_node_else();
    }
    // : for
    else if (consume_keyword(TK_FOR))
    {
        Node *nodes[3];
        expect(TK_RESERVED, "(");

        char *op[] = {";", ";", ")"};
        for (int i = 0; i < 3; i++)
        {
            if (consume(TK_RESERVED, op[i]))
                nodes[i] = create_node(ND_NULL);
            else
            {
                nodes[i] = expr();
                expect(TK_RESERVED, op[i]);
            }
        }

        // 条件式が空欄な時は恒真式とみなすので　１が入っているとしてparseする
        if (nodes[1]->kind == ND_NULL)
        {
            nodes[1]->kind = ND_NUM;
            nodes[1]->val = 1;
        }

        Node *node_right = new_node(ND_NULL, stmt(), nodes[1]);
        Node *node_left = new_node(ND_NULL, nodes[0], nodes[2]);
        node = new_node(ND_FOR_WHILE, node_left, node_right);
    }
    // : while
    else if (consume_keyword(TK_WHILE))
    {
        expect(TK_RESERVED, "(");
        Node *node_B = expr();
        expect(TK_RESERVED, ")");
        Node *node_right = new_node(ND_NULL, stmt(), node_B);
        Node *node_left = new_node(ND_NULL, create_node(ND_NULL), create_node(ND_NULL));
        node = new_node(ND_FOR_WHILE, node_left, node_right);
    }
    // : do{} while();
    else if (consume_keyword(TK_DO))
    {
        Node *lhs = stmt();
        expect(TK_WHILE, "while");
        expect(TK_RESERVED, "(");
        Node *rhs = expr();
        expect(TK_RESERVED, ")");
        expect(TK_RESERVED, ";");
        node = new_node(ND_DO, lhs, rhs);
    }
    // : continue
    else if (consume_keyword(TK_CONTINUE))
    {
        node = create_node(ND_CONTINUE);
        expect(TK_RESERVED, ";");
    }
    // : break;
    else if (consume_keyword(TK_BREAK))
    {
        node = create_node(ND_BREAK);
        expect(TK_RESERVED, ";");
    }
    // : {}
    else if (consume_keyword(TK_BLOCK_FRONT))
    {
        node = new_node(ND_BLOCK, new_node_block(), NULL);
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
    int i = 0;
    while (!at_eof())
        codes[i++] = stmt();

    codes[i] = NULL;
}