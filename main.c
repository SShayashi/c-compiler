#include "9cc.h"

// 入力プログラム
char *user_input;

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
    Function *pg = program();
    codegen(pg);
    return 0;
}
