#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <bitset>
#include <cmath>
#include <random>
using namespace std;
using ll = long long int;
#define rep(i, s, n) for (ll i = (s); i < (ll)(n); i++)
#define all(v) v.begin(), v.end()

int siz = 10000;

int main()
{
    ofstream ofs("test/sort/sort.c");
    if (!ofs)
    {
        cout << "cannot open" << endl;
        return 0;
    }

    vector<int> v(siz);
    random_device rnd; // 非決定的な乱数生成器
    mt19937 mt(rnd()); // メルセンヌ・ツイスタの32ビット版、引数は初期シード

    ofs << "int main()\n{" << endl;
    ofs << "int v1[" << siz << "];" << endl;

    rep(i, 0, siz)
    {
        ll x = rnd();
        v[i] = abs(x) >> 2;
    }

    rep(i, 0, siz)
    {
        ofs << "v1[" << i << "] = " << v[i] << ";" << endl;
    }

    ofs << "int i;\nint j;\nint ma;" << endl;
    ofs << "for(i = 0;i < " << siz << ";i += 1)\n{" << endl;
    ofs << "ma=v1[0];" << endl;
    ofs << "for(j = 1;j + i < " << siz << ";j += 1)\n{" << endl;
    ofs << "if(ma<=v1[j])\n{" << endl;
    ofs << "ma=v1[j];\n}\nelse\n{" << endl;
    ofs << "v1[j-1]=v1[j];\nv1[j]=ma;\n}" << endl;
    ofs << "}\n}" << endl;

    sort(v.begin(), v.end());

    ofs << "int v2[" << siz << "];" << endl;
    rep(i, 0, siz)
    {
        ofs << "v2[" << i << "] = " << v[i] << ";" << endl;
    }

    ofs << "for(i=0;i<" << siz << ";i+=1)\n{" << endl;
    ofs << "if(v1[i]!=v2[i])" << endl;
    ofs << "{\nreturn 1;\n}\n}" << endl;
    ofs << "return 0;\n}" << endl;

    ofs.close();
}