#include "10cc.h"

#define MAX_TOKEN_SIZE 200
#define MAX_CODE_SIZE 200

// グローバル変数の宣言
char *user_input;
int pos = 0;
Token *tokens[MAX_TOKEN_SIZE];
Node *codes[MAX_CODE_SIZE];
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
    printf("    sub rsp, 208\n");

    for (int i = 0; codes[i]; i++)
    {
        gen(codes[i]);
        printf("    pop rax\n");
    }

    // エピローグ
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}