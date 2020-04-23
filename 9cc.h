#include <stdio.h>
#include <stdbool.h>

//
// Tokenizer
//

typedef enum
{
    TK_RESERVED, // 記号
    TK_INDENT,   // 識別子
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合，その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
};

//
// Parser
//
typedef enum
{
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_NUM,    // 整数
    ND_EQ,     // ==
    ND_NEQ,    // !=
    ND_EQBIG,  // >=
    ND_BIG,    // >
    ND_ASSIGN, // =
    ND_LVAR,   // ローカル変数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node
{
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // kindがND_NUMの場合のみ使う
    int offset;    // kindがND_LVARの場合のみ使う
};

// グローバル変数
// 現在着目しているトークン
extern Token *token;
// 入力プログラム
extern char *user_input;

// 関数群
extern void error(char *fmt, ...);
extern void error_at(char *loc, char *fmt, ...);

Token *consume_indent();
extern bool consume(char *op);
extern void expect(char *op);
extern int expect_number();
extern bool at_eof();
extern Token *new_token(TokenKind kind, Token *cur, char *str, int len);
extern Token *tokenize(char *p);

// parce.c
extern Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
extern Node *new_node_num(int val);
extern void program();
extern Node *stmt();
extern Node *expr();
extern Node *equality();
extern Node *relational();
extern Node *add();
extern Node *mul();
extern Node *unary();
extern Node *primary();
extern void gen(Node *node);