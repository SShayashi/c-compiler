#include <stdio.h>
#include "9cc.h"

//関数定義
static void gen(Node *node);

// 関数名の一時保存先
static char *funcname;
static int label_num = 0;
static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->var->offset); // 引くってことは下方向にスタックを積むということ
    printf("  push rax\n");
}

static void gen_args(Node *node)
{
    int i = 0;
    for (Node *p = node->args; p; p = p->next)
    {
        gen(p);
        printf("  pop %s\n", argreg[i++]);
    }
}

static void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("# == read num  == #\n");
        printf("  push %d\n", node->val);
        return;
    case ND_LVAR:
        printf("# == read local value == #\n");
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    case ND_ASSIGN:
        printf("# == assign local value== #\n");
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    case ND_IF:
    {
        int cur_if_num = label_num++;
        printf("# == if:%d == #\n", cur_if_num);
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if (node->rhs->kind == ND_ELSE)
        {
            printf("  je .Lelse%d\n", cur_if_num);
            gen(node->rhs->lhs);
            printf("  jmp .Lend%d\n", cur_if_num);
            printf(".Lelse%d:\n", cur_if_num);
            gen(node->rhs->rhs);
            printf(".Lend%d:\n", cur_if_num);
        }
        else
        {
            printf("  je .Lend%d\n", cur_if_num);
            gen(node->rhs);
            printf(".Lend%d:\n", cur_if_num);
        }
        return;
    }
    case ND_FOR:
    {
        int cur_for_num = label_num++;
        printf("# == for:%d == #\n", cur_for_num);
        // for(A;B;C) D
        if (node->lhs)
            gen(node->lhs); // A
        printf(".Lbegin%d:\n", cur_for_num);
        if (node->rhs->lhs)
        {
            gen(node->rhs->lhs); // B
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", cur_for_num);
        }
        gen(node->rhs->rhs->rhs); // D
        if (node->rhs->rhs->lhs)  // C
            gen(node->rhs->rhs->lhs);
        printf("  jmp .Lbegin%d\n", cur_for_num);
        printf(".Lend%d:\n", cur_for_num);
        return;
    }
    case ND_WHILE:
    {
        int cur_while_num = label_num++;
        printf("# == while:%d == #\n", cur_while_num);
        printf(".Lbegin%d:\n", cur_while_num);
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%d\n", cur_while_num);
        gen(node->rhs);
        printf("  jmp  .Lbegin%d\n", cur_while_num);
        printf(".Lend%d:\n", cur_while_num);
        return;
    }
    case ND_BLOCK:
    {
        for (Node *p = node->body; p; p = p->next)
        {
            gen(p);
            printf("  pop rax\n");
        }
        return;
    }
    case ND_FUN_CALL:
    {

        int label = label_num++;
        // 16bitアライメントを行う
        printf("# == stmt function call == #\n");
        printf("  mov rax, rsp\n");
        printf("  and rax, 15\n");
        printf("  jnz .L.call.%d\n", label); // 0以外だったらjamp
        gen_args(node);
        printf("  call %s\n", node->funcname);
        printf("  jmp .L.end.%d\n", label);
        printf(".L.call.%d:\n", label);
        printf("  sub rsp, 8\n");
        gen_args(node);
        printf("  call %s\n", node->funcname);
        printf(" add rsp, 8\n");
        printf(".L.end.%d:\n", label);
        printf("  push rax\n");
        return;
    }
    case ND_RETURN:
        printf("# == return == #\n");
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("# == calclation == #\n");
    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NEQ:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_BIG:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_EQBIG:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    }

    printf("  push rax\n");
}

static int align_to(int n, int align)
{
    return (n + align - 1) & ~(align - 1);
}
void codegen(Function *pg)
{
    // ローカル変数のoffsetを計算して，各関数ポインタに入れていく
    for (Function *fn = pg; fn; fn = fn->next)
    {
        int offset = 0;
        for (LVar *var = fn->locals; var; var = var->next)
        {
            offset += 8;
            var->offset = offset;
        }
        fn->stack_size = align_to(offset, 16);
    }

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");

    for (Function *fn = pg; fn; fn = fn->next)
    {
        printf(".global %s\n", fn->name);
        printf("%s:\n", fn->name);
        funcname = fn->name;

        // プロローグ
        printf("# == prologue %s == #\n", fn->name);
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", fn->stack_size);

        // 引数用のスタックを用意
        int i = 0;
        for (LVar *var = fn->args; var; var = var->next)
            i++;

        for (LVar *var = fn->args; var; var = var->next)
            printf("  mov [rbp-%d], %s\n", var->offset, argreg[--i]);
        // アセンブリコードの出力
        for (Node *node = fn->node; node; node = node->next)
            gen(node);

        // エピローグ
        printf("# == epilogue %s == #\n", fn->name);
        printf(".L.return.%s:\n", funcname);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
}