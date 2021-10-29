#include <stdio.h>

#define str_size 4
#define str_len 20

FILE *rp, *wp;
int now_str = 0;
char str[str_size][str_len];
int getf;

void str_reset()
{
    for (int i = 0; i < str_size; i++)
        for (int j = 0; j < str_len; j++)
            str[i][j] = '\0';
}

int pre_str(int val)
{
    return (now_str - val + (str_size)) % (str_size);
}

int next_str()
{
    now_str++;
    if (now_str == (str_size))
        now_str = 0;

    str_reset();
    return now_str;
}

int str_chk(char *p1, char *p2)
{
    for (int i = 0; i < str_len; i++)
    {
        if (p1[i] != p2[i])
            return 0;

        if (p1[i] == '\0' || p2[i] == '\0')
            return 1;
    }
}

int push_pop_can_delete()
{
    if (!str_chk(str[0], "push"))
        return 0;
    if (!str_chk(str[1], "rax"))
        return 0;
    if (!str_chk(str[2], "pop"))
        return 0;
    if (!str_chk(str[3], "rax"))
        return 0;

    return 1;
}

int str_read(int str_pos)
{
    int char_pos = 0;
    while (1)
    {
        getf = fgetc(rp);
        if (feof(rp))
            return 1;

        str[str_pos][char_pos++] = getf;

        if (getf == '\0' || getf == '\n' || getf == ' ')
            return 0;
    }
}

int main()
{
    // sdtio内の構造体FILEを用いてファイルとストリームをつなぐ
    rp = fopen("tmp.s", "r");
    wp = fopen("tmp2.s", "w");

    if (rp == NULL)
    {
        fprintf(stderr, "fileがありません\n");
        return 0;
    }

    while (1)
    {
        getf = fgetc(rp);
        if (feof(rp))
            break;

        str_read(0);

        if (!str_chk(str[0], "push"))
        {
            // pushではなかったので普通に出力
            fprintf(wp, "%s", str[0]);
            str_reset();
            continue;
        }

        int eof = 0;

        for (int i = 1; i < 4; i++)
        {
            if (str_read(i))
            {
                eof = 1;
                break;
            }
        }

        if (!eof)
        {
            if (push_pop_can_delete())
                continue;
            else
            {
                for (int i = 0; i < 4; i++)
                    fprintf(wp, "%s", str[i]);
            }
        }

        else
        {
            for (int i = 0; i < 4; i++)
                fprintf(wp, "%s", str[i]);
            break;
        }
    }
    fclose(rp);
    fclose(wp);
}