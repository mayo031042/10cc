#include "10cc.h"

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
Node *new_node_block();
Node *build_block();

LVar *new_lvar(int max_offset);
LVar *find_lvar();

int find_func(bool serach_only);
Func *new_func(Token *tok);