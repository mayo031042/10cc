#include "codegen.h"

void gen(Node *node)
{
    switch (node->kind)
    {
    // NOP か数値を積む
    case ND_NOP:
        return;
    case ND_PUSH_1:
        pf("    push 1\n");
        return;
    case ND_PUSH_0:
        pf("    push 0\n");
        return;
    case ND_NUM:
        pf("    push %d\n", node->val);
        return;

    // 変数の保持している値を積む
    // 変数をちょうど１つ含むようなnode であることが確定している
    case ND_LVAR:
        gen_addr(node);
        gen_cng_addr_to_imm(node);
        return;
    // *x 変数の保持している値をメモリであると解釈して積む
    // PTR 型を指していることが確定している
    case ND_DEREF:
        gen_deref(node);
        gen_cng_addr_to_imm(node);
        return;

    // &x 変数の割り当てられているアドレスを返す
    case ND_ADDR:
        gen_addr(node->lhs);
        return;

    // 代入 適切なアドレスに保持されている値を書き換え結果を積む
    case ND_ASSIGN:
        gen(node->lhs);
        gen_deref(node->rhs);
        gen_mov_imm_to_addr(node->rhs);
        return;

    // 関数呼び出し
    case ND_FUNC_CALL:
        gen_func_call(node);
        return;

    // ループ
    case ND_FOR_WHILE:
        gen_for_while(node);
        return;
    case ND_DO:
        gen_do_while(node);
        return;
    case ND_CONTINUE:
        pf("    jmp .Lcont%d\n", stack_front());
        return;
    case ND_BREAK:
        pf("    jmp .Lbrk%d\n", stack_front());
        return;

    // その他
    case ND_RETURN:
        gen(node->lhs);
        pf("    pop rax\n");
        gen_epilogue();
        return;
    case ND_ELSE:
        int end_label = count();
        gen_else(node, end_label);
        pf(".Lifend%d:\n", end_label);
        return;
    case ND_BLOCK:
        gen_block(node->lhs);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    // raxが左辺　rdiが右辺
    pf("    pop rdi\n");
    pf("    pop rax\n");

    // 左右辺の展開をした後にstack に積まれた結果について関係を記述する
    switch (node->kind)
    {
    // 四則演算
    case ND_ADD:
        gen_add(node);
        break;
    case ND_SUB:
        gen_sub(node);
        break;
    case ND_MUL:
        gen_mul();
        break;
    case ND_DIV:
        gen_div();
        break;
    case ND_DIV_REM:
        gen_div();
        pf("    mov rax, rdx\n");
        break;

    // 不等号
    case ND_EQ:
        pf("    cmp rax, rdi\n");
        pf("    sete al\n");
        pf("    movzb rax, al\n");
        break;
    case ND_NE:
        pf("    cmp rax, rdi\n");
        pf("    setne al\n");
        pf("    movzb rax, al\n");
        break;
    case ND_LT:
        pf("    cmp rax, rdi\n");
        pf("    setl al\n");
        pf("    movzb rax, al\n");
        break;
    case ND_LE:
        pf("    cmp rax, rdi\n");
        pf("    setle al\n");
        pf("    movzb rax, al\n");
        break;
    }

    pf("    push rax\n");
}

// 関数の定義部分を出力 main も含まれる
void code_gen()
{
    pf(".intel_syntax noprefix\n");

    for (func_pos = 0; funcs[func_pos]; func_pos++)
    {
        gen_prologue();

        gen(funcs[func_pos]->definition);
        pf("    pop rax\n");

        gen_epilogue();
    }
}