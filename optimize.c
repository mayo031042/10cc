#include <stdio.h>

int main()
{
    const char *file_name = "tmp.s";
    
    // sdtio内の構造体FILEを用いてファイルとストリームをつなぐ
    FILE *fp = fopen(file_name, "r+");

    if (fp == NULL)
    {
        fprintf(stderr, "fileがありません\n");
    }
    else
    {
        fprintf(stderr, "file was opened!\n");
        fclose(fp);
    }
}