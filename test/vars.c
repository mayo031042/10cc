int main()
{
    // 変数　
    int num;
    int val;
    int x;

    num = 10;
    if (num != 10)
        return 11;
    num = val = x = 1;
    x = -num + val + 2;
    if (x != 2)
        return 11;

    x += 1 + (num += 2);
    if (x != 6)
        return 11;
    x %= 4;
    if (x != 2)
        return 11;

    // 制御構文
    for (num = 0;; num += 1)
    {
        {} {
            ;
            ;
            ;
        };
        ;
        ;
        {
        }
        if (0)
            ;
        if (0)
            ;
        if (1)
            ;
        if (num == 100)
        {
            // 100 回目のループで真になる
            x = 1;
            break;
        }
        else if (num == -1)
        {
            // 常に偽   x=2;
            break;
        }
        else
        {
            continue;
        }
    }
    if (x != 1)
        return 12;

    while (num -= 1)
    {
        if (num == 10)
        {
            break;
        }
    }
    if (num != 10)
        return 13;

    do
    {
        num += 10;
    } while (num == 0);
    if (num != 20)
        return 14;

    // ここまで問題なし
    return 0;
}