#include "10cc.h"

char *user_input;
FILE *fp;

void compile()
{
    tokenize();
    fprintf(stderr, "T");

    function();
    fprintf(stderr, "P");

    code_gen();
    fprintf(stderr, "C");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の数が間違っています");
    }

    fp = fopen("tmp/tmp.s", "w");

    read_file(argv[1]);

    compile();

    fclose(fp);

    return 0;
}