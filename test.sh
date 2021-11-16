#!/bin/bash
# test/ に含まれるtest file を実行する　返り値が０であることを確認して終了する

assert()
{
    ./bin/10cc "$1"
    gcc -o tmp/tmp tmp/tmp.s
    ./tmp/tmp

    actual="$?"

    if [ "$actual" = 0 ]; then
        echo " OK"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

dir_name="test/"

for file_name in `\find $dir_name -maxdepth 1 -type f`; 
    do
        assert $file_name
    done

echo -e "\n         You are a god-dammit genius !!\n"