#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 入力プログラム
char *user_input;
// 関数名の一時保存先
static char *funcname;

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
