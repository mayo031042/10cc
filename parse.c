#include "parse.h"
// RDP 関数のみ

#define MAX_FUNC_SIZE 1000

Func *funcs[MAX_FUNC_SIZE];
Func *func_pos_ptr;

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
        node = try_node_func_call();

        if (NULL == node)
        {
            // 変数なので 一番近いブロック深度の中から合致する変数を探す なければエラー
            node = new_node_lvar(find_lvar());

            if (type_of_node(node)->kind == ARRAY)
            {
                // node = new_node_addr(node);
            }

            if (consume(TK_RESERVED, "["))
            {
                // node = new_node(ND_ADD, node, expr());
                node->lhs = new_node(ND_ADD, node->lhs, expr());

                node = new_node_deref(node);
                expect(TK_RESERVED, "]");
            }
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
    if (consume(TK_OPERATOR, "+"))
    {
        node = primary();
    }
    // : -a -> 0-a に展開
    else if (consume(TK_OPERATOR, "-"))
    {
        node = new_node(ND_SUB, new_node_num(0), primary());
    }
    else if (consume(TK_OPERATOR, "*"))
    {
        node = new_node_deref(unary());
    }
    else if (consume(TK_OPERATOR, "&"))
    {
        node = new_node_addr(unary());
    }
    // : sizeof
    else if (consume_keyword(TK_SIZEOF))
    {
        node = new_node_sizeof();
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
        if (consume(TK_OPERATOR, "*"))
        {
            node = new_node(ND_MUL, node, unary());
        }
        // ; /
        else if (consume(TK_OPERATOR, "/"))
        {
            node = new_node(ND_DIV, node, unary());
        }
        // : %
        else if (consume(TK_OPERATOR, "%"))
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
        if (consume(TK_OPERATOR, "+"))
        {
            node = new_node(ND_ADD, node, mul());
        }
        // : a-b
        else if (consume(TK_OPERATOR, "-"))
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

// 等号 等号否定で区切る
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

// 計算式を代入式 代入演算子で区切る -> 区切られた後は代入式等は出現しない
// -> equality(), ND_ASSIGN
Node *assign()
{
    // == ,!=, < 等はequality()内で優先的に処理されている
    Node *node = equality();

    if (consume(TK_RESERVED, "="))
    {
        node = new_node_assign(assign(), node);
    }

    else if (current_token_is(TK_ASSIGN_OPERATOR, NULL))
    {
        if (consume(TK_ASSIGN_OPERATOR, "+="))
        {
            node = new_node_assign(new_node(ND_ADD, node, assign()), node);
        }
        else if (consume(TK_ASSIGN_OPERATOR, "-="))
        {
            node = new_node_assign(new_node(ND_SUB, node, assign()), node);
        }
        else if (consume(TK_ASSIGN_OPERATOR, "*="))
        {
            node = new_node_assign(new_node(ND_MUL, node, assign()), node);
        }
        else if (consume(TK_ASSIGN_OPERATOR, "/="))
        {
            node = new_node_assign(new_node(ND_DIV, node, assign()), node);
        }
        else if (consume(TK_ASSIGN_OPERATOR, "%="))
        {
            node = new_node_assign(new_node(ND_DIV_REM, node, assign()), node);
        }

        else
        {
            error_at(tokens[token_pos]->str, "代入演算子ではありません\n");
        }
    }

    return node;
}

// 予約語のない純粋な計算式として解釈する 三項間演算子もここ？
// void と四則演算しない など計算結果が整数であることを保証したい
// -> assign()
Node *expr()
{
    Node *node = assign();
    return node;
}

// 予約語,{} 変数宣言の解釈を行う
// -> expr(), ND_RETURN, ND_ELSE, ND_FOR_WHILE, ND_DO, ND_CONTINUE, ND_BREAK, ND_BLOCK
// 非終端node のType 定義(暗黙的なキャスト) はここで行う -> これ以下のnode はすべて左右辺でのみ結合される
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
        if (current_token_is(TK_TYPE, NULL))
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
        // : break
        else if (consume_keyword(TK_BREAK))
        {
            node = create_node(ND_BREAK);
        }
        // : expr()
        else
        {
            node = expr();
        }

        expect(TK_RESERVED, ";");
    }

    node->type = type_of_node(node);
    return node;
}

// code全体を ; で区切る
// -> ND_BLOCK
Node *program()
{
    expect(TK_BLOCK_FRONT, "{");
    return build_block();
}

// 関数の宣言か定義のみを扱う
void *parse()
{
    while (!at_eof())
    {
        if (current_token_is(TK_TYPE, NULL))
        {
            // 関数判定はident の次の() の有無のみを参照する
            if (look_ahead_function())
            {
                new_function();
            }

            else
            {
                // グローバル変数の解釈は未実装
            }
        }

        else
        {
            error("型宣言がありません -> 後に対応");
        }
    }
}