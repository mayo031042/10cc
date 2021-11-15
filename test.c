#include <stdio.h>

int cmpl()
{
    return 1;
}

FILE *fp;

int main()
{
    // sdtio内の構造体FILEを用いてファイルとストリームをつなぐ
    fp = fopen("tmp.s", "r");

    if (fp == NULL)
    {
        fprintf(stderr, "fileがありません\n");
        return 0;
    }

    
}