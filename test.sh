#!/bin/bash
# 制御構文まで
count=0

test(){
    ./bin/10cc "$count"
    gcc -o tmp/tmp tmp/tmp.s
    ./tmp/tmp
    actual="$?"
    count=$((count += 1))

    if [ "$actual" = 0 ]; then
        echo " OK"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}   

test
test
