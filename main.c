#include "10cc.h"

// グローバル変数の宣言
char *user_input;
int pos = 0;
LVar *locals = NULL;

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

    gen_prologue(208);

    for (int i = 0; codes[i]; i++)
    {
        gen(codes[i]);
        printf("    pop rax\n");
    }

    // printf("    mov rax, 0\n");
    gen_epilogue();
    
    return 0;
}