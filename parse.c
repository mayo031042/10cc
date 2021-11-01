#include "parse.h"
// RDP 関数のみ

#define MAX_FUNC_SIZE 100

Func *funcs[MAX_FUNC_SIZE];
int func_pos = 0;
int block_nest = 0;

// programの中の最小単位 (expr)か数値か変数、関数呼び出し しかありえない
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
    else if (consume_keyword(TK_IDENT))
    {
        // 関数かチェック
        if (consume(TK_RESERVED, "("))
        {
            node = create_node(ND_FUNC_CALL);
            node->func_num = find_func(false);

            // 引数
            expect(TK_RESERVED, ")");
        }
        else
        {
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
Node *unary()
{
    Node *node;

    // : +a
    if (consume(TK_RESERVED, "+"))
    {
        node = primary();
    }
    // : -a
    else if (consume(TK_RESERVED, "-"))
    {
        node = new_node(ND_SUB, new_node_num(0), primary());
    }
    else
    {
        node = primary();
    }

    return node;
}

// 乗除算で区切る
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
        // : break
        else
        {
            break;
        }
    }

    return node;
}

// 加減算で区切る
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
        // : break
        else
        {
            break;
        }
    }

    return node;
}

// 不等号で区切る
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
        // : break
        else
        {
            break;
        }
    }

    return node;
}

// 等号　等号否定で区切る　
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
        // : break
        else
        {
            break;
        }
    }

    return node;
}

// 計算式を代入式　代入演算子で区切る -> 区切られた後は代入式等は出現しない
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
        else
        {
            error_at(tokens[token_pos]->str, "代入演算子ではありません\n");
        }
    }

    return node;
}

// 予約語のない純粋な計算式として解釈する 三項間演算子もここ？
Node *expr()
{
    return assign();
}

// 予約語,{} の解釈を行う
// if else, for, while, do while,
// continue, break,
// return,
// block, expr()
Node *stmt()
{
    Node *node;

    // : return,
    if (consume_keyword(TK_RETURN))
    {
        node = new_node(ND_RETURN, expr(), NULL);
        expect(TK_RESERVED, ";");
    }
    // : if
    else if (consume_keyword(TK_IF))
    {
        node = new_node_else();
    }
    // : for
    else if (consume_keyword(TK_FOR))
    {
        Node *nodes[3];
        expect(TK_RESERVED, "(");

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

        Node *node_right = new_node(ND_NOP, stmt(), nodes[1]);
        Node *node_left = new_node(ND_NOP, nodes[0], nodes[2]);
        node = new_node(ND_FOR_WHILE, node_left, node_right);
    }
    // : while
    else if (consume_keyword(TK_WHILE))
    {
        expect(TK_RESERVED, "(");
        Node *node_B = expr();
        expect(TK_RESERVED, ")");
        Node *node_right = new_node(ND_NOP, stmt(), node_B);
        Node *node_left = new_node(ND_NOP, create_node(ND_NOP), create_node(ND_NOP));
        node = new_node(ND_FOR_WHILE, node_left, node_right);
    }
    // : do{} while();
    else if (consume_keyword(TK_DO))
    {
        Node *lhs = stmt();
        expect(TK_WHILE, "while");
        expect(TK_RESERVED, "(");
        Node *rhs = expr();
        expect(TK_RESERVED, ")");
        expect(TK_RESERVED, ";");
        node = new_node(ND_DO, lhs, rhs);
    }
    // : continue
    else if (consume_keyword(TK_CONTINUE))
    {
        node = create_node(ND_CONTINUE);
        expect(TK_RESERVED, ";");
    }
    // : break;
    else if (consume_keyword(TK_BREAK))
    {
        node = create_node(ND_BREAK);
        expect(TK_RESERVED, ";");
    }
    // : {}
    else if (consume_keyword(TK_BLOCK_FRONT))
    {
        node = build_block();
    }
    else
    {
        node = expr();
        expect(TK_RESERVED, ";");
    }

    return node;
}

// code全体を　;　で区切る
Node *program()
{
    expect(TK_BLOCK_FRONT, "{");
    return build_block();
}

// 関数の宣言か定義のみを扱う
Func *function()
{
    int i = 0;

    while (!at_eof())
    {
        if (!consume_keyword(TK_IDENT))
        {
            error_at(tokens[token_pos]->str, "関数名ではありません");
        }

        expect(TK_RESERVED, "(");

        func_pos = find_func(true);
        // 現在見ている関数のtokens[] での位置
        int loc_of_func_pos = val_of_ident_pos();

        // 引数の処理
        expect(TK_RESERVED, ")");

        if (func_pos == -1)
        {
            // はじめて現れた関数である のでset してからインクリメント
            funcs[i] = new_func(tokens[loc_of_func_pos]);
            funcs[i + 1] = NULL;
            func_pos = i;
            i++;
        }

        // 宣言のみか　定義されるか
        if (!consume(TK_RESERVED, ";"))
        {
            // 定義部分の｛｝が来ていると予想される
            if (funcs[func_pos]->defined == true)
            {
                error_at(tokens[loc_of_func_pos]->str, "関数が複数回定義されています");
            }

            // program()　を呼び出す
            funcs[func_pos]->defined = true;
            funcs[func_pos]->definition = program();
        }
        // 宣言のみなら次の関数の読み込みに移る
    }
}