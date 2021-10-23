#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// token　
typedef enum
{
    TK_RESERVED,        // 演算子 +-*/, ==, !=, <, <=
    TK_ASSIGN_RESERVED, // 代入演算子 +=,,,
    TK_IDENT,           // 変数
    TK_NUM,             // 数値
    TK_RETURN,          // return
    TK_IF,              // if
    TK_ELSE,            // else
    TK_WHILE,           // while
    TK_FOR,             // for
    TK_BLOCK_FRONT,     // {
    TK_BLOCK_END,       // }
    TK_EOF,             // 入力の最後
} TokenKind;

typedef struct Token Token;
struct Token
{
    TokenKind kind;
    Token *next;
    int pos; // 自分がtokens　の何番目か
    int val;
    char *str;
    int len;
};

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
    ND_RETURN, // return
    ND_IF,     // if 条件文とcmp 0 ,je
    ND_ELSE,   // else
    ND_WHILE,  // while
    ND_FOR,    // for
    ND_BLOCK,  // ブロックノードの開始を意味する
    ND_NULL,   // node->next で連結したいがNULLでもあってほしい時 → codegen() ではスルーされる
} NodeKind;

typedef struct Node Node;
struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    Node *next; // ND_BLOCK でのみ使用　最後はND_EOBであるようにする
    int val;
    int offset; // ND_LVARのときのみ使う
};

// LVar
typedef struct LVar LVar;
struct LVar
{
    LVar *next;
    char *name;
    int len;
    int offset; // RBPからの距離
};

// error -> @ codegen.c
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void err(char *fmt, ...);

// tokenize のための関数 -> @ tokenize.c
bool consume(TokenKind kind, char *op);
bool expect(TokenKind kind, char *op);
int expect_number();
int consume_ident();
bool consume_keyword(TokenKind kind);
bool at_eof();
void *tokenize();

// parse のための関数 -> @ parse.c
Node *primary();
Node *unary();
Node *mul();
Node *add();
Node *relational();
Node *equality();
Node *assign();
Node *expr();
Node *stmt();
Node *program();

// codegen のための関数　-> @ codegen.c
void gen(Node *node);

// 分類が半端な関数 @ func_others
int count();
// void clear_semicolon();

// グローバル変数 -> 定義はmainにて
extern int pos;
extern char *user_input;
extern Node *codes[];
extern Token *tokens[];
extern LVar *locals;