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

assert 0 0
assert 42 42
assert 10 "4+6"
assert 3 "5-2"
assert 10 "14+6-10"
assert 9 "1+2-3+4+5"

assert 12 "3* 4"
assert 4 "24/6"
assert 1 "5 /3"
assert 16 "4 *5/2 +2*3"
assert 16 "2*(3+5)"
assert 7 "10-(3*3-2)+16/4"

echo OK