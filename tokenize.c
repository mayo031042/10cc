#include "10cc.h"

#define MAX_TOKEN_SIZE 200

Token *tokens[MAX_TOKEN_SIZE];
int ident_pos;

// 新しいtoken に{種類、文字列、長さ} を登録し　今のtoken のnext としてつなげる
// tokenの作成はここだけでしか行われないので　tknz中のpos の移動はここでのみおこなう
void new_token(TokenKind kind, char *str, int len)
{
    tokens[token_pos] = calloc(1, sizeof(Token));
    tokens[token_pos]->kind = kind;
    tokens[token_pos]->str = str;
    tokens[token_pos]->len = len;
    tokens[token_pos]->pos = token_pos;
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

void *tokenize()
{
    char *p = user_input;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        // 予約語ゾーン
        if (is_keyword(p, "return", 6))
        {
            new_token(TK_RETURN, p, 6);
            p += 6;
            continue;
        }
        else if (is_keyword(p, "if", 2))
        {
            new_token(TK_IF, p, 2);
            p += 2;
            continue;
        }
        else if (is_keyword(p, "else", 4))
        {
            new_token(TK_ELSE, p, 4);
            p += 4;
            continue;
        }
        else if (is_keyword(p, "while", 5))
        {
            new_token(TK_WHILE, p, 5);
            p += 5;
            continue;
        }
        else if (is_keyword(p, "for", 3))
        {
            new_token(TK_FOR, p, 3);
            p += 3;
            continue;
        }
        else if (is_keyword(p, "do", 2))
        {
            new_token(TK_DO, p, 2);
            p += 2;
            continue;
        }
        else if (is_keyword(p, "continue", 8))
        {
            new_token(TK_CONTINUE, p, 8);
            p += 8;
            continue;
        }
        else if (is_keyword(p, "break", 5))
        {
            new_token(TK_BREAK, p, 5);
            p += 5;
            continue;
        }

        // : { or }
        if (strchr("{}", *p))
        {
            if (*p == '{')
            {
                new_token(TK_BLOCK_FRONT, p, 1);
            }
            else
            {
                new_token(TK_BLOCK_END, p, 1);
            }
            p++;
            continue;
        }

        // 2文字演算子ゾーン
        if (strchr("+-", *p) && *p == *(p + 1))
        {
            // インクリメント　の実装
        }
        else if (strchr("+-*/%!=<>", *p) && *(p + 1) == '=')
        {
            if (strchr("!=<>", *p))
            {
                new_token(TK_RESERVED, p, 2);
            }
            else
            {
                new_token(TK_ASSIGN_RESERVED, p, 2);
            }
            p += 2;
            continue;
        }
        // 1文字解釈ゾーン
        else if (strchr("+-*/%()=<>;", *p))
        {
            new_token(TK_RESERVED, p++, 1);
            continue;
        }

        // 数値、変数、関数名 解釈ゾーン　
        if (isdigit(*p))
        {
            new_token(TK_NUM, p, 0);
            tokens[token_pos - 1]->val = strtol(p, &p, 10);
            continue;
        }
        else if (is_alnum(*p))
        {
            char *q;
            for (q = p; is_alnum(*q); q++)
                ;
            new_token(TK_IDENT, p, q - p);
            p = q;
            continue;
        }

        error_at(tokens[token_pos]->str, "%s tokenizeできません\n", p);
    }

    new_token(TK_EOF, p, 0);
    token_pos = 0;
}
