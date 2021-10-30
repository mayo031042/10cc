#include "10cc.h"

void cmp_rax(int val)
{
    printf("    pop rax\n");
    printf("    cmp rax, %d\n", val);
}

void gen_prologue(int sz)
{
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", sz);
}

void gen_epilogue()
{
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}

// nodeを左辺値とみなせた時　そのアドレスをスタックに積む
void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
    {
        error("右辺値ではありません\n");
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen_if(Node *node, int end_label)
{
    gen(node->lhs); // B

    int next_label = count();
    cmp_rax(0);
    printf("    je .Lifnext%d\n", next_label);

    gen(node->rhs); // X
    printf("    jmp .Lifend%d\n", end_label);

    printf(".Lifnext%d:\n", next_label);
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
    stack_push(count());

    gen(node->lhs->lhs); // A
    printf("    jmp .Lreq%d\n", stack_front());

    printf(".Lexe%d:\n", stack_front());
    gen(node->rhs->lhs); // X

    printf(".Lcont%d:\n", stack_front()); // continue先
    gen(node->lhs->rhs);                  // C :whileの場合はND_NULLなので何も出力されない

    printf(".Lreq%d:\n", stack_front());
    gen(node->rhs->rhs); // B :forで空欄の場合　数値の１が入っているとしてparse で処理されている

    cmp_rax(0);
    printf("    jne .Lexe%d\n", stack_front());
    printf(".Lbrk%d:\n", stack_front());

    stack_pop();
}

void gen_do(Node *node)
{
    stack_push(count());

    printf(".Lexe%d:\n", stack_front());
    gen(node->lhs); // X

    printf(".Lcont%d:\n", stack_front()); // continue 先
    gen(node->rhs);                       // B

    cmp_rax(0);
    printf("    jne .Lexe%d\n", stack_front());
    printf(".Lbrk%d:\n", stack_front());

    stack_pop();
}

// 意味のある最初のnode からNULL になる直前までをnext 順にgen()していく
void gen_loop(Node *node)
{
    for (; node; node = node->next)
    {
        gen(node);
        printf("    pop rax\n");
    }
}

void gen(Node *node)
{
    if (node->kind == ND_NULL)
    {
        return;
    }

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

    case ND_FUNC_CALL:
        printf("mov rax, 0\n");
        printf("call %s\n", funcs[node->func_pos]->name);
        printf("push rax\n");
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
        printf(".Lifend%d:\n", end_label);
        return;
    case ND_FOR_WHILE:
        gen_for_while(node);
        return;
    case ND_DO:
        gen_do(node);
        return;
    case ND_CONTINUE:
        printf("    jmp .Lcont%d\n", stack_front());
        return;
    case ND_BREAK:
        printf("    jmp .Lbrk%d\n", stack_front());
        return;
    case ND_BLOCK:
        gen_loop(node->lhs);
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

    for (int i = 0; funcs[i]; i++)
    {

        printf("    .globl %s\n", funcs[i]->name);
        printf("%s:\n", funcs[i]->name);

        gen_prologue(208);

        gen_loop(funcs[i]->def);

        // printf("    mov rax, 0\n");
        gen_epilogue();
    }
}