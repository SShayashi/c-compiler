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
    TK_IF,       // if
    TK_ELSE,     // else
    TK_FOR,      // for
    TK_WHILE,    // while
    TK_RETURN,   // returnを表す
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

typedef struct LVar LVar;

// ローカル変数の型
struct LVar
{
    LVar *next; // 次の変数かNULL
    char *name; // 変数の名前
    int len;    // 名前の長さ
    int offset; // RBPからのオフセット
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
    ND_IF,     // if
    ND_ELSE,   // else
    ND_FOR,    // for
    ND_WHILE,  // while
    ND_RETURN  // return
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
// プログラムの各行が入った配列
extern Node *code[100];
// ローカル変数
extern LVar *locals;

// 関数群

// container.c
extern void error(char *fmt, ...);
extern void error_at(char *loc, char *fmt, ...);
extern bool consume_token(TokenKind kind);
extern bool consume_return();
extern Token *consume_indent();
extern bool consume(char *op);
extern void expect(char *op);
extern int expect_number();
extern bool at_eof();
extern Token *new_token(TokenKind kind, Token *cur, char *str, int len);
extern Token *tokenize(char *p);

// parce.c
extern LVar *find_lvar(Token *tok);
extern Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
extern Node *new_node_num(int val);
extern void program();
extern Node *stmt();
extern Node *expr();
extern Node *assign();
extern Node *equality();
extern Node *relational();
extern Node *add();
extern Node *mul();
extern Node *unary();
extern Node *primary();

// codegen.c
extern void gen(Node *node);