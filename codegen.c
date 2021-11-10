#include "codegen.h"

void gen(Node *node)
{
    switch (node->kind)
    {
    // NOP か数値を積む
    case ND_NOP:
        return;
    case ND_PUSH_1:
        printf("    push 1\n");
        return;
    case ND_PUSH_0:
        printf("    push 0\n");
        return;
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;

    // 変数や関数について
    case ND_LVAR:
        gen_addr(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    // *x 続くunary() をアドレス計算式として計算し　結果を左辺値のアドレスとしてpush する
    case ND_DEREF:
        gen_deref(node);
        return;
    // &x　
    case ND_ADDR:
        gen_addr(node->lhs);
        return;

    // 代入 適切なアドレスの値を書き換え結果を積む
    case ND_ASSIGN:
        gen(node->lhs);
        gen_deref(node->rhs);
        printf("    pop rax\n");
        printf("    pop rdi\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
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
        printf("    jmp .Lcont%d\n", stack_front());
        return;
    case ND_BREAK:
        printf("    jmp .Lbrk%d\n", stack_front());
        return;

    // その他
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        gen_epilogue();
        return;
    case ND_ELSE:
        int end_label = count();
        gen_else(node, end_label);
        printf(".Lifend%d:\n", end_label);
        return;
    case ND_BLOCK:
        gen_block(node->lhs);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);
    // raxが左辺　rdiが右辺
    printf("    pop rdi\n");
    printf("    pop rax\n");

    // 左右辺の展開をした後にstack に積まれた結果について関係を記述する
    switch (node->kind)
    {
    // 四則演算
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        gen_div();
        break;
    case ND_DIV_REM:
        gen_div();
        printf("    mov rax, rdx\n");
        break;

    // 不等号
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}

// 関数の定義部分を出力 main も含まれる
void code_gen()
{
    printf(".intel_syntax noprefix\n");

    for (func_pos = 0; funcs[func_pos]; func_pos++)
    {
        gen_prologue();

        gen(funcs[func_pos]->definition);
        printf("    pop rax\n");

        gen_epilogue();
    }
}