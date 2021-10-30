#!/bin/bash
# 関数対応開始
assert() {
    expected="$1"
    input="$2"

    ./10cc "$input" > tmp.s
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

    ./10cc "$input" > tmp.s
    echo "compiled"

    ./optimize
    exit 0
}

echo "TEST func"
assert 42 "
foo()
{
    return 42;
}
main()
{
    x=foo();
    return x;
}"

assert 32 "
foo1()
{
    30;
}
main()
{
    x=1+foo1()+3;
    return x-2;
}"

assert 11 "
plus()
{
    return 3+4;
}
minus()
{
    return 5-2;
}
main()
{
    x=plus()-minus();
    return x+plus();
}"

# assemble 8 "fuga();hoge(){    return fuga()+3;}fuga(){  return 5;}main(){return hoge();}"

echo -e "\n         You are a god-dammit genius !!\n"