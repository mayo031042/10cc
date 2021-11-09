#include "10cc.h"

// tokens[] を見てnode 作成を補助する関数
bool current_token_is(TokenKind kind, char *op);
bool consume(TokenKind kind, char *op);
bool consume_keyword(TokenKind kind);
bool consume_ident();
bool current_token_is_type();

bool expect(TokenKind kind, char *op);
TypeKind expect_vartype();
bool expect_ident();
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

LVar *new_lvar();
LVar *find_lvar_within_block(int depth);
LVar *find_lvar();
Node *declare_lvar();
void declare_arg(); // 関数宣言、定義の際に 引数表をセットする
Node *build_arg();  // 関数呼び出しのときに　引数をexpr() 解釈して登録先のレジスタを記憶する
void consume_arg();

int find_func();
Func *new_func(Token *tok);
