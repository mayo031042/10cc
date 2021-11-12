#include "10cc.h"

#define MAX_STACK_SIZE 100

int Stack[MAX_STACK_SIZE];
int StackTop = 0;
int block_nest = 0;

// エラー処理

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void aaa()
{
    fprintf(stderr, "\n\tnow-> %s\n", tokens[token_pos]->str);
}

// 重要な変数の操作

int count()
{
    static int cnt = 2;
    return cnt++;
}

int stack_front()
{
    if (!StackTop)
        error("ループ構文内ではありません");
    return Stack[StackTop];
}

void stack_push(int val)
{
    StackTop++;
    if (MAX_STACK_SIZE <= StackTop)
        error("Stack size overflowed.");
    Stack[StackTop] = val;
}
void stack_pop()
{
    if (!StackTop)
        error("Stack size underflowed.");
    StackTop--;
}

int val_of_block_nest()
{
    return block_nest;
}

void add_block_nest()
{
    block_nest++;
}

void sub_block_nest()
{
    block_nest--;
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

int size_of(Type *type)
{
    switch (type->kind)
    {
    case CHAR:
        return 1;
    case INT:
        return 4;
    case PTR:
        return 8;
    }
}

char *char_of(Type *type)
{
    switch (type->kind)
    {
    case CHAR:
        return "BYTE\0";
    case INT:
        return "DWORD\0";
    case PTR:
        return "QWORD\0";
    }
}
