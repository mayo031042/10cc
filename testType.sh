#!/bin/bash
# 関数対応開始
assert() {
    expected="$1"
    input="$2"

    ./bin/10cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "   $input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assemble(){
    expected="$1"
    input="$2"

    ./bin/10cc "$input" > tmp.s
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
        int x; x=2;
        {
            int x; x=3;    
        }
        ret=x;
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
echo -e "\n         You are a god-dammit genius !!\n"