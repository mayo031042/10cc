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
};

Token *token;

typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;
struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return 0;
    token = token->next;
    return 1;
}

void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
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

Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
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

        if (*p == '+' || *p == '-')
        {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("tokenizeできません\n");
    }

    new_token(TK_EOF, cur, p);
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

// 以下解析関数では　行うのはtokenの頂点化と適切な順序での結合である　したがってアセンブリの出力は別

// exprからはNodeを返すようにする　今回の実装方法では右辺と左辺を備えたNodeを返すがreturnを
// 繰り返すうちに親に近づいていくことを考えると　最上層のexprの返り値は　構文木全体の最親であることがわかる

// とりあえず　整数とその加減算のみ（優先処理もない）を実装する
// 相変わらずtokenの操作（移動）についてはconsumeやexpect系がやってくれる　→役割分担大事　tokenのことを考えなくて良くなっている
// つまり　tokenの移動は各計算記号かprimaryでの即値でしか　実行されないということ

// 今回の計算式の範囲を　（）の中、掛け算グループ、足し算グループに分けることができるということを確認する
// 各オペランド、即値でしかtokenの移動がないことを踏まえると　即値はprimaryのみで操作、それいがいは適宜操作することで　網羅できている気がすれば良さそう？
Node *primary();
Node *mul();
Node *expr();

// 即値か（）を担当　即値なら終端　（）なら数式全体に戻る
Node *primary()
{
    if (consume('('))
    {
        Node *node = expr();
        expect(')');
        return node;
    }
    // 即値が入る場合もある　即値用の頂点を作成
    return new_node_num(expect_number());
}

// 乗除算を担当　乗除は（）の優先式か即値のみを対象にする
Node *mul()
{
    Node *node = primary();
    // mul=primary (('*' | '/' ) primary)* なのでまず初めにprimaryをみる
    for (;;)
    {
        if (consume('*'))
        {
            // 左辺は元の自分primary　親は乗算　右辺は再帰で求める　以下同様
            // ちょうど１木単位分の操作がprimary内で行われることによって　反対「へ」の字的に進む感じ？　
            // 元のオペランドに　右側にあるオペランドの左足がくっつくいめーじ
            node = new_node(ND_MUL, node, primary());
        }
        else if (consume('/'))
        {
            node = new_node(ND_DIV, node, primary());
        }
        else
            return node;
    }
}

// 加減算を担当　加減は乗除算や（）を対象にする
Node *expr()
{
    Node *node = mul();

    for (;;)
    {
        if (consume('+'))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if (consume('-'))
        {
            node = new_node(ND_SUB, node, mul());
        }
        return node;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の数が間違っています\n");
        return 1;
    }

    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("    mov rax, %d\n", expect_number());

    while (!at_eof())
    {
        if (consume('+'))
        {
            printf("    add rax, %d\n", expect_number());
            continue;
        }
        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}