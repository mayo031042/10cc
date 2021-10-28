#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./10cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "   $input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assemble(){
    expected="$1"
    input="$2"

    ./10cc "$input" > tmp.s
    echo "compiled"

    ./optimize
    exit 0
}

# << COMMENTOUT
# 数値
assert 1 "1;"
assert 42 " 42  ; "
# 加減算
assert 10 "4+6;"
assert 3 "5-2;"
assert 7 "1+2-3+4+15-12;"
# 乗除算　優先処理　空白スキップ
assert 12 "3* 4;"
assert 4 "24/6;"
assert 1 "5 /3;"
assert 16 "4 *5/2 +2*3;"
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
# 複長の変数
assert 42 "nagainamae = 42;"
assert 100 "suuji_100_hairu = 100;"
assert 5 "num=10; x=12; num=3+x-num;"
# return 文
assert 1 "return 1;"
assert 42 "returnx=42; return returnx; return 5;" # return に類似した変数
assert 2 "return x= -20 /(-3* 3 - 1); return 100;" # 代入と数式のreturn
echo -e " return OK\n"

# 代入演算子
assert 2 "a = 1; a += 1;" # 代入演算
assert 4 "a=1; b=2; a+=b=3;" # 代入演算式の右辺で　代入
assert 5 "a=b=1; a+=b+=3;" # 代入演算式の右辺で　代入演算
assert 10 "a=xyz=2; a+=3+(xyz=5);" # 代入演算の右辺で優先処理
assert 3 "a=b=2; a+=b==1+1;" # 代入式の右辺で　ブーリアン
assert 1 "a=2; a-=1;" # 代入減算
assert 1 "a=1;b=3; b-=a=2;"
assert 3 "a=6;b=5; a-=b-=2;" # 連続代入減算
assert 1 "a=xyz=3; a-=6/((xyz-=2)+2);"
assert 9 "a=b=3; a*=b;" # 代入乗算
assert 18 "a=3; xyz=4; a*=3*(xyz-=2);"
assert 3 "a=b=4; a+=8; a/=b;" # 代入除算
assert 3 "a=15; xyz=23; a/=(xyz-=8)/3;"
# 制御構文
assert 2 "if(1)return 2; 0;" # 数値条件　return 
assert 2 "ret=1; if(ret==1)ret=2 ; ret;"
assert 5 "ret=3; if(ret==3)return ret+2; return 2;"
assert 4 "ret=2; if(ret=1)return 2*2; return 10; " # 条件式の中で代入
assert 9 "ret=3; if(ret==2)return 4; return 9;" # 条件式が偽
echo -e " if OK\n"

assert 10 "if(1) 10; else 20;" # if else 真
assert 20 "if(0) 10; else 20;" # if else 偽
assert 10 "if(1) return 10; else return 20;" # 上記　+ return 
assert 20 "if((3+1)*2-8) return 10; else return 20;" # 条件式に四則演算
assert 6 "ret=10; if(ret!=10)return 9;else return ret-=4;" # 変数や代入演算
assert 4 "ret=1; if(1)ret+=1; if(1)ret+=1; if(1)ret+=1; ret;"
assert 11 "if(0)return 10; else if(1)return 11;" # if, else if
assert 13 "if(0)return 11; else if(0)return 12; else return 13;" # if, else if, else
assert 12 "if(0)return 10; else if(0)return 11; else if(1)return 12;" # if, else if, else if
assert 25 "ret=11; if(0)ret=1; else if(1)ret=22; if(1)ret+=3;"
assert 30 "ret=11; if(1)ret+=4; else if(1)ret+=40; if(0)return 3; if(0)ret-=2; else if(1)ret=30; else ret=100; return ret;" # 多重ネストを多重ネスト
assert 1 "if(1)if(1)return 1; return 2;"
assert 42 "if(1)if(0)if(1)return 2; return 42;"
echo -e " if else OK\n"

