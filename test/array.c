int arr1()
{
    int a[10];
    if (sizeof(a) != 40)
        return 1;
    if (sizeof(a[0]) != 4)
        return 2;

    return 0;
}

int arr2()
{
    int a[10];
    int i;
    i = 3;

    a[3] = 4;
    a[i] = 10;

    if (a[3] != 10)
        return 1;

    for (i = 0; i < 10; i += 1)
    {
        a[i] = i;
    }

    if (a[0] != 0)
        return 2;
    if (a[4] != 4)
        return 3;
    if (a[9] != 9)
        return 4;

    if (a[2] - a[0] != 2)
        return 5;

    if (a != &a[0])
        return 6;

    // if (a[1] != 1 [a])
    //     return 7;

    return 0;
}

int main()
{
    if (arr1() != 0)
        return 1;
    if (arr2() != 0)
        return 2;

    return 0;
}