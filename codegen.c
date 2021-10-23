#include "10cc.h"

// nodeを左辺値とみなせた時　そのアドレスをスタックに積む
void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("右辺値ではありません\n");
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void cmp_rax(int val)
{
    printf("    pop rax\n");
    printf("    cmp rax, %d\n", val);
}

void gen_if(Node *node, int end_label)
{
    gen(node->lhs); // A

    int next_label = count();
    cmp_rax(0);
    printf("    je .L%d\n", next_label);

    gen(node->rhs); // X
    printf("    jmp .Lend%d\n", end_label);

    printf(".L%d:\n", next_label);
}

void gen_else(Node *node, int end_label)
{
    if (node->kind != ND_ELSE)
    {
        gen(node);
        return;
    }
    gen_if(node->lhs, end_label);
    gen_else(node->rhs, end_label);
}

void gen_for_while(Node *node)
{
    int req_label = count();
    int exe_label = count();

    printf("    jmp .Lreq%d\n", req_label);
    printf(".Lexe%d:\n", exe_label);

    gen(node->next->next); // X
    gen(node->next);       // C :whileの場合はND_NULLなので何も出力されない

    printf(".Lreq%d:\n", req_label);
    gen(node); // B :forで空欄の場合　数値の１が入っているとしてparse で処理されている
    cmp_rax(0);
    printf("    jne .Lexe%d\n", exe_label);
}

void gen_do(Node *node)
{
    int exe_label = count();

    printf(".Lexe%d:\n", exe_label);
    gen(node->lhs); // X

    gen(node->rhs); // B
    cmp_rax(0);
    printf("    jne .Lexe%d\n", exe_label);
}

void gen(Node *node)
{
    if (node->kind == ND_NULL)
        return;

    // 数値や変数　終端記号であって左右辺の展開を行わずにreturn したい場合と
    // 代入式　　　変数のアドレスに対して値のコピーをし　その値をstack に保存
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
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
    }

    switch (node->kind)
    {
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_ELSE:
        int end_label = count();
        gen_else(node, end_label);
        printf(".Lend%d:\n", end_label);
        return;
    case ND_FOR:
        gen(node->next); // A
        gen_for_while(node->next->next);
        return;
    case ND_WHILE:
        gen_for_while(node->next);
        return;
    case ND_DO:
        gen_do(node);
        return;
    case ND_BLOCK:
        for (Node *n = node->next; n; n = n->next)
        {
            gen(n);
            printf("    pop rax\n");
        }
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

    // 等号　不等号
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

    // 各頂点で結果はstack に保存される
    printf("    push rax\n");
}
