#include "10cc.h"

// tokens[] を見てnode 作成を補助する関数

// token の判別だけを行いtoken_pos を進めない関数 -> current...
bool current_token_is(TokenKind kind, char *op);
bool current_token_is_type();

// token の判別を行いtoken_pos を進める関数 -> consume...
bool consume(TokenKind kind, char *op);
bool consume_keyword(TokenKind kind);
bool consume_ident();

// token の断定を行いtoken_pos を進める関数 -> expect...
bool expect(TokenKind kind, char *op);
bool expect_ident();
TypeKind expect_vartype();
int expect_number();

bool at_eof();
int val_of_ident_pos();

// RDP 関数
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

// RDP 補助関数
Node *create_node(NodeKind kind);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(LVar *lvar);
Node *new_node_if();
Node *new_node_else();

Node *new_node_for();
Node *new_node_while();
Node *new_node_do();

Node *build_block();

// 変数を解釈
LVar *new_lvar();
LVar *find_lvar_within_block(int depth);
LVar *find_lvar();
Node *declare_lvar();

// 引数を解釈
void declare_arg(); // 関数の定義時 引数表を登録する
void consume_arg(); // 関数の宣言時 引数表を読み飛ばす
Node *build_arg();  // 関数呼び出しのときに　引数をexpr() 解釈して登録先のレジスタを割り当てる

// 関数を解釈
int find_func();
Func *new_func(Token *tok);
