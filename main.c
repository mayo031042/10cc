#include "10cc.h"

// グローバル変数の宣言
char *user_input;
int token_pos = 0;

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
    code_gen();

    return 0;
}