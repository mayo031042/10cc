#include "tokenize.h"

// uip とstr を比較して　uipがkeyword　と判定できたらtrue
bool is_keyword(char *str)
{
    int num = strlen(str);
    return (!strncmp(user_input_pos, str, num) && !is_alnum(user_input_pos[num]));
}

// 新しいtoken に{種類、文字列ptr、長さ} を登録し　uipを進める
// tokenの作成はここだけでしか行われないので　token_pos の移動はここでのみおこなう
void new_token(TokenKind kind, int len)
{
    tokens[token_pos] = calloc(1, sizeof(Token));
    tokens[token_pos]->kind = kind;
    tokens[token_pos]->str = user_input_pos;
    tokens[token_pos]->len = len;
    token_pos++;
    user_input_pos += len;
}

// 変数に使える文字か否かを返す -> token 構成文字
int is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

// コメントアウトを処理したときはtrue しなかったときはfalse を返す
bool commentout()
{
    // 改行までtknz しない
    if (strncmp(user_input_pos, "//", 2) == 0)
    {
        user_input_pos += 2;
        while (*user_input_pos != '\n')
        {
            user_input_pos++;
        }
        return true;
    }

    /* 終端記号が出るまでtknz しない　*/
    else if (strncmp(user_input_pos, "/*", 2) == 0)
    {
        // 開始記号の次から探しはじめて　発見した終端記号の先頭のポインタｗｐ返す
        char *p = strstr(user_input_pos + 2, "*/");
        // 終端記号が見つからないとNULL が返る
        if (!p)
        {
            error_at(tokens[token_pos]->str, "コメントが閉じられていません");
        }

        // コメントアウトの次のtoken にセット
        user_input_pos = p + 2;
        return true;
    }

    return false;
}
