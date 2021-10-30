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
    function();

    printf(".intel_syntax noprefix\n");

    for (int i = 0; funcs[i]; i++)
    {
        printf("    .globl %s\n", funcs[i]->name);
        printf("%s:\n", funcs[i]->name);

        gen_prologue(208);
        for (Node *n = funcs[i]->def; n; n = n->next)
        {
            gen(n);
            printf("    pop rax\n");
        }
        // printf("    mov rax, 0\n");
        gen_epilogue();
    }

    return 0;
}