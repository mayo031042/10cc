#include "parse.h"
// tokens[] やtoken_pos を扱い parse を補助する関数など
// また node やfuncs[] の作成を補助する関数など

// 最後に確認した識別子のtoken_pos
int ident_pos = 0;

bool current_token_is(TokenKind kind, char *op)
{
    return (tokens[token_pos]->kind == kind && tokens[token_pos]->len == strlen(op) && !memcmp(tokens[token_pos]->str, op, tokens[token_pos]->len));
}

// 今見ているtoken と引数の文字列が一致していたら　true を返しtoken を読み進める
bool consume(TokenKind kind, char *op)
{
    if (!current_token_is(kind, op))
    {
        return false;
    }
    token_pos++;
    return true;
}

// consumeと同じ判定をするが　falseが返る場合は代わりにerror を吐く
bool expect(TokenKind kind, char *op)
{
    if (consume(kind, op))
    {
        return true;
    }
    error_at(tokens[token_pos]->str, "%cではありません\n", op);
}

// expectと同様にflase ならerrorを吐く　true ならtoken に数値を登録し読み進める
int expect_number()
{
    if (tokens[token_pos]->kind != TK_NUM)
    {
        error_at(tokens[token_pos]->str, "数値ではありません\n");
    }
    int val = tokens[token_pos]->val;
    token_pos++;
    return val;
}

// 今のtoken が引数通りなら読み進める　更に識別子のときはident_pos を更新する
bool consume_keyword(TokenKind kind)
{
    if (tokens[token_pos]->kind != kind)
    {
        return false;
    }
    if (kind == TK_IDENT)
    {
        ident_pos = token_pos;
    }
    token_pos++;
    return true;
}

// token 列の最後尾の次だったらtrue
bool at_eof()
{
    return (tokens[token_pos]->kind == TK_EOF);
}

int val_of_ident_pos()
{
    return ident_pos;
}

// 以下　Node LVar Func を扱う関数

Node *create_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

// node に種類と それにつながる左辺、右辺を登録する
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = create_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 数値を指すnode を作成する　常に葉であるような終端記号になるので左右に辺は持たない
Node *new_node_num(int val)
{
    Node *node = create_node(ND_NUM);
    node->val = val;
    return node;
}

// LVarを参照する
Node *new_node_ident(LVar *lvar)
{
    Node *node = create_node(ND_LVAR);
    node->offset = lvar->offset;
    return node;
}

// stmt を期待する位置の直後に；が来ている場合　1push 保証の観点から 代わりにcreate_node() する
Node *create_or_stmt(NodeKind kind)
{
    Node *node;
    if (consume(TK_RESERVED, ";"))
    {
        node = create_node(kind);
    }
    else
    {
        node = stmt();
    }

    return node;
}

// 完全に独立してif node を完成させる
Node *new_node_if()
{
    Node *node = create_node(ND_IF);
    expect(TK_RESERVED, "(");
    node->lhs = expr();
    expect(TK_RESERVED, ")");
    node->rhs = create_or_stmt(ND_PUSH_0);
    return node;
}

// if の時点で else を作る　else node　の左辺にif を配置 右辺にはNULLかstmt()
// 条件分岐の終了はif かelse -> if で終了するときはelse 0; の省略と解釈する
Node *new_node_else()
{
    // 既にif があることがわかっていて消費されている
    Node *node = create_node(ND_ELSE);
    node->lhs = new_node_if();

    // 条件分岐制御構文がまだ続くなら
    if (consume_keyword(TK_ELSE))
    {
        // else if として続くなら
        if (consume_keyword(TK_IF))
        {
            node->rhs = new_node_else();
        }
        // else だけで終わるなら
        else
        {
            node->rhs = create_or_stmt(ND_PUSH_0);
        }
    }
    // if で終了するならelse 0; の省略として処理
    else
    {
        node->rhs = create_node(ND_PUSH_0);
    }

    return node;
}

// 孫node に引数順のnode を持つnode を作成して返す
Node *new_grand_node(NodeKind kind, Node *l_l, Node *l_r, Node *r_l, Node *r_r)
{
    Node *lhs = new_node(ND_NOP, l_l, l_r);
    Node *rhs = new_node(ND_NOP, r_l, r_r);
    return new_node(kind, lhs, rhs);
}

Node *new_node_for()
{
    expect(TK_RESERVED, "(");

    Node *nodes[3];
    char *op[] = {";", ";", ")"};
    for (int i = 0; i < 3; i++)
    {
        if (current_token_is(TK_RESERVED, op[i]))
        {
            // 条件式が空欄な時は恒真式なので　１が入っているとしてparseする
            nodes[i] = create_node(ND_PUSH_1);
        }
        else
        {
            nodes[i] = expr();
        }

        expect(TK_RESERVED, op[i]);
    }

    return new_grand_node(ND_FOR_WHILE, nodes[0], nodes[2], create_or_stmt(ND_PUSH_0), nodes[1]);
}

Node *new_node_while()
{
    expect(TK_RESERVED, "(");
    Node *node_B = expr(); // 空欄を許さない
    expect(TK_RESERVED, ")");

    // A式, C式はfor の空欄時に従って　ND_PUSH_1 を入れておく
    return new_grand_node(ND_FOR_WHILE, create_node(ND_PUSH_1), create_node(ND_PUSH_1), create_or_stmt(ND_PUSH_0), node_B);
}

