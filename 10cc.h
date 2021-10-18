#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// error -> @ codegen.c
void error(char *fmt, ...);

// token　
typedef enum
{
    TK_RESERVED,
    TK_IDENT,
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

// tokenize のための関数 -> @ tokenize.c
bool consume(char *op);
void expect(char *op);
int expect_number();
Token *consume_ident();
bool at_eof();
void *tokenize();

// node
typedef enum
{
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_ASSIGN, // =
    ND_LVAR,   // 変数
    ND_NUM,    // 数値
} NodeKind;

typedef struct Node Node;
struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset; // kindがND_LVARのときのみ使う
};

// parse のための関数 -> @ parse.c
Node *program();

// LVar
typedef struct LVar LVar;
struct LVar
{
    LVar *next;
    char *name;
    int len;
    int offset; // RBPからの距離
};

// codegen のための関数　-> @ codegen.c
void gen(Node *node);

// グローバル変数 -> 定義はmainにて
extern char *user_input;
extern Token *token;
extern Node *code[100];
extern LVar *locals;