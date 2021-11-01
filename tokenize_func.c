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
