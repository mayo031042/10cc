int main()
{
    int a[10];

    if (sizeof(a) != 40)
        return 1;
    if (sizeof(a[0]) != 4)
        return 2;

    int x;
    x = 3;

    a[3] = 4;
    a[x] = 10;

    if (a[3] != 10)
        return 3;
        
    // int i;
    // for (i = 0; i < 10; i += 1)
    // {
    //     a[i] = x;
    // }

    // if (a[1] != 3)
    //     return 4;

    return 0;
}