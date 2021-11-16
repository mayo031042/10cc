int ptr1()
{
    int x;
    x = 42;
    int *y;
    y = &x;
    *y = 1;
    return x;
}

int ptr2()
{
    int x;
    x = 1;
    {
        {
            int *y;
            y = &x;
            *y = 3;
        }
        return x;
    }
}

int ptr3()
{
    int x;
    x = 1;
    int *p;
    p = &x;
    int **pp;
    pp = &p;
    int ***ppp;
    ppp = &pp;

    ***ppp = 0;
    return x;
}

// ポインタの加減算
int ptr4()
{
    int x;
    x = 1;
    int y;
    y = 2;
    int z;
    z = 3;
    int *p;
    p = &y;

    *(p + 1) = 10;

    if (x != 10)
        return 1;

    *(p - 1) = 11;
    if (z != 11)
        return 2;

    // 先に登録された変数からrbp に近い位置に登録されていく
    // 今回ではｘがrbp に最も近いのでp-=2、つまりオフセットを16下げることで
    // rbp から最も遠いz にオフセットが一致することになる
    // os(x)==rbp-8, os(z)==rbp-24

    p = &x;
    p -= 1;
    p -= 1;

    if (p == &z)
        return 0;

    return 1;
}

int size_of()
{
    if (sizeof(1) != 4)
        return 1;
    if (sizeof(sizeof(1)) != 4)
        return 2;

    int x;
    x = 1;
    if (sizeof(x) != 4)
        return 3;
    if (sizeof(&x) != 8)
        return 4;

    int *p;
    p = &x;
    if (sizeof(p) != 8)
        return 5;
    if (sizeof(*p) != 4)
        return 6;

    if (sizeof(1 + 2) != 4)
        return 7;
    if (sizeof(*p = 0) != 4)
        return 8;

    // sizeof の中身は実行されないので　ｘの値は変更されない
    if (x != 1)
        return 9;

    return 0;
}

int main()
{
    if (ptr1() != 1)
        return 1;
    if (ptr2() != 3)
        return 2;
    if (ptr3() != 0)
        return 3;
    if (ptr4())
        return 4;

    if (size_of() != 0)
        return 5;

    return 0;
}
