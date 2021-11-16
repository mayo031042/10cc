#include "10cc.h"
// codegen を補助する関数

void gen(Node *node);
void pf(char *fmt, ...);

void cmp_rax(int val);
void gen_prologue();
void gen_epilogue();

void gen_add(Node *node);
void gen_sub(Node *node);
void gen_mul();
void gen_div();

void gen_declare(Node *node);
void gen_addr(Node *node);
void gen_deref(Node *node);
void gen_func_call(Node *node);

void gen_if(Node *node, int end_label);
void gen_else(Node *node, int end_label);

void gen_for_while(Node *node);
void gen_do_while(Node *node);

void gen_block(Node *node);

// 型に対応した適切なサイズでデータの変換、転送を行う
void gen_cng_addr_to_imm(Node *node);
void gen_mov_imm_to_addr(Node *node);

void gen_cast(Node *node);