#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "10cc.h"
Node *code[100];

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(int offset)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = offset;
    return node;
}

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
    // 変数の場合 offset != -1
    int offset = consume_ident();
    if (offset != -1)
    {
        return new_node_ident(offset);
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

Node *relational()
{
    Node *node = add();

    for (;;)
    {
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

Node *stmt()
{
    Node *node = expr();
    // expect(";");
    return node;
}

Node *program()
{
    int i = 0;
    while (!at_eof())
        code[i++] = stmt();
    code[i] = NULL;
}