#!/bin/bash
# test/ に含まれるtest file を実行する 返り値が０であることを確認して終了する

sort_file="test/sort/sort.c"
tmp_file="tmp/tmp.s"

create_file()
{
    make srt
    ./bin/10cc "$sort_file"
}

assert()
{
    # 10cc のテスト
    gcc -o tmp/tmp "$tmp_file"
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
time assert

make opt
tmp_file="tmp/tmp_opt.s"
time assert

rm -f a