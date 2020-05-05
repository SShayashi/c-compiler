#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

Node *code[100];
LVar *locals = NULL;

LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
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

// program = stmt*
void program()
{
    int i = 0;
    while (!at_eof())
        code[i++] = stmt();
    code[i] = NULL;
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
    }
    else if (consume_token(TK_IF))
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
    else if (consume_token(TK_FOR))
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
    else if (consume_token(TK_WHILE))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->lhs = expr();
        expect(")");
        node->rhs = stmt();
        return node;
    }
    else if (consume("{"))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        Node *p = node;
        while (!consume("}"))
        {
            p->next = stmt();
            p = p->next;
        }
        p = NULL;
        return node;
    }
    else
    {
        node = expr();
    }
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

    Token *tok = consume_indent();
    if (tok && consume("()"))
    {
        // 関数
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_FUNC;

        // TODO 名前を保存
        // TODO 二重定義かどうかを調べることも必要
        return node;
    }
    else if (tok)
    {
        // 変数
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar)
        {
            node->offset = lvar->offset;
        }
        else
        {

            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            if (locals)
                lvar->offset = locals->offset + 8;
            else
                lvar->offset = 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }

    return new_node_num(expect_number());
}