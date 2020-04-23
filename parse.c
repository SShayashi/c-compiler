#include <stdio.h>
#include <stdlib.h>
#include "9cc.h"

Node *code[100];

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

// program = stmt*
void program()
{
    int i = 0;
    while (!at_eof())
        code[i++] = stmt();
    code[i] = NULL;
}

// stmt = expr ";"
Node *stmt()
{
    Node *node = expr();
    expect(";");
    return node;
}

// expr = equality
Node *expr()
{
    Node *node = assign();
    return node;
}

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

// primary = num | indent |  "(" expr ")"
Node *primary()
{
    // 次のトークンが"("なら，"(" expr ")" のはず
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_indent();
    if (tok)
    {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8; // 変数はa~zのみなのでa~の差分をとっている．
        return node;
    }

    return new_node_num(expect_number());
}