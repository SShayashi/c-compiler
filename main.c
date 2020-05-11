#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;
// 関数名の一時保存先
static char *funcname;

int is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p)
    {
        // 空白文字をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6]))
        {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2]))
        {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4]))
        {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3]))
        {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5]))
        {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        if (isalpha(*p))
        {
            char *q = p;
            cur = new_token(TK_INDENT, cur, p, 0);
            while (isalpha(*p))
                p++;
            cur->len = p - q;
            continue;
        }

        if (startswith(p, ">=") ||
            startswith(p, "<=") ||
            startswith(p, "==") ||
            startswith(p, "!="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>;={},", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
        error_at(token->str, "トークナイズできません");
    }
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません");
        return 1;
    }

    // トークナイズしてパースする
    user_input = argv[1];
    token = tokenize(user_input);
    Function *prog = program();

    // ローカル変数のoffsetを計算して，各関数ポインタに入れていく
    for (Function *fn = prog; fn; fn = fn->next)
    {
        int offset = 0;
        for (LVar *var = prog->locals; var; var = var->next)
        {
            offset += 8;
            var->offset = offset;
        }
        fn->stack_size = offset;
    }

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");

    //TODO ここで各関数のStacksize計算する
    for (Function *fn = prog; fn; fn = fn->next)
    {
        printf(".global main\n");
        printf("%s:\n", fn->name);
        funcname = fn->name;

        // プロローグ
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", fn->stack_size);

        // アセンブリコードの出力
        for (Node *node = fn->node; node; node = node->next)
            gen(node);

        // エピローグ
        printf(".L.return.%s:\n", funcname);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
    return 0;
}
