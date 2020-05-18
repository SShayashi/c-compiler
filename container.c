#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op)
{
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

bool consume_token(TokenKind kind)
{
    if (token->kind != kind)
        return false;
    token = token->next;
    return true;
}

// 次のトークンがreturnの場合，トークンを一つ進めて
// trueを返す，それ以外の場合falseを返す．
bool consume_return()
{
    if (token->kind != TK_RETURN)
        return false;
    token = token->next;
    return true;
}

// TK_IDENTから１つつtokenを進め読み込んだ識別子を文字列で返す
char *expect_ident(void)
{
    if (token->kind != TK_IDENT)
        error_at(token->str, "識別子ではありません");
    char *s = strndup(token->str, token->len);
    token = token->next;
    return s;
}
// 次のトークンが変数もしくは関数名の場合，トークンを1つ詠み進めて
// 変数か関数名のトークンを返す，それ以外はNullを返す
Token *consume_ident()
{
    if (token->kind != TK_IDENT)
        return NULL;
    Token *intent = token;
    token = token->next;
    return intent;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op)
{
    if (token->kind != TK_RESERVED)
    {
        error("len:%d, kind:%d, str:%s, nextstr: %s", token->len, token->kind, token->str, token->next->str);
        error_at(token->str, "'%s'ではありません，トークンの型が違います, トークンの型は%dです", op, token->kind);
    }

    if (strlen(op) != token->len)
        error_at(token->str, "'%s'ではありません，トークンの長さが違います，トークンの長さは%dです", op, token->len);
    if (memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません，memcmpの結果が違います", op);
    token = token->next;
}

// 次のトークンが数値の場合，トークンを1つ読み進めてその数値を返す．
// それ以外の場合にはエラーを報告する．
int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "数ではありません, kind:%d, str:%s", token->kind, token->str);
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}