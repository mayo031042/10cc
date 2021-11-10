#!/bin/bash
# 関数対応開始
assert() {
    expected="$1"
    input="$2"

    ./bin/10cc "$input" > tmp/tmp.s
    gcc -o tmp/tmp tmp/tmp.s
    ./tmp/tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        # echo "   $input => $actual"
        echo "  OK  $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assemble(){
    expected="$1"
    input="$2"

    ./bin/10cc "$input" > tmp/tmp.s
    echo "compiled"

    # ./optimize
    exit 0
}

echo "TEST func"

assert 1 " int main(){ return 1;}"
assert 1 " int main(){ int x; return 1;}"
assert 1 " int main(){ int x; x=1; return x;}"

assert 42 "
int foo()
{
    return 42;
}
int main()
{
    int x;
    x=foo();
    return x;
}"

assert 5 "
int foo11()
{
    return 3;
}
int main()
{
    int x;
    x=1+foo11()+3;
    return x-2;
}"

assert 11 "
int plus()
{
    return 3+4;
}
int minus()
{
    return 5-2;
}
int main()
{
    int x;
    x=plus()-minus();
    return x+plus();
}"

assert 8 "
int fuga();
int hoge()
{
    return fuga()+3;
}
int fuga()
{
    return 5;
}
int main()
{
    return hoge();
}"

# ネスト位置によって参照すべき変数が違う -> スコープ
assert 2 "
int main()
{
    int ret; ret=0;
    int x; x=1;
    {
        {
            int x; x=2;
            {
                int x; x=3;    
            }
            ret=x;
        }
    }
    return ret;
}"

assert 1 "
int main()
{
    int i;
    for(i=0;i<1000;i+=1)
    {
        int x;
    }
    return 1;
}
"

assert 1 "
int foo(int x)
{
    return x;
}
int main()
{
    return foo(1);
}
"

assert 6 "
int plus2(int x)
{
    return x+2;
}
int main()
{
    int x;
    x=plus2(4);
    return x;
}"

assert 34 "
int fib(int x,int y)
{
    if(30 < x+y)return x+y;
    return fib(y,x+y);
}
int main()
{
    return fib(1,1);
}"

assert 5 "
int foo(int x);
int main()
{
    return foo(2)+3;
}
int foo(int x)
{
    return x;
}"

assert 19 "
int hoge(int x);
int fuga(int x)
{
    if(10<x)return x;
    return hoge(x+5);
}
int hoge(int x)
{
    return fuga(x)+1;
}
int main()
{
    return hoge(0);
}
"

assert 40 "
int main()
{ 
// return 42;
/*
return 41;
*/
return 40;
}"

assert 3 "
int main()
{
    int x;  x=1;
    int *y; y=&x;
    *y=3;
    return x;
}"

assert 3 "
int main()
{
    int x; x=1;
    {
        int *y; y=&x;
        {
            *y=3;   
        }
        return x;   
    }
}"

assert 2 "
int main()
{
    int x;      x=1;
    int *p;     p=&x;
    int **pp;   pp=&p;
    int ***ppp; ppp=&pp;

    ***ppp=2;
    return x;
}"

# C の仕様に反するが以下は正常に動作する
# assert 0 "
# int main()
# {
#     int x; int y;
#     x=1; y=2;
#     int *p; p=&x;
#     *(p-8)=0;
#     return y;
# }"

echo -e "\n         You are a god-dammit genius !!\n"