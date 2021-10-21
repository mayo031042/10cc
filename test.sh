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

# << COMMENTOUT
# 数値
assert 0 "0;"
assert 42 "42;"
# 加減算
assert 10 "4+6;"
assert 3 "5-2;"
assert 10 "14+6-10;"
assert 9 "1+2-3+4+5;"
# 乗除算　優先処理　空白スキップ
assert 12 "3* 4;"
assert 4 "24/6;"
assert 1 "5 /3;"
assert 16 "4 *5/2 +2*3;"
assert 16 "2*(3+5);"
assert 7 "10-(3*3-2)+16/4;"
# 単項演算子
assert 12 "+12;"
assert 6 "12 + 2*(-3);"
assert 8 "-5+ +13;"
# 不等号　全組み合わせ 4(<,>,<=,>=)*3(左:右 -> ==,<,>) =12通り
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
# 等号
assert 1 "1==1;"
assert 0 "1==0;"
assert 0 "1!=1;"
assert 1 "1!=0;"
assert 1 "11==5+6;"
# 単長の変数
assert 1 "a=1;" # 代入
assert 2 "a=b=c=d=1; a=b+1;" # 連続代入
assert 6 "a= 2 *(5-2); a;"
assert 7 "a=3; a+4;"
assert 2 "a=-2; -a;"
assert 5 "b=20; b=b/4;"
assert 7 "x=3; y=4; z=x+y;"
assert 4 "a=3; b=a==3; a=b+a;"
assert 25 "a=b=5; a=b*b;"
# 複長の変数
assert 42 "nagainamae = 42;"
assert 100 "suuji_100_hairu = 100;"
assert 5 "num=10; x=12; num=3+x-num;"
assert 1 "num=10; x=3*3; (num-1)==x;"
# return 文
assert 1 "return 1;"
assert 2 "return 2; return 1;" # 複数のreturn 
assert 42 "returnx=42; return returnx;" # return に類似した変数
assert 4 "x=4; return x;" # 変数をreturn 
assert 2 "return x= -20 /(-3* 3 - 1); return 100;" # 代入と数式のreturn
# COMMENTOUT
# 代入演算子
assert 2 "a = 1; a += 1;" # 代入演算
assert 4 "a=1; b=2; a+=b=3;" # 代入演算式の右辺で　代入
assert 5 "a=b=1; a+=b+=3;" # 代入演算式の右辺で　代入演算
assert 10 "a=xyz=2; a+=3+(xyz=5);" # 代入演算の右辺で優先処理
assert 3 "a=b=2; a+=b==1+1;" # 代入式の右辺で　ブーリアン

echo OK