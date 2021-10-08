#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の数が間違っています\n");
        return 0;
    }

    // 各区切りに対して
    char *p=argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // 今回は　正しい入力の場合空白文字がないものとして考え　以下コメントアウトは正しい入力を前提としている
    // strtolについて　第一引数のポインタのしめすものについて第三引数を基数としたものに変換する　
    // この場合数値変換できなかった最初のポインタを第二引数に渡す
    printf("    mov rax, %d\n", strtol(p,&p,10));

    while(*p){
        if(*p=='+'){
            // 数値と想定されるものを指すポインタに進む
            p++;
            printf("    add rax, %d\n", strtol(p,&p,10));
            continue;
        }
        else if(*p=='-'){
            p++;
            printf("    sub rax, %d\n", strtol(p,&p,10));
            continue;
        }
        else {    
            fprintf(stderr, "入力が間違っています\n");
            return 1;
        }
    }

    printf("    ret\n");
    return 0;
}