#include "codegen.h"

char *regi64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *regi32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

// スタックトップと引数をcmp する 条件分岐jmp 命令の直前に使用
void cmp_rax(int val)
{
    printf("    pop rax\n");
    printf("    cmp rax, %d\n", val);
}

// スタックトップをアドレスと解釈し　そのアドレスに入っている値に変更する
// そのアドレスがどのサイズの型に対応しているかを判断し適切なアセンブリを出力する
void gen_cng_addr_to_imm(Node *node)
{
    printf("    pop rax\n");

    switch (size_of_node(node))
    {
    case 1:
        printf("    movsx rax, BYTE PTR [rax]\n");
        break;
    case 4:
        printf("    movsx rax, DWORD PTR [rax]\n");
        break;
    case 8:
        printf("    mov rax, [rax]\n");
        break;
    }

    printf("    push rax\n");
}

// スタックから２つ取り出し　変数に対応した適切なサイズ分　メモリに移動する
void gen_mov_imm_to_addr(Node *node)
{
    printf("    pop rax\n");
    printf("    pop rdi\n");

    switch (size_of_node(node))
    {
    case 1:
        printf("    mov BYTE PTR [rax], di\n");
        break;
    case 4:
        printf("    mov DWORD PTR [rax], edi\n");
        break;
    case 8:
        printf("    mov [rax], rdi\n");
        break;
    }

    printf("    push rdi\n");
}

// 適切にレジスタへ解釈された引数が渡されている前提のうえで　レジスタからメモリに値を移す
void pop_regi()
{
    int i = 0;
    for (LVar *lvar = funcs[func_pos]->locals[0]; lvar; lvar = lvar->next)
    {
        printf("    mov DWORD PTR -%d[rbp], %s\n", 4 * (i + 1), regi32[i]);
        // printf("    mov QWORD PTR -%d[rbp], %s\n", 8 * (i + 1), regi64[i]);
        i++;
    }
}

// rbp, rsp を準備する
// これが実行されるときは必ず関数の先頭にあるので　func_pos を参照して良い
void gen_prologue()
{
    printf("    .globl %s\n", funcs[func_pos]->name);
    printf("%s:\n", funcs[func_pos]->name);

    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", funcs[func_pos]->max_offset);

    pop_regi();
}

// rbp, rsp をもとに関数呼び出し前に戻しreturn する
void gen_epilogue()
{
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}

// nodeを左辺値とみなせた時　そのアドレスをスタックに積む
// addr, deref もある変数からアドレスを計算して　アドレスのまま積むという点では同じ
// addr は変数として割り当てられている領域のみを対象とする点で異なる
void gen_addr(Node *node)
{
    if (node->kind != ND_LVAR)
    {
        error_at(tokens[token_pos]->str, "左辺値ではありません\n");
    }

    // rbp とoffsset からアドレスを計算し積む
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

// unary() を計算する　結果をアドレスとして返す
// 呼び出しの際　呼び出しもとで参照は外されない（ND_DEREFのまま引数に渡される）
// addr, deref もある変数からアドレスを計算して　アドレスのまま積むという点では同じ
// deref は変数として割り当てられていないようなアドレスも積むことができる
// assign で参照外しとアドレス積みのどちらにも対応する必要があるので両対応で実装
void gen_deref(Node *node)
{
    // 参照が外れきったら変数のアドレスを積む
    if (node->kind == ND_LVAR)
    {
        gen_addr(node);
        return;
    }

    // 変数でないなら参照であるはずである
    if (node->kind != ND_DEREF)
    {
        error_at(tokens[token_pos]->str, "参照が正しくありません");
    }

    // 参照を１つ外す　まだ参照が続くならばgen() 内でまたgen_deref() が呼ばれる
    gen(node->lhs);
}

// rsp は変化しない
void push_regi(Node *node)
{
    Node *n;
    int i = 0;

    // 解釈された引数を末尾から順にstack に積む
    for (n = node; n; n = n->next)
    {
        gen(n->lhs);
    }

    // スタックに値を積んだことにより順番が逆転し 先頭の引数から処理される
    for (n = node; n; n = n->next)
    {
        printf("    pop %s\n", regi64[i]);
        i++;
    }
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

    // 引数を解釈して　レジスタにセットする
    push_regi(node->lhs);

    printf("    mov rax, 0\n");
    printf("    call %s\n", node->func->name);
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

// 加減算を行う　一方がポインタの場合　もう一方をそのポインタが指しているサイズ分掛け算する　
// 左右辺のどちらもポインタならエラー
void gen_mul_ptr_size(Node *node)
{
    int cmp = cmp_node_size(node);

    if (cmp == -1)
    {
        if (node->rhs->type->kind == PTR)
        {
            // 右辺がポインタなので　左辺を右辺が参照しているサイズ分掛け算する
            printf("    imul rax, %d\n", size_of(node->rhs->type->ptr_to));
        }
    }
    else if (cmp == 0)
    {
        // 両方同じサイズの型なのでポインタ同士でなければ問題ない
        if (node->lhs->type->kind == PTR)
        {
            error_at(tokens[token_pos]->str, "ポインタ同士の演算はできません");
        }
    }
    else
    {
        if (node->lhs->type->kind == PTR)
        {
            // 左辺がポインタなので　右辺を左辺が参照しているサイズ分掛け算する
            printf("    imul rdi, %d\n", size_of(node->lhs->type->ptr_to));
        }
    }
}

void gen_add(Node *node)
{
    gen_mul_ptr_size(node);
    printf("    add rax, rdi\n");
}

void gen_sub(Node *node)
{
    gen_mul_ptr_size(node);
    printf("    sub rax, rdi\n");
}

void gen_mul()
{
    printf("    imul rax, rdi\n");
}

// rax = rax / rdi;
// rdx = rax % rdi;
void gen_div()
{
    printf("    cqo\n");
    printf("    idiv rdi\n");
}
