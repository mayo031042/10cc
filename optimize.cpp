#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <bitset>
#include <set>
#include <string>
#include <cmath>
#include <random>
using namespace std;
using ll = long long int;
using ss = string;
#define rep(i, s, n) for (ll i = (s); i < (ll)(n); i++)
#define all(v) v.begin(), v.end()

ifstream ifs;
ofstream ofs;

void sor()
{
    ifs.open("tmp/tmp.s", ios::out);
    ofs.open("tmp/tmp_opt.s", ios::out);

    if (!ifs or !ofs)
    {
        cout << "cannot open" << endl;
        return;
    }
}

void eor()
{
    ifs.close();
    ofs.close();
}

vector<ss> cds;

void clr(ss &s)
{
    int n = s.size();
    rep(i, 0, n)
    {
        if (s[i] != ' ')
        {
            s = s.substr(i, n - i);
            return;
        }
    }
}

void add(ss &s)
{
    cds.push_back(s);
    s = "";
}

vector<vector<ss>> prg;
vector<ss> lns;

void jdg(const ss &s1)
{
    lns.push_back(s1);
    cds.clear();
    ss s = s1;
    clr(s);
    if (s[0] != 'p')
    {
        prg.push_back({"x", "x"});
        return;
    }

    ss buf;
    rep(i, 0, s.size())
    {
        if (s[i] == ' ' or s[i] == ',')
        {
            add(buf);
        }
        else
        {
            buf += s[i];
        }
    }

    if (buf != "")
    {
        add(buf);
    }

    prg.push_back(move(cds));
}

void lnspb(ss rg, ss im)
{
    if (rg != im)
    {
        lns.push_back("    mov " + rg + ", " + im);
    }
}

void write()
{
    int n = prg.size();
    ss fst = prg[n - 1][0];

    if (n <= 2 and fst == "push")
    {
        return;
    }
    else if (n >= 2 and fst == "pop")
    {
        if (n == 2)
        {
            lns.clear();
            ss im = prg[0][1];
            ss rg = prg[1][1];

            lnspb(rg, im);
        }
        else if (n == 4)
        {
            lns.clear();
            ss im1 = prg[0][1];
            ss im2 = prg[1][1];
            ss rg2 = prg[2][1];
            ss rg1 = prg[3][1];
            lnspb(rg1, im1);
            lnspb(rg2, im2);
        }
    }

    for (auto &x : lns)
    {
        ofs << x << endl;
    }
    lns.clear();
    prg.clear();
}

int main()
{
    sor();

    ss s;
    rep(i, 0, 3)
    {
        getline(ifs, s);
        ofs << s << endl;
    }

    while (getline(ifs, s))
    {
        jdg(s);
        write();
    }

    eor();
}