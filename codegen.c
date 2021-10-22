#include "10cc.h"

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void err(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

// nodeを左辺値とみなせた時　そのアドレスをスタックに積む
void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("右辺値ではありません\n");
    // stackに変数の指すアドレスをpush
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen_if(Node *node)
{
    gen(node->lhs); // A
    // stack top が０であるときのみ偽と判定して goto next_label:
    printf("    pop rax\n");
    printf("    cmp rax, 0\n");
    printf("    je .L%02d%02d\n", node->end_label, node->next_label);

    // 直前でjmpしていたら実行されない部分に　実行式
    gen(node->rhs); // X
    // 実行式が実行された時は　if ネストを抜ける
    printf("    jmp .Lend%02d\n", node->end_label);

    // ちょうど実行文１つだけを挟んで　ラベルを配置
    printf(".L%02d%02d:\n", node->end_label, node->next_label);
}

void gen_else(Node *node)
{
    if (node->kind != ND_ELSE)
    {
        gen(node);
        return;
    }
    gen_if(node->lhs);
    gen_else(node->rhs);
}

void gen(Node *node)
{
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
        gen_else(node);
        printf(".Lend%02d:\n", node->end_label);
        return;
    // if に入るのは　単純if なときのみ
    case ND_IF:
        gen_if(node);
        printf(".Lend%02d:\n", node->end_label);
        return;
    case ND_BLOCK:
        node = node->next;
        while (node->kind != ND_EOB)
        {
            gen(node);
            printf("    pop rax\n");
            node = node->next;
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
