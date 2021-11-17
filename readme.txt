int などの型宣言はcurrent_token_is_type() で判定することにする

関数の依存関係を記述する
-> その関数内で呼び出される可能性のある自作関数を改行して羅列
-> 配列と構造体,enum も含めて記載


==parce_func.c==

bool current_token_is(TokenKind kind, char *op);
f
v
    tokens[]
    token_pos
    
bool current_token_is_type();
f
v
    TokenKind
    tokens[]
    token_pos

bool consume(TokenKind kind, char *op);
f
    current_token_is()
v
    token_pos

bool consume_keyword(TokenKind kind);
f
v
    tokens[]
    token_pos

bool consume_ident();
f
v
    tokens[]
    token_pos
    ident_pos

bool expect(TokenKind kind, char *op);
f
    consume()
    error_at()
v
    tokens[]
    token_pos

bool expect_ident();
f
    consume_ident()
    error_at()
v
    tokens[]
    token_pos

TypeKind expect_vartype();
f
    consume_keyword()
    error_at()
v
    tokens[]
    token_pos

int expect_number();
f
    error_at()
v
    tokens[]
    token_pos

bool at_eof();
f
v
    tokens[]
    token_pos

int val_of_ident_pos();
f
v
    ident_pos


==node.c==
Node 構造体は全てに出現するので省略

Node *create_node(NodeKind kind);
f
v

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
f
    create_node()
v

Node *new_node_num(int val);
f
    create_node()
v

Node *new_node_lvar(LVar *lvar);
f
    create_node()
v
    LVar

Node *create_or_stmt(NodeKind kind);
f
    consume()
    create_node()
    stmt()
v

Node *new_node_if();
f
    create_node()
    expr()
    expect()
    create_or_stmt()
v

Node *new_node_else();
f
    create_node()
    new_node_if()
    consume_keyword()
    new_node_else()
    create_or_stmt()
v

Node *new_grand_node(NodeKind kind, Node *l_l, Node *l_r, Node *r_l, Node *r_r)
f
    new_node()
v

Node *new_node_for();
f
    expect()
    current_token_is()
    create_node()
    expr()
    expect()
    new_grand_node()
    create_or_stmt()
v

Node *new_node_while();
f
    expect()
    expr()
    new_grand_node()
    create_node()
    create_or_stmt()
v

Node *new_node_do();
f
    create_or_stmt()
    expect()
    expr()
    new_node()
v

Node *new_node_sizeof();
f
    unary()
    new_node_num()
    size_of_node()
v

Node *new_node_block();
f
    consume()
    consume_keyword()
    stmt()
    type_of_node()
    new_node_block()
v

Node *build_block();
f
    add_block_nest()
    new_node()
    new_node_block()
    sub_block_nest()
v


==parce.c==
Node 構造体は全てに出現するので省略

Node *primary();
f
    consume()
    expr()
    expect()
    consume_ident()
    current_token_is()
    create_node()
    find_func()
    error_at()
    val_of_ident_pos()
    build_arg()
    new_node_lvar()
    find_lvar()
    new_node()
v
    Func

Node *unary();
f
    consume()
    primary()
    new_node()
    new_node_num()
    unary()
    new_node_sizeof()
v

Node *mul();
f
    consume()
    new_node()
    unary()
v

Node *add();
f
    mul()
    consume()
    new_node()
v

Node *relational();
f
    add()
    consume()
    new_node()
v

Node *equality();
f
    relational()
    consume()
    new_node()
v

Node *assign();
f
    equality()
    consume()
    new_node()
    assign()
    error_at()
v
    tokens[]
    token_pos

Node *expr();
f
    assign()
    type_of_node()
v

Node *stmt();
f
    consume_keyword()
    build_block()
    new_node_else()
    new_node_for()
    new_node_while()
    new_node_do()
    current_token_is_type()
    declare_lvar()
    new_node()
    expr()
    create_node()
    expect()
v

Node *program();
f
    expect()
    build_block()
v

void *function()
f
    at_eof()
    current_token_is_type()
    error()
    new_type()
    expect_vartype()
    add_type_ptr()
    expect_ident()
    find_func()
    new_func()
    val_of_ident_pos()
    declare_arg()
    consume_arg()
    error_at()
    program()
v
    Type
    Func
    funcs[]
    func_pos


==lvar.c==
LVar 構造体は全てに出現するので省略

int culc_offset();
f
    val_of_block_nest()
v
    funcs[]
    func_pos

LVar *new_lvar();
f
    val_of_ident_pos()
    culc_offset()
    size_of()
    val_of_block_nest()
v
    tokens[]
    funcs[]
    func_pos
    
LVar *find_lvar_within_block(int depth);
f
    val_of_ident_pos()
v
    funcs[]
    func_pos
    tokens[]

LVar *find_lvar();
f
    val_of_block_nest()
    find_lvar_within_block()
    error_at()
v
    tokens[]
    token_pos

Node *declare_lvar();
f
    create_node()
    new_type()
    add_type_ptr()
    expect_ident()
    add_type_array()
    find_lvar_within_block()
    val_of_block_nest()
    error_at()
    new_lvar()
v
    Node
    Type


==func.c==
Func 構造体は全てに出現するので省略

Func **find_func();
f
    val_of_ident_pos()
v
    funcs[]    

Func *new_func(Token *tok, Type *type);
f
v
    Token

int offset_arg(int func_pos);
f
v
    funcs[]
    func_pos

void declare_arg();
f
    expect()
    consume()
    declare_lvar()
v

void consume_arg();
f
    expect()
    consume()
v
    token_pos

Node *build_arg();
f
    expect()
    consume()
    new_node()
    expr()
v
    Node


==type.c==
Type 構造体は全てに出現するので省略

Type *new_type(TypeKind kind);
f
v

Type *add_type_ptr(Type *type);
f
    consume()
    new_type()
v

Type *add_type_array(Type *type);
f
    consume()
    new_type()
    expect_number()
    expect()
    add_type_array()
v
