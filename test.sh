#!/bin/bash
# test/ に含まれるtest file を実行する　返り値が０であることを確認して終了する

dir_name="test/"
flag=0

assert()
{
    file_name="$1"

    ./bin/10cc "$file_name"
    gcc -o tmp/tmp tmp/tmp.s
    ./tmp/tmp

    actual="$?"

    if [ "$actual" = 0 ]; then
        echo " OK"
    else
        echo -e "\n  10cc returned $actual...\n"
        flag=1
    fi

    gcc -o a "$file_name" 
    ./a
    
    actual="$?"

    if [ "$actual" -ne 0 ]; then
        echo -e "  $file_name returned $actual...,\n  something is wrong in test case\n"
        flag=1
    fi
}


for file_name in `\find $dir_name -maxdepth 1 -type f`; 
    do
        assert $file_name
    done

if [ $flag = 0 ]; then
    echo -e "\n         You are a god-dammit genius !!\n"
else 
    echo -e "\nsomething was wrong..."
fi

rm -f a