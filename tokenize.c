#include "10cc.h"

bool is_expected_token(TokenKind kind, char *op)
{
    if (tokens[pos]->kind != kind || tokens[pos]->len != strlen(op) || memcmp(tokens[pos]->str, op, tokens[pos]->len))
        return false;
    return true;
}

// 今見ているtoken と引数の文字列が一致していたら　true を返しtoken を読み進める
bool consume(TokenKind kind, char *op)
{
    if (!is_expected_token(kind, op))
        return false;
    pos++;
    return true;
}

// consumeと同じ判定をするが　falseが返る場合は代わりにerror を吐く
bool expect(TokenKind kind, char *op)
{
    if (consume(kind, op))
        return true;
    error_at(tokens[pos]->str, "%cではありません\n", op);
}

// expectと同様にflase ならerrorを吐く　true ならtoken に数値を登録し読み進める
int expect_number()
{
    if (tokens[pos]->kind != TK_NUM)
        error_at(tokens[pos]->str, "数値ではありません\n");
    int val = tokens[pos]->val;
    pos++;
    return val;
}

// token が識別子である時　そのtoken のコピーを返し　token を読み進める
int consume_ident()
{
    if (tokens[pos]->kind != TK_IDENT)
        return -1;
    return tokens[pos++]->pos;
}

bool consume_keyword(TokenKind kind)
{
    if (tokens[pos]->kind != kind)
        return false;
    pos++;
    return true;
}

// token 列の最後尾の次だったらtrue
bool at_eof()
{
    return tokens[pos]->kind == TK_EOF;
}

// 新しいtoken に{種類、文字列、長さ} を登録し　今のtoken のnext としてつなげる
// tokenの作成はここだけでしか行われないので　tknz中のpos の移動はここでのみおこなう
void new_token(TokenKind kind, char *str, int len)
{
    tokens[pos] = calloc(1, sizeof(Token));
    tokens[pos]->kind = kind;
    tokens[pos]->str = str;
    tokens[pos]->len = len;
    tokens[pos]->pos = pos;
    pos++;
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
    return !strncmp(p, str, num) && !is_alnum(p[num]);
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
                new_token(TK_BLOCK_FRONT, p, 1);
            else
                new_token(TK_BLOCK_END, p, 1);
            p++;
            continue;
        }

        // 2文字演算子ゾーン
        if (strchr("+-", *p) && *p == *(p + 1))
        {
            // インクリメント　の実装
        }
        else if (strchr("+-*/!=<>", *p) && *(p + 1) == '=')
        {
            if (strchr("!=<>", *p))
                new_token(TK_RESERVED, p, 2);
            else
                new_token(TK_ASSIGN_RESERVED, p, 2);
            p += 2;
            continue;
        }
        // 1文字解釈ゾーン
        else if (strchr("+-*/()=<>;", *p))
        {
            new_token(TK_RESERVED, p++, 1);
            continue;
        }

        // 数値、変数解釈ゾーン　
        if (isdigit(*p))
        {
            new_token(TK_NUM, p, 0);
            tokens[pos - 1]->val = strtol(p, &p, 10);
            continue;
        }
        else if (is_alnum(*p))
        {
            char *q = p;
            for (; is_alnum(*q); q++)
                ;
            new_token(TK_IDENT, p, q - p);
            p = q;
            continue;
        }

        error_at(tokens[pos]->str, "%s tokenizeできません\n", p);
    }

    new_token(TK_EOF, p, 0);
    pos = 0;
}
