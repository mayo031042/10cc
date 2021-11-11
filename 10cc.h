#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// token　
typedef enum
{
    TK_OPERATOR,        // 演算子,* ,&
    TK_RESERVED,        // ==, !=, <, <= ,;()
    TK_ASSIGN_OPERATOR, // 代入演算子 +=,,,
    TK_IDENT,           // 変数
    TK_NUM,             // 数値
    TK_SIZEOF,          // sizeof
    TK_INT,             // int型
    TK_RETURN,          // return
    TK_SWITCH,          // switch
    TK_CASE,            // case
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

// LVar
typedef enum
{
    CHAR, // 1
    INT,  // 4
    PTR,  // 8
} TypeKind;

typedef struct Type Type;
struct Type
{
    TypeKind kind;
    Type *ptr_to;
};

typedef struct LVar LVar;
struct LVar
{
    Type *type; // 変数の型
    LVar *next;
    char *name;
    int len;
    int offset; // RBPからの距離
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
    ND_FUNC_CALL, // 関数呼び出し
    ND_LVAR,      // 変数
    ND_ADDR,      // address
    ND_DEREF,     // dereference
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
    Node *next;   // ND_BLOCK でのみ使用　最後はNULL であるようにする
    int offset;   // ND_LVAR でのみ使用
    Type *type;
    LVar *lvar;   // ND_LVAR でのみ使用
    int func_num; // ND_FUNC_CALL でのみ使用
    int val;
};

// Func
typedef struct Func Func;
struct Func
{
    Type *type;     // 関数の型  
    LVar *locals[100]; // ブロック深度ごとの変数列　locals[0] と関数の引数　が常に一致するよう実装
    Node *definition;  // 定義
    char name[100];    // 関数名
    bool defined;      // すでに定義がされているか→definition がNULLかだけでは判定できない
    int len;           // 名前の長さ
    int max_offset;    // プロローグ時に下げるrsp の幅を決める
};

// tokenize のための関数 -> @ tokenize.c
void *tokenize();

// parse のための関数 -> @ parse.c
void *function();

// codegen のための関数　-> @ codegen.c
void code_gen();

// 分類が半端な関数 -> @ utils
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
int count();

int stack_front();
void stack_push(int val);
void stack_pop();

int val_of_block_nest();
void add_block_nest();
void sub_block_nest();

void aaa();

int size_of_node(Node *node);
int size_of(Type *type);
char *char_of(Type *type);

// グローバル変数 -> 定義はmainにて
extern int token_pos;    // 今見ているtokens の位置
extern int func_pos;     // 今見ているfuncs の位置
extern int block_nest;   // 今見ているコードの｛｝ネストの階層
extern char *user_input; // main関数の引数を保持すptr

extern Token *tokens[];
extern Func *funcs[];
