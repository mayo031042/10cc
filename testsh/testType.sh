#!/bin/bash
# 関数対応開始
count=0

assert() {
    expected="$1"
    input="$2"

    ./bin/10cc "$input" > tmp/tmp.s
    gcc -o tmp/tmp tmp/tmp.s
    ./tmp/tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "  OK  $count"
        count=$((count += 1))
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

# 0
assert 1 " int main(){ return 1;}"
assert 1 " int main(){ int x; return 1;}"
assert 1 " int main(){ int x; x=1; return x;}"
assert 1 " int main(){ int i; for(i=0;i<10000;i+=1){ i; } return 1;}"

assert 42 "
int foo()
{
    return 42;
}
int main()
{   
    return foo();
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
}"

# 10
assert 1 "
int foo(int x)
{
    return x;
}
int main()
{
    return foo(1);
}"

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

assert 0 "
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
    return hoge(0) != 19;
}"

assert 0 "
int main()
{ 
// return 2;
/*
/*
return 1;
*/
return 0;
}"

assert 0 "
int main()
{
    int x;  x=1;
    int *y; y=&x;
    *y=0;
    return x;
}"

assert 0 "
int main()
{
    int x; x=1;
    {
        {
            int *y; y=&x;
            *y=3;   
        }
        return x != 3;   
    }
}"

assert 0 "
int main()
{
    int x;      x=1;
    int *p;     p=&x;
    int **pp;   pp=&p;
    int ***ppp; ppp=&pp;

    ***ppp=0;
    return x;
}"

assert 0 "
int foo(int *p)
{
    int x;    x=2;  p=&x;
    int **pp; pp=&p;
    **pp=0;
    return x;
}
int main()
{
    int *p;
    return foo(p);
}"

# 20
assert 0 "
int main()
{
    int x;  x=1; 
    int y;  y=2;
    int z;  z=3;
    int *p; p=&y;

    *(p+1)=10;

    if(x!=10) return 1;

    *(p-1)=11;
    if(z!=11) return 2;

    p=&z;
    p+=1; p+=1;

    return *p!=x;
}"

assert 0 "
int main()
{
    if(sizeof(1)!=4) return 1;
    if(sizeof(sizeof(1)) != 4)  return 2;
    
    int x; x=1;
    if(sizeof(x) != 4) return 3;
    if(sizeof(&x) != 8) return 4;
    
    int *p; p=&x;
    if(sizeof(p) != 8) return 5;
    if(sizeof(*p) != 4) return 6;
    
    if(sizeof(1+2) != 4) return 7;
    if(sizeof(*p=0) != 4) return 8;
    if(x!=1) return 9;

    return 0;
}"

echo -e "\n         You are a god-dammit genius !!\n"