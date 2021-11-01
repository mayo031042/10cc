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
    TK_DO,              // do while
    TK_CONTINUE,        // continue
    TK_BREAK,           // break
    TK_BLOCK_FRONT,     // {
    TK_BLOCK_END,       // }
    TK_EOF,             // 入力の最後
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

// node
typedef enum
{
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_DIV_REM,   // %
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_ASSIGN,    // =
    ND_FUNC_DEF,  // 関数定義
    ND_FUNC_CALL, // 関数呼び出し
    ND_LVAR,      // 変数
    ND_NUM,       // 数値
    ND_RETURN,    // return
    ND_IF,        // if 条件文とcmp 0 ,je
    ND_ELSE,      // else
    ND_FOR_WHILE, // for, while
    ND_DO,        // do
    ND_CONTINUE,  // continue
    ND_BREAK,     // break
    ND_BLOCK,     // ブロックノードの開始を意味する
    ND_NOP,       // node->next で連結したいがNULLでもあってほしい時 → codegen() ではスルーされる
    ND_PUSH_0,    // push 0 のみ行う
    ND_PUSH_1,    // push 1 のみ行う
} NodeKind;

typedef struct Node Node;
struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    Node *next; // ND_BLOCK でのみ使用　最後はND_EOBであるようにする
    // char *name; // func のラベル作成に使用
    int val;
    int offset; // ND_LVARのときのみ使う
    int func_num;
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

typedef struct Func Func;
struct Func
{
    LVar *locals[100]; // 引数→とりあえず無視
    Node *def;         // 定義
    char name[100];    // 関数名
    int len;           // 名前の長さ
    int max_offset;    // プロローグ時に下げるrsp の幅を決める
    bool defined;      // すでに定義がされているか
};


// tokenize のための関数 -> @ tokenize.c
bool current_token_is(TokenKind kind, char *op);
bool consume(TokenKind kind, char *op);
bool expect(TokenKind kind, char *op);
int expect_number();
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
Func *function();

// codegen のための関数　-> @ codegen.c
void code_gen();
void gen(Node *node);

// 分類が半端な関数 @ utils
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
int count();
int stack_front();
void stack_push(int val);
void stack_pop();
int val_of_ident_pos();
void aaa();
// void clear_semicolon();

// グローバル変数 -> 定義はmainにて
extern int token_pos;  // 今見ているtokens の位置
extern int ident_pos;  // 最後に確認した識別子のtoken_pos
extern int func_pos;   // 今見ているfuncs の位置
extern int block_nest; // 今見ているコードの｛｝ネストの階層

extern char *user_input;
extern Token *tokens[];
extern Func *funcs[];
