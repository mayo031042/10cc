#include "10cc.h"

char *user_input;

void compile(char *p)
{
    user_input = p;
    tokenize();
    fprintf(stderr, "t");

    function();
    fprintf(stderr, "f");

    code_gen();
    fprintf(stderr, "c");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の数が間違っています\n");
        return 1;
    }

    compile(argv[1]);

    return 0;
}