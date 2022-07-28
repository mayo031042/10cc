#!/bin/bash
# test/ に含まれるtest file を実行する 返り値が０であることを確認して終了する

dir_name="test/"
flag_10cc=1
flag_gcc=1

assert()
{
    file_name="$1"

    # 10cc のテスト
    ./bin/10cc "$file_name"
    gcc -o tmp/tmp tmp/tmp.s
    ./tmp/tmp

    actual="$?"

    if [ "$actual" = 0 ]; then
        echo " OK"
    else
        echo -e "\n  10cc returned $actual...\n"
        flag_10cc=0
    fi

    # テストケースのテスト
    gcc -o a "$file_name" 
    ./a
    
    actual="$?"

    if [ "$actual" -ne 0 ]; then
        echo -e "  $file_name returned $actual...,\n  something is wrong in test case\n"
        flag_gcc=0
    fi

    if [ $flag_10cc = 0 -o $flag_gcc = 0 ]; then
        rm -f a
        exit 1
    fi
}


for file_name in `\find $dir_name -maxdepth 1 -type f`; 
    do
        assert $file_name
    done

echo 
if [ $flag_10cc = 1 -a $flag_gcc = 1 ]; then
    echo -e "         You are a god-dammit genius !!\n"
else 

    if [ $flag_10cc = 0 ]; then
        echo "10cc was wrong..."
    fi
    if [ $flag_gcc = 0 ]; then
        echo "test case was wrong..."
    fi
fi

rm -f a