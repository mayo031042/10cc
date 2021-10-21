#include "10cc.h"

void error(char *fmt, ...)
{
    // 可変長引数をそのまま受け取れる型va_list の変数ap を宣言
    va_list ap;
    // ap に対してfmt の次の引数以降を格納する (初期化(代入？))
    va_start(ap, fmt);
    // 第一引数に出力したいストリーム　第２にフォーマット　第３に２に対しての置換文字列を指定
    // 第３引数は配列になっている必要がある　多分vector の意味
    vfprintf(stderr, fmt, ap);
    // 上記と異なるのが第３引数である　可変長引数として解釈されるので　第２引数内の置換文字分指定すれば良い
    fprintf(stderr, "\n");
    // exit(1);
}

void err(char *fmt)
{
    fprintf(stderr, fmt);
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
        // 左辺をアドレスとみなして　そのアドレスへと右辺の値を　コピーする
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
    // return 文　これ以降のアセンブリは実行されない
    case ND_RETURN:
        gen(node->lhs);
        // この時　return したい値はstack のトップに存在しているはずである
        printf("    pop rax\n");
        //　最初のret 以外　実行されないコードも生成されてしまうが今回はそれを許容する　つまりここでエピローグを書いてよい
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
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
