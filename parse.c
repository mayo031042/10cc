#include "parse.h"
// RDP 関数のみ

#define MAX_FUNC_SIZE 100

Func *funcs[MAX_FUNC_SIZE];
int func_pos = 0;

// programの中の最小単位 (expr)か数値か変数、関数呼び出し しかありえない -> ここでの宣言はありえない
// -> ND_LVAR, ND_FUNC_CALL, ND_NUM, expr()
Node *primary()
{
    Node *node;

    // : ()
    if (consume(TK_RESERVED, "("))
    {
        node = expr();
        expect(TK_RESERVED, ")");
    }

    // 変数,関数
    else if (consume_ident())
    {
        // 関数かチェック
        if (current_token_is(TK_RESERVED, "("))
        {
            // 関数なので　関数呼び出しである
            node = create_node(ND_FUNC_CALL);

            node->func_num = find_func();
            if (node->func_num == -1)
            {
                error_at(tokens[val_of_ident_pos()]->str, "未定義な関数です");
            }

            // 引数を解釈する
            node->lhs = build_arg();
        }

        else
        {
            // 変数なので 一番近いブロック深度の中から合致する変数を探す なければエラー
            node = new_node_ident(find_lvar());
        }
    }

    // 数値
    else
    {
        node = new_node_num(expect_number());
    }

    return node;
}

// 単項演算子で区切る
// -> primary(), ND_SUB
Node *unary()
{
    Node *node;

    // : +a
    if (consume(TK_RESERVED, "+"))
    {
        node = primary();
    }
    // : -a -> 0-a に展開
    else if (consume(TK_RESERVED, "-"))
    {
        node = new_node(ND_SUB, new_node_num(0), primary());
    }
    else if (consume(TK_RESERVED, "*"))
    {
        node = new_node(ND_DEREF, unary(), NULL);
    }
    else if (consume(TK_RESERVED, "&"))
    {
        node = new_node(ND_ADDR, unary(), NULL);
    }
    else
    {
        node = primary();
    }

    return node;
}

// 乗除算で区切る
// -> unary(), ND_MUL, ND_DIV, ND_DIV_REM
Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        // : *
        if (consume(TK_RESERVED, "*"))
        {
            node = new_node(ND_MUL, node, unary());
        }
        // ; /
        else if (consume(TK_RESERVED, "/"))
        {
            node = new_node(ND_DIV, node, unary());
        }
        // : %
        else if (consume(TK_RESERVED, "%"))
        {
            node = new_node(ND_DIV_REM, node, unary());
        }

        else
        {
            break;
        }
    }

    return node;
}

// 加減算で区切る
// -> MUL(), ND_ADD, ND_SUB
Node *add()
{
    Node *node = mul();

    for (;;)
    {
        // : a+b
        if (consume(TK_RESERVED, "+"))
        {
            node = new_node(ND_ADD, node, mul());
        }
        // : a-b
        else if (TK_RESERVED, consume(TK_RESERVED, "-"))
        {
            node = new_node(ND_SUB, node, mul());
        }

        else
        {
            break;
        }
    }

    return node;
}

// 不等号で区切る
// -> add(), ND_LE, ND_LT
Node *relational()
{
    Node *node = add();

    for (;;)
    {
        // 2文字の不等号を先に処理する
        if (consume(TK_RESERVED, "<="))
        {
            node = new_node(ND_LE, node, add());
        }
        else if (consume(TK_RESERVED, ">="))
        {
            node = new_node(ND_LE, add(), node);
        }
        // 次に１文字の不等号を処理
        else if (consume(TK_RESERVED, "<"))
        {
            node = new_node(ND_LT, node, add());
        }
        else if (consume(TK_RESERVED, ">"))
        {
            node = new_node(ND_LT, add(), node);
        }

        else
        {
            break;
        }
    }

    return node;
}

// 等号　等号否定で区切る　
// -> relational(), ND_EQ, ND_NE
Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume(TK_RESERVED, "=="))
        {
            node = new_node(ND_EQ, node, relational());
        }
        else if (consume(TK_RESERVED, "!="))
        {
            node = new_node(ND_NE, node, relational());
        }

        else
        {
            break;
        }
    }

    return node;
}

