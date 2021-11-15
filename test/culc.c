int main()
{
    // 四則演算
    if (+10 - (3 * 3 - 2) + 16 / 4 != 7)
        return 1;
    if (-5 + 10 + (3 + 4) * 2 % 4 - 2 != 5)
        return 1;

    if ((1 < 0) != 0)
        return 2;
    if ((1 < 1) != 0)
        return 2;
    if ((1 < 2) != 1)
        return 2;

    if ((1 > 0) != 1)
        return 3;
    if ((1 > 1) != 0)
        return 3;
    if ((1 > 2) != 0)
        return 3;

    if ((1 <= 0) != 0)
        return 4;
    if ((1 <= 1) != 1)
        return 4;
    if ((1 <= 2) != 1)
        return 4;

    if ((1 >= 0) != 1)
        return 5;
    if ((1 >= 1) != 1)
        return 5;
    if ((1 >= 2) != 0)
        return 5;

    if ((1 == 1) != 1)
        return 6;
    if ((1 == 0) != 0)
        return 6;
    if ((1 != 1) != 0)
        return 6;
    if ((1 != 0) != 1)
        return 6;

    return 0;
}
