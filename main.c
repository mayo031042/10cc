#include "10cc.h"

char *user_input;
FILE *test_file_ptr;
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

// 指定されたファイルの内容をuser_input に渡す
void read_file(char *path)
{
    fprintf(stderr, "%s: ", path);

    test_file_ptr = fopen(path, "r");

    // ファイルの長さを調べる
    if (fseek(test_file_ptr, 0, SEEK_END) == -1)
        error("%s: fseek ", path);

    size_t size = ftell(test_file_ptr);
    if (fseek(test_file_ptr, 0, SEEK_SET) == -1)
        error("%s: fseek ", path);

    // ファイル内容を読み込む
    user_input = calloc(1, size + 2);
    fread(user_input, size, 1, test_file_ptr);

    // ファイルが必ず"\n\0"で終わっているようにする
    if (size == 0 || user_input[size - 1] != '\n')
    {
        user_input[size++] = '\n';
    }

    user_input[size] = '\0';
    fclose(test_file_ptr);
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

    return 0;
}