Node *new_node_do()
{
    Node *lhs = create_or_stmt(ND_PUSH_0);

    expect(TK_WHILE, "while");
    expect(TK_RESERVED, "(");
    Node *rhs = expr();
    expect(TK_RESERVED, ")");
    expect(TK_RESERVED, ";");

    return new_node(ND_DO, lhs, rhs);
}
// : }が出現するまでnextつなぎにnode を登録していく　
// 全体として繋がれたnode の先頭を返す 終端はNULL
Node *new_node_block()
{
    // 意味のある; はstmt() 内で処理をする
    while (consume(TK_RESERVED, ";"))
        ;

    if (consume_keyword(TK_BLOCK_END))
    {
        return NULL;
    }

    Node *node = stmt();
    node->next = new_node_block();
    return node;
}

// build_block() とgen_block() でのみ block_nest の値をいじる
Node *build_block()
{
    block_nest++;

    Node *node = new_node(ND_BLOCK, new_node_block(), NULL);

    block_nest--;
    return node;
}

// 引数に渡された値から更に自分の必要とするメモリサイズ分下げたoffset が登録されたLVar を作成する
// さらに該当関数のmax_offset も更新し　locals[] の最新を変更する
// funcs[]->locals[] は常に　該当関数の該当ネスト部分で作成された最新の変数を保持している（offset がそのネストの中で最大）
// name, len, offset, next が登録された変数を作成する
// 関数のmax_offset が常に最新の登録変数のoffset を指しているとは限らないので　引数を採る
LVar *new_lvar(int max_offset)
{
    // 8 は自分の型に合わせて　要変更
    int my_offset = max_offset + 8;
    if (funcs[func_pos]->max_offset < my_offset)
    {
        funcs[func_pos]->max_offset = my_offset;
    }

    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->name = tokens[val_of_ident_pos()]->str;
    lvar->len = tokens[val_of_ident_pos()]->len;

    lvar->offset = my_offset;
    lvar->next = funcs[func_pos]->locals[block_nest];

    funcs[func_pos]->locals[block_nest] = lvar;
    return lvar;
}

// func_pos を参照しつつ引数のブロック深度内のみから 直前識別子名に合致する変数を探す
LVar *find_lvar_within_block(int depth)
{
    for (LVar *lvar = funcs[func_pos]->locals[depth]; lvar; lvar = lvar->next)
    {
        // 指定ブロック深度内に　条件に合致する変数を発見したときは その変数を返す
        if (lvar->len == tokens[val_of_ident_pos()]->len && !memcmp(lvar->name, tokens[val_of_ident_pos()]->str, lvar->len))
        {
            return lvar;
        }
    }

    return NULL;
}

// 既出変数から直前識別子名に一致するものを探す
// offset の算出のみが目的なら　最大のoffset を計算して必ず 変数を新規登録する
// only_culc_offset がfalse なら探索も行い合致する変数が見つかればそれを　なければ新規登録して返す
LVar *find_lvar(bool only_culc_offset)
{
    int max_offset = 0;
    LVar *lvar;

    for (int depth = block_nest; 0 <= depth; depth--)
    {
        lvar = funcs[func_pos]->locals[depth];
        if (lvar == NULL)
        {
            continue;
        }

        // 最大のoffset を保持するmax_offset を更新する -> 変数が１つ以上登録されているならば　１度だけ更新される
        if (max_offset < lvar->offset)
        {
            max_offset = lvar->offset;
        }

        if (only_culc_offset == false)
        {
            lvar = find_lvar_within_block(depth);
            if (lvar)
            {
                return lvar;
            }
        }
    }

    if (only_culc_offset == false)
    {
        // 宣言なしに作成はできないのでエラー
        error_at(tokens[token_pos]->str, "宣言されていない変数です");
    }

    // 登録されている既出変数の中で最大のoffsetを渡す
    // -> 現在のブロックの最新変数のoffset を参照する方法ではNULLに対応できない
    return new_lvar(max_offset);
}

// 変数の宣言について扱う　型部分だけ既に読み勧めている
// funcs[]->locals[]にlvar を登録する
Node *lvar_declare()
{
    if (!consume_keyword(TK_IDENT))
    {
        error_at(tokens[token_pos]->str, "変数ではありません");
    }

    LVar *lvar = find_lvar_within_block(block_nest);

    // 現在のスコープの中から同一変数が見つかればエラー
    if (lvar)
    {
        error_at(tokens[token_pos]->str, "既に宣言されている変数です");
    }

    // 新規変数なので上位ブロック深度に合致する変数が存在するしないに関わらず　必ず新規登録する
    find_lvar(true);

    return create_node(ND_PUSH_0);
}

// 既出関数名から直前識別子名に一致するものを探す　
// 引数によってerror の有無を分岐させ　既出ならそのfuncs[] のpos を返す
int find_func(bool serach_only)
{
    for (int i = 0; funcs[i]; i++)
    {
        if (!memcmp(tokens[val_of_ident_pos()]->str, funcs[i]->name, funcs[i]->len))
        {
            return i;
        }
    }

    if (serach_only == true)
    {
        return -1;
    }
    else
    {
        error_at(tokens[val_of_ident_pos()]->str, "未定義な関数です");
    }
}

// 新しいfuncに len, max_offset, defined, name を設定
Func *new_func(Token *tok)
{
    Func *func = calloc(1, sizeof(Func));
    func->len = tok->len;
    func->max_offset = 0;
    func->defined = false;
    strncpy(func->name, tok->str, tok->len);
    return func;
}

// func_pos で指定された関数の引数の最大offset を返す
int arg_offset(int func_pos)
{
    if (funcs[func_pos]->locals[0])
    {
        return funcs[func_pos]->locals[0]->offset;
    }
    else
    {
        return 0;
    }
}