// 関数

int foo()
{
    return 2;
}

// 関数から別の関数呼び出し
int bar1()
{
    return 1;
}
int bar2()
{
    return bar1() + 2;
}

// 引数あり
int mul2(int x)
{
    return x * 2;
}

// 相互参照
int fuga1();
int fuga2()
{
    return fuga1() + 3;
}
int fuga1()
{
    return 5;
}

// main の後方に定義を配置
int hoge(int x, int y, int z);

int main()
{
    if (-1 + foo() + 3 != 4)
        return 1;
    if (-bar2() != -3)
        return 2;
    if (mul2(3) != 6)
        return 2;
    if (fuga2() != 8)
        return 3;
    if (hoge(1, 1, 30) != 34)
        return 4;

    return 0;
}

int hoge(int x, int y, int z)
{
    x += y;
    if (z < x)
        return x;
    return hoge(y, x, z);
}