// 計算式を代入式　代入演算子で区切る -> 区切られた後は代入式等は出現しない
// -> equality(), ND_ASSIGN
Node *assign()
{
    // == ,!=, < 等はequality()内で優先的に処理されている
    Node *node = equality();

    if (consume(TK_RESERVED, "="))
    {
        node = new_node(ND_ASSIGN, assign(), node);
    }

    else if (tokens[token_pos]->kind == TK_ASSIGN_RESERVED)
    {
        if (consume(TK_ASSIGN_RESERVED, "+="))
        {
            node = new_node(ND_ASSIGN, new_node(ND_ADD, node, assign()), node);
        }
        else if (consume(TK_ASSIGN_RESERVED, "-="))
        {
            node = new_node(ND_ASSIGN, new_node(ND_SUB, node, assign()), node);
        }
        else if (consume(TK_ASSIGN_RESERVED, "*="))
        {
            node = new_node(ND_ASSIGN, new_node(ND_MUL, node, assign()), node);
        }
        else if (consume(TK_ASSIGN_RESERVED, "/="))
        {
            node = new_node(ND_ASSIGN, new_node(ND_DIV, node, assign()), node);
        }
        else if (consume(TK_ASSIGN_RESERVED, "%="))
        {
            node = new_node(ND_ASSIGN, new_node(ND_DIV_REM, node, assign()), node);
        }
        else
        {
            error_at(tokens[token_pos]->str, "代入演算子ではありません\n");
        }
    }

    return node;
}

// 予約語のない純粋な計算式として解釈する 三項間演算子もここ？
// void と四則演算しない　など計算結果が整数であることを保証したい
// -> assign()
Node *expr()
{
    return assign();
}

// 予約語,{} 変数宣言の解釈を行う
// -> expr(), ND_RETURN, ND_ELSE, ND_FOR_WHILE, ND_DO, ND_CONTINUE, ND_BREAK, ND_BLOCK
// 意味のない 空欄+; はブロックで処理する
Node *stmt()
{
    Node *node;

    // : {}
    if (consume_keyword(TK_BLOCK_FRONT))
    {
        node = build_block();
    }
    // : if
    else if (consume_keyword(TK_IF))
    {
        node = new_node_else();
    }
    // : for
    else if (consume_keyword(TK_FOR))
    {
        node = new_node_for();
    }
    // : while
    else if (consume_keyword(TK_WHILE))
    {
        node = new_node_while();
    }
    // : do{} while();
    else if (consume_keyword(TK_DO))
    {
        node = new_node_do();
    }

    else
    {
        // 変数の宣言はcodegen() としては何も出力しない
        // : int
        if (current_token_is_type())
        {
            node = declare_lvar();
        }
        // : return
        else if (consume_keyword(TK_RETURN))
        {
            node = new_node(ND_RETURN, expr(), NULL);
        }
        // : continue
        else if (consume_keyword(TK_CONTINUE))
        {
            node = create_node(ND_CONTINUE);
        }
        // : break;
        else if (consume_keyword(TK_BREAK))
        {
            node = create_node(ND_BREAK);
        }
        else
        {
            node = expr();
        }

        expect(TK_RESERVED, ";");
    }

    return node;
}

// code全体を　;　で区切る
// -> ND_BLOCK
Node *program()
{
    expect(TK_BLOCK_FRONT, "{");
    return build_block();
}

// 関数の宣言か定義のみを扱う
void *function()
{
    int i = 0;

    while (!at_eof())
    {
        expect_vartype();
        expect_ident();

        // 識別子から　今までに登録されている関数列を全探索する
        int declared = find_func();

        if (declared == -1)
        {
            // 関数がはじめて宣言、定義されるので　funcs[] と引数リストの登録を行う
            funcs[i] = new_func(tokens[val_of_ident_pos()]);
            funcs[i + 1] = NULL;
            func_pos = i;
            i++;
            declare_arg();
        }
        else
        {
            // 既に登録済みの関数なので　func_pos のセットだけ行い　引数リストは読み飛ばす
            func_pos = declared;
            consume_arg();
        }

        // 宣言のみなら　次の関数読み込みに移る
        if (consume(TK_RESERVED, ";"))
        {
            continue;
        }

        // 定義がくるので　多重定義されていないか確認する
        if (funcs[func_pos]->defined == true)
        {
            error_at(tokens[token_pos]->str, "関数が多重定義されています");
        }

        funcs[func_pos]->defined = true;
        funcs[func_pos]->definition = program();
    }

    // 各関数のmax offset を、それを超えるような最小の16の倍数で改める
    for (i--; 0 <= i; i--)
    {
        funcs[i]->max_offset = (funcs[i]->max_offset + 16 - 1) / 16 * 16;
    }
}