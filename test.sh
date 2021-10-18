#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./10cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"
assert 10 "4+6;"
assert 3 "5-2;"
assert 10 "14+6-10;"
assert 9 "1+2-3+4+5;"

assert 12 "3* 4;"
assert 4 "24/6;"
assert 1 "5 /3;"
assert 16 "4 *5/2 +2*3;"
assert 16 "2*(3+5);"
assert 7 "10-(3*3-2)+16/4;"

assert 12 "+12;"
assert 6 "12 + 2*(-3);"
assert 8 "-5+ +13;"

assert 0 "1<0;"
assert 0 "1<1;"
assert 1 "1<2;"
assert 1 "1>0;"
assert 0 "1>1;"
assert 0 "1>2;"

assert 0 "1<=0;"
assert 1 "1<=1;"
assert 1 "1<=2;"
assert 1 "1>=0;"
assert 1 "1>=1;"
assert 0 "1>=2;"

assert 1 "1==1;"
assert 0 "1==0;"
assert 0 "1!=1;"
assert 1 "1!=0;"

assert 1 "11==5+6;"

assert 1 "a=1;"
assert 6 "a=2*3; a;"
assert 7 "a=3; a+4;"
assert 2 "a=(3-1);"
assert 5 "b=20; b=b/4;"
assert 7 "x=3; y=4; z=x+y;"
assert 4 "a=3; b=a==3; a=b+a;"
assert 25 "a=b=5; a=b*b;"

assert 1 "nm=1;"
assert 42 "nagainamae = 42;"
assert 100 "suuji_100_hairu = 100;"
assert 5 "num=10; x=12; num=3+x-num;"
assert 1 "num=10; x=3*3; (num-1)==x;"

echo OK