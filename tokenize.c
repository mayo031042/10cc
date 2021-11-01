#include "tokenize.h"

#define MAX_TOKEN_SIZE 200

Token *tokens[MAX_TOKEN_SIZE];
int token_pos = 0;
int ident_pos = 0;
char *user_input_pos;

void *tokenize()
{
    user_input_pos = user_input;

    while (*user_input_pos)
    {
        if (isspace(*user_input_pos))
        {
            user_input_pos++;
            continue;
        }

        // 予約語ゾーン
        if (is_keyword("return"))
        {
            new_token(TK_RETURN, user_input_pos, 6);
            user_input_pos += 6;
            continue;
        }
        else if (is_keyword("if"))
        {
            new_token(TK_IF, user_input_pos, 2);
            user_input_pos += 2;
            continue;
        }
        else if (is_keyword("else"))
        {
            new_token(TK_ELSE, user_input_pos, 4);
            user_input_pos += 4;
            continue;
        }
        else if (is_keyword("while"))
        {
            new_token(TK_WHILE, user_input_pos, 5);
            user_input_pos += 5;
            continue;
        }
        else if (is_keyword("for"))
        {
            new_token(TK_FOR, user_input_pos, 3);
            user_input_pos += 3;
            continue;
        }
        else if (is_keyword("do"))
        {
            new_token(TK_DO, user_input_pos, 2);
            user_input_pos += 2;
            continue;
        }
        else if (is_keyword("continue"))
        {
            new_token(TK_CONTINUE, user_input_pos, 8);
            user_input_pos += 8;
            continue;
        }
        else if (is_keyword("break"))
        {
            new_token(TK_BREAK, user_input_pos, 5);
            user_input_pos += 5;
            continue;
        }

        // : { or }
        if (strchr("{}", *user_input_pos))
        {
            if (*user_input_pos == '{')
            {
                new_token(TK_BLOCK_FRONT, user_input_pos, 1);
            }
            else
            {
                new_token(TK_BLOCK_END, user_input_pos, 1);
            }
            user_input_pos++;
            continue;
        }

        // 2文字演算子ゾーン
        if (strchr("+-", *user_input_pos) && *user_input_pos == *(user_input_pos + 1))
        {
            // インクリメント　の実装
        }
        else if (strchr("+-*/%!=<>", *user_input_pos) && *(user_input_pos + 1) == '=')
        {
            if (strchr("!=<>", *user_input_pos))
            {
                new_token(TK_RESERVED, user_input_pos, 2);
            }
            else
            {
                new_token(TK_ASSIGN_RESERVED, user_input_pos, 2);
            }
            user_input_pos += 2;
            continue;
        }
        // 1文字解釈ゾーン
        else if (strchr("+-*/%()=<>;", *user_input_pos))
        {
            new_token(TK_RESERVED, user_input_pos++, 1);
            continue;
        }

        // 数値、変数、関数名 解釈ゾーン　
        if (isdigit(*user_input_pos))
        {
            new_token(TK_NUM, user_input_pos, 0);
            tokens[token_pos - 1]->val = strtol(user_input_pos, &user_input_pos, 10);
            continue;
        }
        else if (is_alnum(*user_input_pos))
        {
            char *p;
            for (p = user_input_pos; is_alnum(*p); p++)
                ;
            new_token(TK_IDENT, user_input_pos, p - user_input_pos);
            user_input_pos = p;
            continue;
        }

        error_at(tokens[token_pos]->str, "%s tokenizeできません\n", user_input_pos);
    }

    new_token(TK_EOF, user_input_pos, 0);
    token_pos = 0;
}
