#include "tokenize.h"

// 新しいtoken に{種類、文字列、長さ} を登録し　今のtoken のnext としてつなげる
// tokenの作成はここだけでしか行われないので　tknz中のpos の移動はここでのみおこなう
void new_token(TokenKind kind, char *str, int len)
{
    tokens[token_pos] = calloc(1, sizeof(Token));
    tokens[token_pos]->kind = kind;
    tokens[token_pos]->str = str;
    tokens[token_pos]->len = len;
    token_pos++;
}

// 変数に使える文字か否かを返す -> token 構成文字
int is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

// p とs を比較して　pがkeyword　と判定できたらtrue
bool is_keyword(char *p, char *str, int num)
{
    return (!strncmp(p, str, num) && !is_alnum(p[num]));
}



