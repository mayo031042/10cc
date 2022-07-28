#!/bin/bash
# test/ に含まれるtest file を実行する 返り値が０であることを確認して終了する

file_name="test/sort/sort.c"

create_file()
{
    make srt
    ./bin/10cc "$file_name"
}

assert()
{
    # 10cc のテスト
    gcc -o tmp/tmp tmp/tmp.s
    ./tmp/tmp

    actual="$?"

    if [ "$actual" = 0 ]; then
        echo -e "\n         You are a god-dammit genius !!\n"
    else
        echo -e "\n  10cc returned $actual...\n"
        flag_10cc=0
    fi
}


create_file
time assert $file_name

make opt

time assert $file_name

rm -f a