#include "codegen.h"

// スタックトップと引数をcmp する 条件分岐jmp 命令の直前に使用
void cmp_rax(int val)
{
    printf("    pop rax\n");
    printf("    cmp rax, %d\n", val);
}

// rbp, rsp を準備し引数の分だけローカル変数用のメモリを確保する
void gen_prologue(int sz)
{
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", sz);
}

// rbp, rsp をもとに関数呼び出し前に戻しreturn する
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

// 条件式が偽のときのみ次の処理パートへjmp する
// 同一else 群の中で１つでもif 実行文が実行されたときは　else 群の末尾の次にjmp する
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

// 同一else 群のif 文を順次処理していく　
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

// 初期化式、条件式、(jmp)、実行式、変化式の順に処理する
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

void gen_do_while(Node *node)
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

// build_block() とgen_block() でのみ block_nest の値をいじる
// 意味のある最初のnode からNULL になる直前までをnext 順にgen()していく
void gen_block(Node *node)
{
    block_nest++;

    for (; node; node = node->next)
    {
        gen(node);
        printf("    pop rax\n");
    }

    printf("    push rax\n");

    block_nest--;
}
