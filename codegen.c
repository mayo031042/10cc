#include "codegen.h"

void gen(Node *node)
{
    // NOPや 数値のみをpush するnode を処理
    switch (node->kind)
    {
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
    }

    // 変数や関数呼び出しを処理
    switch (node->kind)
    {
    // ストア 代入式
    case ND_ASSIGN:
        gen(node->lhs);
        gen_lval(node->rhs);
        printf("    pop rax\n");
        printf("    pop rdi\n");
        printf("    mov [rax], rdi\n");
        // 代入式自体の評価は　左辺の値に同じ　→　代入式全体の計算結果(=左辺)はstack に積まれる
        printf("    push rdi\n");
        return;
    // ロード
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    // 関数呼び出し
    case ND_FUNC_CALL:
        printf("    mov rax, 0\n");
        printf("    call %s\n", funcs[node->func_num]->name);
        printf("    push rax\n");
        return;
    }

    // ループに関するnode を処理
    switch (node->kind)
    {
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
    }

    switch (node->kind)
    {
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
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_DIV_REM:
        printf("    cqo\n");
        printf("    idiv rdi\n");
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
        printf("    .globl %s\n", funcs[func_pos]->name);
        printf("%s:\n", funcs[func_pos]->name);

        gen_prologue(208);

        gen(funcs[func_pos]->definition);
        printf("    pop rax\n");

        gen_epilogue();
    }
}