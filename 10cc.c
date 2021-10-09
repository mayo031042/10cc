#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token
{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

Token *token;

typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
} NodeKind;

typedef struct Node Node;
struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

char *user_input;

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op)
{
    // 入力されたopに対して　tokenの種類、長さ、文字自体が一つでも違うとだめです
    if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
        return 0;
    token = token->next;
    return 1;
}

void expect(char *op)
{
    if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
        error("%cではありません\n", op);
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
        error("数値ではありません\n");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        // <, >, <=, >=
        if (!strncmp(p, "==", 2) || !strncmp(p, "!=", 2) || !strncmp(p, "<=", 2) || !strncmp(p, ">=", 2))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        // <, >
        else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>')
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        else if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("tokenizeできません\n");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

// Node指定してRDPをかけるということは　適切な順番にtokenの処理を並び替えるということに等しいと思われる
// 具体的には　nodeの最親を得ることができるので　そのまま木構造に従って再帰的に処理をしていくことになる
// main関数内で行っていたtokenの左端から順番に処理していく方法を外側で行ってみる

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 即値は終端記号である　そのため左右に別頂点を持たない
Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// 今回の計算式の範囲を　（）の中、掛け算グループ、足し算グループに分けることができるということを確認する
// 各オペランド、即値でしかtokenの移動がないことを踏まえると　即値はprimaryのみで操作、それいがいは適宜操作することで　網羅できている気がすれば良さそう？
Node *primary();
Node *unary();
Node *mul();
Node *add();
Node *relational();
Node *equality();
Node *expr();

Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }
    // 即値が入る場合もある　即値用の頂点を作成
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
            // 左辺は元の自分primary　親は乗算　右辺は再帰で求める　以下同様
            // ちょうど１木単位分の操作がprimary内で行われることによって　反対「へ」の字的に進む感じ？　
            // 元のオペランドに　右側にあるオペランドの左足がくっつくいめーじ
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

// 従来のexpr
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

// < ,> ,<= ,>=
Node *relational()
{
    Node *node = add();

    // >=,> の２つは<=, <を用いて書き直す　ことでgenを簡略化
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

// ==, !=
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

Node *expr()
{
    Node *node = equality();
}

void gen(Node *node)
{
    // 終端なら左右を展開しない　スタックに積むだけ
    if (node->kind == ND_NUM)
    {
        printf("    push %d\n", node->val);
        return;
    }

    // 記号なので左右に展開　計算結果がスタックに積まれているはずである
    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    // 当頂点は　この２つに対してND_KINDな操作をすることを期待されたものである
    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;

    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の数が間違っています\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);

    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}