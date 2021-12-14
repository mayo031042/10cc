#include "codegen.h"

char *regi64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *regi32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

// my printf
void pf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
}

// default 条件を追加　配列などは1,2,4,8 以外のサイズを取りうるがPTR と同等に扱うので８として分類する
char *byte_size_string(int size, char *p)
{
    if (p == NULL)
    {
        switch (size)
        {
        case 1:
            return "BYTE PTR\0";
        case 2:
            return "WORD PTR\0";
        case 4:
            return "DWORD PTR\0";
        default:
            return "QWORD PTR\0";
        }
    }
    else if (p == "rax")
    {
        switch (size)
        {
        case 1:
            return "al\0";
        case 2:
            return "ax\0";
        case 4:
            return "eax\0";
        default:
            return "rax\0";
        }
    }
    else if (p == "rdi")
    {
        switch (size)
        {
        case 1:
            return "dil\0";
        case 2:
            return "di\0";
        case 4:
            return "edi\0";
        default:
            return "rdi\0";
        }
    }
    // rax に8バイト未満のものを移動させる際のゼロクリアに対応する
    else if (p == "mov")
    {
        switch (size)
        {
        case 1:
        case 2:
        case 4:
            return "movsx\0";
        default:
            return "mov\0";
        }
    }

    else
    {
        error("引数の文字列が何れのパターンにもマッチしません byte_size_string()");
    }

    printf("%d\n\n", size);
    error("引数のサイズが不正な値を取っています byte_size_string()");
}

// スタックトップと引数をcmp する 条件分岐jmp 命令の直前に使用
void cmp_rax(int val)
{
    pf("    pop rax\n");
    pf("    cmp rax, %d\n", val);
}

// スタックから２つ取り出し　変数に対応した適切なサイズ分　メモリに移動する
void gen_mov_imm_to_addr(Node *node)
{
    node = node->lhs;
    pf("    pop rax\n");
    pf("    pop rdi\n");

    int size = size_of_node(node);
    pf("    mov %s [rax], %s\n", byte_size_string(size, NULL), byte_size_string(size, "rdi"));

    pf("    push rdi\n");
}

// 適切にレジスタへ解釈された引数が渡されている前提のうえで　レジスタからメモリに値を移す
void pop_regi()
{
    int i = 0;
    int total_offset = 0;

    for (LVar *lvar = func_pos_ptr->locals[0]; lvar; lvar = lvar->next)
    {
        if (6 <= i)
        {
            error("６つ以上の引数には未対応です,,,!!");
        }

        total_offset += size_of(lvar->type);

        pf("    mov %s -%d[rbp], %s\n", byte_size_string(size_of(lvar->type), NULL), total_offset, regi32[i]);
        i++;
    }
}

// rbp, rsp を準備する
// これが実行されるときは必ず関数の先頭にあるので　func_pos を参照して良い
void gen_prologue()
{
    pf("    .globl %s\n", func_pos_ptr->label);
    pf("%s:\n", func_pos_ptr->label);

    pf("    push rbp\n");
    pf("    mov rbp, rsp\n");
    pf("    sub rsp, %d\n", func_pos_ptr->max_offset);

    pop_regi();
}

// rbp, rsp をもとに関数呼び出し前に戻しreturn する
void gen_epilogue()
{
    pf("    mov rsp, rbp\n");
    pf("    pop rbp\n");
    pf("    ret\n");
}

// 変数宣言に対応　配列の場合はサイズを計算する
// 基本的にはpush 0 のみ行う
void gen_declare(Node *node)
{
    // 配列の場合　サイズを計算したい
    if (node->lhs)
    {
        gen(node->lhs);
        pf("    pop rax\n");
        // rax に配列のサイズが入っているはずであるが、、、
    }

    pf("    push 0\n");
}

// nodeを左辺値とみなせた時　rbp-offset をスタックに積む
// addr は変数のみをオペランドとして持つ
void gen_addr(Node *node)
{
    node = node->lhs;

    if (node)
    {
        // &* は打ち消す
        if (ND_DEREF == node->kind)
        {
            gen(node->lhs);
            return;
        }
    }

    if (node->kind != ND_LVAR)
    {
        error_at(tokens[token_pos]->str, "左辺値ではありません gen_addr()\n");
    }

    // rbp とoffsset からアドレスを計算し積む
    pf("    mov rax, rbp\n");
    pf("    sub rax, %d\n", node->lvar->offset);
    pf("    push rax\n");
}

// スタックトップを参照する　積まれた数値をアドレスとして解釈し それに入っている値に交換する
void gen_deref(Node *node)
{
    gen(node->lhs);

    if (type_of_node(node)->kind == ARRAY)
    {
        return;
    }

    int size = size_of_node(node);

    pf("    pop rax\n");
    pf("    %s rax, %s [rax]\n", byte_size_string(size, "mov"), byte_size_string(size, NULL));
    pf("    push rax\n");
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
        pf("    pop %s\n", regi64[i]);
        i++;
    }
}

