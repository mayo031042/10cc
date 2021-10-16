#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *fmt, ...);

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

extern Token *token;
extern char *user_input;

bool consume(char *op);
void expect(char *op);
int expect_number();
Token *consume_ident();

bool at_eof();
void *tokenize();

typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_ASSIGN,
    ND_LVAR,
    ND_NUM,
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

Node *program();
extern Node *code[100];

typedef struct LVar LVar;
struct LVar{
    LVar *next;
    char *name;
    int len;
    int offset;
};

extern LVar *locals;

void gen(Node *node);