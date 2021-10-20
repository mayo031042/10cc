#include "10cc.h"

// グローバル変数の宣言
char *user_input;
Token *token;
Node *code[100];
LVar *locals;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の数が間違っています\n");
        return 1;
    }

    user_input = argv[1];
    tokenize();
    program();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // プロローグ
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    // 変数が使用する分の　メモリを確保
    printf("    sub rsp, 208\n");

    for (int i = 0; code[i]; i++)
    {
        gen(code[i]);
        // 全ての部分木に対してstack への保存は１度だけと言える
        printf("    pop rax\n");
    }

    // エピローグ
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}