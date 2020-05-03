#include <stdio.h>
#include "9cc.h"

int label_num = 0;

void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset); // 引くってことは下方向にスタックを積むということ
    printf("  push rax\n");
}

void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    case ND_IF:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        int cur_if_num = label_num++;
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
    case ND_FOR:
        // for(A;B;C) D
        if (node->lhs)
            gen(node->lhs); // A
        int cur_for_num = label_num++;
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
    case ND_RETURN:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

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