// call 命令前にrsp の値をrsp が16の倍数になるように１回または２回push する
// return されてからrax を変更する前にpop rsp することでpush 回数に関わらずrsp の復元ができる
void gen_func_call(Node *node)
{

    pf("    mov rax, rsp\n");
    pf("    mov rdi, 16\n");
    gen_div();
    // rdx = rax % rdi(16);

    pf("    mov rax, rsp\n");

    int cnt = count();
    pf("    cmp rdx, 0\n");
    pf("    jne .Lcall%d\n", cnt);
    pf("    push rax\n");

    pf(".Lcall%d:\n", cnt);
    pf("    push rax\n");

    // 引数を解釈して　レジスタにセットする
    push_regi(node->lhs);

    pf("    mov rax, 0\n");
    pf("    call %s\n", node->func->label);
    pf("    pop rsp\n");
    pf("    push rax\n");
}

// 条件式が偽のときのみ次の処理パートへjmp する
// 同一else 群の中で１つでもif 実行文が実行されたときは　else 群の末尾の次にjmp する
// １つのif 内で1push 保証される
void gen_if(Node *node, int end_label)
{
    int next_label = count();

    gen(node->lhs); // B
    cmp_rax(0);
    pf("    je .Lifnext%d\n", next_label);

    gen(node->rhs); // X
    pf("    jmp .Lifend%d\n", end_label);

    pf(".Lifnext%d:\n", next_label);
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
    pf("    pop rax\n");
    pf("    jmp .Lreq%d\n", stack_front());

    pf(".Lexe%d:\n", stack_front());

    // 実行文　ここでも必ず1push を保証する
    gen(node->rhs->lhs); // X
    pf("    pop rax\n");

    pf(".Lcont%d:\n", stack_front()); // continue先 → 変化式の前

    gen(node->lhs->rhs); // C
    pf("    pop rax\n");

    pf(".Lreq%d:\n", stack_front());

    // 条件式を実行、評価して　実行文に移動するかを決める
    gen(node->rhs->rhs); // B
    cmp_rax(0);
    pf("    jne .Lexe%d\n", stack_front());

    pf(".Lbrk%d:\n", stack_front());

    // for, while 全体でも1push を保証
    pf("    push rax\n");

    stack_pop();
}

void gen_do_while(Node *node)
{
    stack_push(count());

    pf(".Lexe%d:\n", stack_front());
    gen(node->lhs);      // X
    pf("    pop rax\n"); // x の揃えポップ　→　continue ではすでに揃っているのでスキップ

    pf(".Lcont%d:\n", stack_front()); // continue 先 contiの有無に関わらずスタックトップは揃っている

    gen(node->rhs); // B
    cmp_rax(0);
    pf("    jne .Lexe%d\n", stack_front());

    pf(".Lbrk%d:\n", stack_front()); // break先　有無に関わらずスタックトップは揃っている

    pf("    push rax\n");

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
        pf("    pop rax\n");
    }

    pf("    push rax\n");

    sub_block_nest();
}

// 加減算を行う　一方がポインタの場合　もう一方をそのポインタが指しているサイズ分掛け算する　
// 左右辺のどちらもポインタならエラー
int gen_mul_ptr_size(Node *node)
{
    int cmp = cmp_node_size(node);

    if (cmp == -1)
    {
        if (has_ptr_to(node->rhs->type))
        {
            // 右辺がポインタなので　左辺を右辺が参照しているサイズ分掛け算する
            pf("    imul rax, %d\n", size_of(node->rhs->lhs->type->ptr_to));
        }
    }
    // else if (cmp == 0)
    // {
    //     // 両方同じサイズの型なのでポインタ同士でなければ問題ない
    //     if (node->lhs->type->kind == PTR)
    //     {
    //         error_at(tokens[token_pos]->str, "ポインタ同士の演算はできません");
    //     }
    // }
    else if (cmp == 1)
    {
        if (has_ptr_to(node->lhs->type))
        {
            // 左辺がポインタなので　右辺を左辺が参照しているサイズ分掛け算する
            pf("    imul rdi, %d\n", size_of(node->lhs->lhs->type->ptr_to));
        }
    }

    return cmp;
}

void gen_add(Node *node)
{
    int cmp = gen_mul_ptr_size(node);
    if (cmp == 0)
    {
        if (has_ptr_to(node->rhs->type))
        {
            error_at(tokens[token_pos]->str, "ポインタ同士の演算はできません");
        }
    }

    pf("    add rax, rdi\n");
}

void gen_sub(Node *node)
{
    int cmp = gen_mul_ptr_size(node);
    pf("    sub rax, rdi\n");

    if (cmp == 0)
    {
        if (node->rhs->type == node->lhs->type)
        {
            if (has_ptr_to(node->rhs->type))
            {
                pf("    mov rdi, %d\n", size_of_node(node));
                gen_div();
            }
        }
    }
}

void gen_mul()
{
    pf("    imul rax, rdi\n");
}

// rax = rax / rdi;
// rdx = rax % rdi;
void gen_div()
{
    pf("    cqo\n");
    pf("    idiv rdi\n");
}
