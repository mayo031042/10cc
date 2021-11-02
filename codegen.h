#include "10cc.h"
// codegen を補助する関数

void gen(Node *node);

void cmp_rax(int val);
void gen_prologue(int sz);
void gen_epilogue();

void gen_div();

void gen_lval(Node *node);
void gen_func_call(Node *node);

void gen_if(Node *node, int end_label);
void gen_else(Node *node, int end_label);

void gen_for_while(Node *node);
void gen_do_while(Node *node);

void gen_block(Node *node);