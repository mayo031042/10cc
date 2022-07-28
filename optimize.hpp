#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *user_input;

// token
typedef enum
{
    TK_IDENT, // 変数
    TK_NUM,   // 数値
    TK_SUB,
    TK_ADD,
    TK_RAX,
    TK_RDI,
    TK_RSP,
    TK_RBP,

    TK_JMP,

    TK_RETURN,
    TK_PUSH,
    TK_POP,
    TK_MOV,

    TK_ELSE, // その他のラベル
    TK_EOF,  // 入力の最後
} TokenKind;

typedef struct Token Token;
struct Token
{
    TokenKind kind;
    // Token *next;
    int val;
    int len;
    char *str;
};

// 指定されたファイルの内容をuser_input に渡す
void read_file(char *path)
{
    fprintf(stderr, "%s: ", path);

    FILE *test_file_ptr = fopen(path, "r");

    size_t size = ftell(test_file_ptr);

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
