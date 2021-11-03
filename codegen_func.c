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

// rax = rax / rdi;
// rdx = rax % rdi;
void gen_div()
{
    printf("    cqo\n");
    printf("    idiv rdi\n");
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

// call 命令前にrsp の値をrsp が16の倍数になるように１回または２回push する
// return されてからrax を変更する前にpop rsp することでpush 回数に関わらずrsp の復元ができる
void gen_func_call(Node *node)
{
    printf("    mov rax, rsp\n");
    printf("    mov rdi, 16\n");
    gen_div();
    // rdx = rax % rdi(16);

    printf("    mov rax, rsp\n");

    int cnt = count();
    printf("    cmp rdx, 0\n");
    printf("    jne .Lcall%d\n", cnt);
    printf("    push rax\n");

    printf(".Lcall%d:\n", cnt);
    printf("    push rax\n");

    printf("    mov rax, 0\n");
    printf("    call %s\n", funcs[node->func_num]->name);
    printf("    pop rsp\n");
    printf("    push rax\n");
}

// 条件式が偽のときのみ次の処理パートへjmp する
// 同一else 群の中で１つでもif 実行文が実行されたときは　else 群の末尾の次にjmp する
// １つのif 内で1push 保証される
void gen_if(Node *node, int end_label)
{
    int next_label = count();

    gen(node->lhs); // B
    cmp_rax(0);
    printf("    je .Lifnext%d\n", next_label);

    gen(node->rhs); // X
    printf("    jmp .Lifend%d\n", end_label);

    printf(".Lifnext%d:\n", next_label);
}

// 同一else 群のif 文を順次処理していく　
// else 終了後で1push 保証されている
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
//
void gen_for_while(Node *node)
{
    stack_push(count());

    // 初期化してから　条件式へjmp
    gen(node->lhs->lhs); // A
    printf("    pop rax\n");
    printf("    jmp .Lreq%d\n", stack_front());

    printf(".Lexe%d:\n", stack_front());

    // 実行文　ここでも必ず1push を保証する
    gen(node->rhs->lhs); // X
    printf("    pop rax\n");

    printf(".Lcont%d:\n", stack_front()); // continue先 → 変化式の前

    gen(node->lhs->rhs); // C
    printf("    pop rax\n");

    printf(".Lreq%d:\n", stack_front());

    // 条件式を実行、評価して　実行文に移動するかを決める
    gen(node->rhs->rhs); // B
    cmp_rax(0);
    printf("    jne .Lexe%d\n", stack_front());

    printf(".Lbrk%d:\n", stack_front());

    // for, while 全体でも1push を保証
    printf("    push rax\n");

    stack_pop();
}

void gen_do_while(Node *node)
{
    stack_push(count());

    printf(".Lexe%d:\n", stack_front());
    gen(node->lhs);          // X
    printf("    pop rax\n"); // x の揃えポップ　→　continue ではすでに揃っているのでスキップ

    printf(".Lcont%d:\n", stack_front()); // continue 先 contiの有無に関わらずスタックトップは揃っている

    gen(node->rhs); // B
    cmp_rax(0);
    printf("    jne .Lexe%d\n", stack_front());

    printf(".Lbrk%d:\n", stack_front()); // break先　有無に関わらずスタックトップは揃っている

    printf("    push rax\n");

    stack_pop();
}

// build_block() とgen_block() でのみ block_nest の値をいじる
// 意味のある最初のnode からNULL になる直前までをnext 順にgen()していく
void gen_block(Node *node)
{
    add_block_nest();

    for (; node; node = node->next)
    {
        gen(node);
        printf("    pop rax\n");
    }

    printf("    push rax\n");

    sub_block_nest();
}
