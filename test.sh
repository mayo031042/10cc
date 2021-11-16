#!/bin/bash
# all_files に含まれる要素に一致するような名前のファイルをtest/ から探して実行する　返り値が０であることを書くにして終了する

all_files="culc vars func ptr_sizeof various"

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
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

for file_name in $all_files
    do
        assert $file_name
    done
