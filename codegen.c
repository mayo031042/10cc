#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "10cc.h"

// nodeを右辺値とみなせた時　そのアドレスをスタックに積む
void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("右辺値ではありません");
    // stackに変数の指すアドレスをpush
    printf("    mov rax rbp\n");
    printf("    sub rax %d", node->offset);
    printf("    push rax");
}

void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    // ストア
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("    pop rdi");
        printf("    pop rax");
        printf("    mov [rax], rdi");
        // 代入式自体の評価は　左辺値に同じ
        printf("    mov rdi");
        return;
    // ロード
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax");
        printf("    mov rax, [rax]");
        printf("    push rax");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
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
