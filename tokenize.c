#include "tokenize.h"

#define MAX_TOKEN_SIZE 5000

Token *tokens[MAX_TOKEN_SIZE];
int token_pos = 0;
char *user_input_pos;

void *tokenize()
{
    user_input_pos = user_input;

    while (*user_input_pos)
    {
        // 空白飛ばし
        if (isspace(*user_input_pos))
        {
            user_input_pos++;
            continue;
        }

        // コメントアウト飛ばし
        if (commentout())
        {
            continue;
        }

        // 変数の型ゾーン
        if (is_keyword("void"))
        {
            new_token(TK_TYPE, 4);
            continue;
        }
        else if (is_keyword("char"))
        {
            new_token(TK_TYPE, 4);
            continue;
        }
        else if (is_keyword("int"))
        {
            new_token(TK_TYPE, 3);
            continue;
        }
        else if (is_keyword("bool"))
        {
            new_token(TK_TYPE, 4);
            continue;
        }

        // 予約語ゾーン
        if (is_keyword("return"))
        {
            new_token(TK_RETURN, 6);
            continue;
        }
        else if (is_keyword("if"))
        {
            new_token(TK_IF, 2);
            continue;
        }
        else if (is_keyword("else"))
        {
            new_token(TK_ELSE, 4);
            continue;
        }
        else if (is_keyword("while"))
        {
            new_token(TK_WHILE, 5);
            continue;
        }
        else if (is_keyword("for"))
        {
            new_token(TK_FOR, 3);
            continue;
        }
        else if (is_keyword("do"))
        {
            new_token(TK_DO, 2);
            continue;
        }
        else if (is_keyword("continue"))
        {
            new_token(TK_CONTINUE, 8);
            continue;
        }
        else if (is_keyword("break"))
        {
            new_token(TK_BREAK, 5);
            continue;
        }
        else if (is_keyword("sizeof"))
        {
            new_token(TK_SIZEOF, 6);
            continue;
        }
        else if (is_keyword("switch"))
        {
            new_token(TK_SWITCH, 6);
            continue;
        }
        else if (is_keyword("case"))
        {
            new_token(TK_CASE, 4);
            continue;
        }

        // : { }
        if (*user_input_pos == '{')
        {
            new_token(TK_BLOCK_FRONT, 1);
            continue;
        }
        else if (*user_input_pos == '}')
        {
            new_token(TK_BLOCK_END, 1);
            continue;
        }

        // 2文字演算子ゾーン
        if (strchr("+-", *user_input_pos) && *user_input_pos == *(user_input_pos + 1))
        {
            // インクリメント の実装
        }
        else if (strchr("+-*/%!=<>", *user_input_pos) && *(user_input_pos + 1) == '=')
        {
            if (strchr("!=<>", *user_input_pos))
            {
                new_token(TK_RESERVED, 2);
            }
            else
            {
                new_token(TK_ASSIGN_OPERATOR, 2);
            }
            continue;
        }

        // 1文字解釈ゾーン
        if (strchr("+-*/%&", *user_input_pos))
        {
            new_token(TK_OPERATOR, 1);
            continue;
        }
        else if (strchr("()[]=<>,:;", *user_input_pos))
        {
            new_token(TK_RESERVED, 1);
            continue;
        }

        // 数値、変数、関数名 解釈ゾーン
        if (isdigit(*user_input_pos))
        {
            new_token(TK_NUM, 0);
            tokens[token_pos - 1]->val = strtol(user_input_pos, &user_input_pos, 10);
            continue;
        }
        else if (is_alnum(*user_input_pos))
        {
            char *p;
            for (p = user_input_pos; is_alnum(*p); p++)
                ;
            new_token(TK_IDENT, p - user_input_pos);
            continue;
        }

        error_at(tokens[token_pos]->str, "tokenizeできません\n");
    }

    new_token(TK_EOF, 0);
    token_pos = 0;
}
