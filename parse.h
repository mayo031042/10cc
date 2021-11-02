#include "10cc.h"

// tokens[] を見てnode 作成を補助する関数
bool current_token_is(TokenKind kind, char *op);
bool consume(TokenKind kind, char *op);
bool expect(TokenKind kind, char *op);
int expect_number();
bool consume_keyword(TokenKind kind);
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

LVar *new_lvar(int max_offset);
LVar *find_lvar_from_cur_block(LVar *lvar);
LVar *find_lvar();

int find_func(bool serach_only);
Func *new_func(Token *tok);
