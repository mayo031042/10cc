#include "codegen.h"

void gen(Node *node)
{
    switch (node->kind)
    {
    // NOP か数値を積む
    case ND_NOP:
        return;

    case ND_LVAR:
        error("ND_ADDR 以外からND_LVAR をgen しようとしています");
        return;

    case ND_NUM:
        pf("    push %d\n", node->val);
        return;

    case ND_DECLARE:
        gen_declare(node);
        return;

    // 左辺の計算結果をアドレスとして解釈し　そのアドレスに積まれている値に変更する
    case ND_DEREF:
        gen_deref_(node);
        return;

    // 左辺にND_LVAR を持ち　そこで登録されている変数のアドレスをスタックに積む
    // &* の列が出現した場合は読み飛ばし* の左辺をgen() する
    case ND_ADDR:
        gen_addr(node);
        return;

    // 代入 適切なアドレスに保持されている値を書き換え結果を積む
    case ND_ASSIGN:
        gen(node->lhs);
        gen_addr(node->rhs);
        // gen_deref(node->rhs);
        gen_mov_imm_to_addr(node->rhs);
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
        pf("    jmp .Lcont%d\n", stack_front());
        return;
    case ND_BREAK:
        pf("    jmp .Lbrk%d\n", stack_front());
        return;

    // その他
    case ND_RETURN:
        gen(node->lhs);
        pf("    pop rax\n");
        gen_epilogue();
        return;
    case ND_ELSE:
        int end_label = count();
        gen_else(node, end_label);
        pf(".Lifend%d:\n", end_label);
        return;
    case ND_BLOCK:
        gen_block(node->lhs);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    // raxが左辺　rdiが右辺
    pf("    pop rdi\n");
    pf("    pop rax\n");

    // 左右辺の展開をした後にstack に積まれた結果について関係を記述する
    switch (node->kind)
    {
    // 四則演算
    case ND_ADD:
        gen_add(node);
        break;
    case ND_SUB:
        gen_sub(node);
        break;
    case ND_MUL:
        gen_mul();
        break;
    case ND_DIV:
        gen_div();
        break;
    case ND_DIV_REM:
        gen_div();
        pf("    mov rax, rdx\n");
        break;

    // 不等号
    case ND_EQ:
        pf("    cmp rax, rdi\n");
        pf("    sete al\n");
        pf("    movzb rax, al\n");
        break;
    case ND_NE:
        pf("    cmp rax, rdi\n");
        pf("    setne al\n");
        pf("    movzb rax, al\n");
        break;
    case ND_LT:
        pf("    cmp rax, rdi\n");
        pf("    setl al\n");
        pf("    movzb rax, al\n");
        break;
    case ND_LE:
        pf("    cmp rax, rdi\n");
        pf("    setle al\n");
        pf("    movzb rax, al\n");
        break;
    }

    pf("    push rax\n");
}

// 関数の定義部分を出力 main も含まれる
void code_gen()
{
    pf(".intel_syntax noprefix\n");

    for (int i = 0; funcs[i]; i++)
    {
        func_pos_ptr = funcs[i];

        gen_prologue();

        gen(func_pos_ptr->definition);
        // gen(funcs[i]->definition);
        pf("    pop rax\n");

        gen_epilogue();
    }
}