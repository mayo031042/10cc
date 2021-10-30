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
assert 0 "
foo()
{
    return 42;
}

main()
{
    x=foo();
    return x;
}
"

echo -e "\n         You are a god-dammit genius !!\n"