# {}
assert 3 "{return 1+1+1;} "
# assert 3 "{ 1+1+1;}" # {}を抜けるときにpop するのでrax に不正な値が戻ってしまう
assert 4 "ret=1; {ret+=1;} {ret+=1;} {ret+=1;} ret;"
assert 1 "if(1){return 1;} return 2;"
assert 6 "ret=3; if(1){ret+=5; ret-=2;} return ret;"
assert 10 "ret=1; if(0){ret=0;}else if(1){ret=10;}else ret=0; return ret;"
echo -e " BLOCK OK\n"

# for(;;){}
assert 42 "for(;0;){1;} return 42;" # Bに何も書かないと恒真式になってしまう
assert 5 "ret=0; for(i=0; i<5; i+=1)ret+=1; return ret;"
assert 6 "ret=1; i=10; for(;i;i-=2){ret+=1; 12;} return ret;"
assert 20 "ret=0; for(i=0;i<5;i+=1)ret+=1; for(j=5;j<20;j+=1)ret+=1; return ret;" # 複数のfor
echo -e " for OK\n"

# COMMENTOUT
# semicolon 連続　を消費
# assert 42 "  42 ;;; ;; ;;; "
# assert 42 "3;;;;;   ; ;  ; ;;42;;;"
# assert 42 ";;;;3; ;; ;  ;42 ;;;  " # 前方セミコロン連続は未対応
# # assert 13 ";; i=10;  ;;for(;0;){;;;return 1;;;} ;;return i+=3;;;;"
# assert 10 "ret=10; return ret;;;;;; "
# assert 10 "ret=1; if(1){if(0)ret=2; else ret=3;};;;else ret=4; if(ret)return 10; else return 5;"

assert 2 "ret=3; cnt=5; while(ret){cnt-=1; ret-=1;} return cnt;"
assert 10 "ret=6; cnt=5; while(ret){if(cnt==2)return 10;  cnt-=1; ret-=1;} return cnt;"
assert 2 "x=3; ret=0; while(x<5){x+=1;ret+=1; } return ret;"

assert 15 "ret=10; do{ret+=5;}  while(0); return ret;"
assert 5 "ret=5; do return ret; while(1); return 10;"
echo -e "do while OK\n"

assert 2 "acc=0; for(x=0; x<10; x+=1){if(x<8)continue; acc+=1;} return acc;"
assert 3 "acc=0; x=0; while(x<10){x+=1; if(acc==3)continue; acc+=1;} return acc;"
assert 4 "acc=0; x=0; do{x+=1; if(acc==4)continue; acc+=1;} while(x!=4); return acc;"
echo -e "continue ok\n"

assert 2 "acc=0; for(; acc<10; acc+=1){ if(acc==2)break; } return acc;"
assert 3 "acc=0; x=0; while(x<10){x+=1; if(acc==3)break; acc+=1;} return acc;"
assert 4 "acc=0; x=0; do{x+=1; if(acc==4)break; acc+=1;} while(x!=4); return acc;"
echo -e "break ok\n"

assert 1 "{{return 1;}} {}{{{}}}"
assert 3 "ret=0; while(1){ while(1){if(ret==3)break;{} ret+=1;} break;} return ret;"
echo -e "block ネスト ok\n"
# x=6以下でセグメンテーションフォールト？
assert 10 "
x=7; acc=0;
while(x){
    for(i=0;i<5;i+=1){
        if(i==1)continue;

        if(x==1)acc+=5;
        else if(x==2)acc+=3;
        else acc+=1;

        if(i==1){
            if(x==3){
                if(acc==10)acc-=2;
                else {
                    acc+=2;
                }
            }
        }
    }
    x+=1;
    if(x==10) break;
}
return x;
"

assemble 2 "return 2;"

# assert 10 "
# x=6; acc=0;
# while(x){
#     for(i=0;i<5;i+=1){
#         acc=1;
#         if(acc==1){
#             x+=100;
#             x-=100;
#         }
#     }
    
#     x+=1;
#     if(x==10) break;
# }
# return x;
# "

# assert 100 "for(i=0;i<100;i+=1){} return i;"

echo -e "\n         You are a god-dammit genius !!\n"