#include "9cc.h"

Node *code[100];
LVar *locals = NULL;
Function *functions = NULL;

LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

static LVar *new_lvar(char *name)
{
    LVar *var = calloc(1, sizeof(LVar));
    var->name = name;
    // var->ty = ty; 将来の型情報
    var->next = locals;

    var->len = strlen(name);
    locals = var;
    return var;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// program = function*
Function *program()
{
    Function head = {}; // これの意味知りたい
    Function *cur = &head;

    while (token->kind != TK_EOF)
        cur = cur->next = funcdef();
    return head.next;
}

// funcdef = ident"(" (num | num ("," num)*)?  ")" "{" stmt* "}"
// funcdef = ident"(" ")" "{" stmt* "}" ←まずはこちら
Function *funcdef()
{
    locals = NULL;

    // 関数の引数まで読む
    LVar lvar_head = {};
    LVar *lvar_cur = &lvar_head;
    char *name = expect_ident();
    expect("(");
    while (!consume(")"))
    {
        if (lvar_cur != &lvar_head)
            consume(",");
        char *ident = expect_ident();
        lvar_cur = new_lvar(ident);
    }

    Function *func = calloc(1, sizeof(Function));
    func->args = locals;

    // 関数のブロック部分を読む
    expect("{");
    Node node_head = {};
    Node *node_cur = &node_head;
    while (!consume("}"))
    {
        node_cur->next = stmt();
        node_cur = node_cur->next;
    }

    func->name = name;
    func->node = node_head.next;
    func->locals = locals;
    return func;
}
/* stmt = expr ";"
 *       | "{" stmt* "}"
 *       | "if" "(" expr ")" stmt ("else" stmt)?
 *       | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *       | "while" "(" expr ")" stmt
 *       | "return" expr ";"
 */
Node *stmt()
{
    Node *node;

    if (consume_return())
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
        return node;
    }

    if (consume_token(TK_IF))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->lhs = expr();
        expect(")");

        Node *truenode = stmt();
        if (consume_token(TK_ELSE))
            node->rhs = new_node(ND_ELSE, truenode, stmt());
        else
            node->rhs = truenode;

        return node;
    }

    if (consume_token(TK_FOR))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;

        Node *node_first = calloc(1, sizeof(Node));
        node_first->kind = ND_FOR;

        Node *node_second = calloc(1, sizeof(Node));
        node_second->kind = ND_FOR;

        node->rhs = node_first;
        node_first->rhs = node_second;

        // for (A;B;C) D
        expect("(");
        if (!consume(";"))
        {
            node->lhs = expr(); // A
            expect(";");
        }

        if (!consume(";"))
        {
            node_first->lhs = expr(); // B
            expect(";");
        }

        if (!consume(")"))
        {
            node_second->lhs = expr(); // C
            expect(")");
        }
        node_second->rhs = stmt(); // D
        return node;
    }

    if (consume_token(TK_WHILE))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->lhs = expr();
        expect(")");
        node->rhs = stmt();
        return node;
    }

    if (consume("{"))
    {
        Node head = {};
        Node *cur = &head;

        while (!consume("}"))
        {
            cur->next = stmt();
            cur = cur->next;
        }

        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->body = head.next;
        return node;
    }

    node = expr();
    expect(";");
    return node;
}

// expr = assign
Node *expr()
{
    Node *node = assign();
    return node;
}

// assign =  equality ( '=' equality )?
Node *assign()
{
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    else
        return node;
}

// equality = relaitonal ("==" relational | "!=" relational)*
Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NEQ, node, relational());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
    Node *node = add();
    for (;;)
    {
        if (consume("<="))
            node = new_node(ND_EQBIG, node, add());
        else if (consume("<"))
            node = new_node(ND_BIG, node, add());
        else if (consume(">="))
            node = new_node(ND_EQBIG, add(), node);
        else if (consume(">"))
            node = new_node(ND_BIG, add(), node);
        else
            return node;
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = primary ("*" primary | "/" primary)*
Node *mul()
{
    Node *node = unary();
    for (;;)
    {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary()
{
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

// funcall = ident "(" (assign ("," assign)*)? ")"
Node *funcall(Token *tk)
{
    Node head = {};
    Node *cur = &head;
    while (!consume(")"))
    {
        if (cur != &head)
            consume(",");
        cur->next = assign();
        cur = cur->next;
    }
    Node *node = calloc(1, sizeof(Node));
    node->funcname = strndup(tk->str, tk->len);
    node->kind = ND_FUN_CALL;
    node->args = head.next;
    return node;
}

// primary = num | indent ( "(" ")")? |  "(" expr ")"
Node *primary()
{
    // 次のトークンが"("なら，"(" expr ")" のはず
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    // 関数呼び出しの場合
    if (tok && consume("("))
        return funcall(tok);

    // 変数の場合
    if (tok)
    {
        // 変数
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar)
        {
            node->var = lvar;
        }
        else
        {
            char *name = strndup(tok->str, tok->len);
            node->var = new_lvar(name);
        }
        return node;
    }

    return new_node_num(expect_number());
}