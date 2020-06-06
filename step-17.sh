#!/bin/bash

cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }
int add6(int a, int b, int c, int d, int e, int f) {
 return a+b+c+d+e+f;
}
EOF

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s tmp2.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
   echo "$input => $actual"
  else
   echo "$input => $expected expected, but got $actual"
   exit 1
  fi
}


#tmp int型の導入
assert 3 'int main() { int x; x = 3; return x;}'

# &, *の演算子が利用できる
assert 3 'int main() { int x; x = 3; int y; y = 5; int z; z = &y - 8; return *z;}'
assert 3 'int main() { int x; x = 3; int y; y = &x; return *y;}'

# 関数を定義して呼び出せる
assert 55 'int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); } '
assert 9 'int main() { int x; x=9; return fib(x); } int fib(int x) {return x;}'
assert 18 'int main() { return fib(9); } int fib(int x) {return add(x,x); }'
assert 32 'int main() { return ret32(); } int ret32() { return 32; }'
assert 7 'int main() { return add2(3,4); } int add2(int x, int y) { return x+y; }'
assert 1 'int main() { return sub2(4,3); } int sub2(int x, int y) { return x-y; }'

# 外部のオブジェクトファイルが持つ関数を呼び出せる
assert 3 'int main() { return ret3(); }'
assert 5 'int main() { return ret5(); }'
assert 8 'int main() { return add(3, 5); }'
assert 2 'int main() { return sub(5, 3); }'
assert 21 'int main() { return add6(1,2,3,4,5,6); }'

# 引数なしの関数が定義できて実行できる
assert 1 'int foo() { return 1; } int main(){ int a; a = foo(); return a;}'


# block機能
## blockで囲んだif文が書ける
assert 1 'int main(){ int a; a = 1; if(a) { return 1; } else { return 0; } }'
assert 3 'int main() { {1; {2;} return 3;} }'
assert 10 'int main() {int i; i=0; while(i<10) i=i+1; return i; }'

## blockで囲むだけのことができる
assert 99 'int main(){int a; { a = 99;} return a; }'
## blockで囲んだfor文が書ける
assert 3 'int main(){int a; int b; int i;  a = 0; b = 0; for(i = 0; i<3; i=i+1) { a = a + i*i; b = b + i; } return b;}'

## blockで囲んでwhile文が書ける
assert 100 'int main(){ a = 0; while(a<100){ for(i = 0;i<10; i=i+1){ a = a+1;}} return a;}'

# while文が利用できる
assert 10 'int main(){ a = 1; while(a<10)  a = a+1; return a;}'
## while文の中でif文が利用できる
assert 3 'int main(){ a = 3; while(a<10) if(a) return a; return -1;}'


# for 文が利用できる
assert 50 ' int main(){ a = 0; for(i=0;i<50;i=i+1) a = a+1; return a;}'
## for文の条件部分がオプショナルになっている
assert 1 'int main(){ for(;;) return 1;}'
assert 51 'int main(){ for(a = 0;a<51;) a = a+1; return a;}'
assert 2 'int main(){ for(a = 2;; a=a+2) return a;}'

## for文の条件部分がオプショナルになっており，その中でif文が利用できる
assert 10 'int main(){ a = 10; for(;;a = a+10) return a;}'
assert 6 'int main(){ a = 1; for(;;a = a+1) if(a>5) return a;}'


# 2回以上，別のstmtでif文が使える
assert 31 'int main(){ int a; int b; a = 10; if(1) b = 20; if(1) a = 11; return a+b;}'
assert 18 'int main(){  int a; int b;  a = 30; if(0) a = a+2; else a = a-2; if(0)  a = a+10; else  a = a-10; return a;}'

# if else 文が利用できる
assert 3 'int main(){if (1 > 2) return 2;else return 3; }'
assert 2 'int main(){if (1 < 2) return 2;else return 3;}'

# if文が利用できる
assert 1 'int main(){if (1 < 2) return 1;return 3;}'
assert 3 'int main(){if (1 > 2) return 1;return 3;}'

# returnが利用できる．returnは関数内のそれ以下を処理しない
assert 1 'int main(){int hoge; int fuga; return 1;hoge = 2;fuga = 3; }'

# 一文字の変数が利用できる
assert 2 'int main(){int a; int b; a = 1;  b = 1;  return a+b;}'
assert 14 'int main(){int a; int b;  a = 3;  b = 5 * 6 - 8;  return a+b / 2;}'

# >と>=が区別できる
assert 1 'int main(){ return 1>0;}'
assert 0 'int main(){ return 1>1;}'
assert 0 'int main(){ return 1>2;}'
assert 1 'int main(){ return 1>=0;}'
assert 1 'int main(){ return 1>=1;}'
assert 0 'int main(){ return 1>=2;}'


# <と<=が区別できる
assert 1 'int main(){ return 0<1;}'
assert 0 'int main(){ return 1<1;}'
assert 0 'int main(){ return 2<1;}'
assert 1 'int main(){ return 0<=1;}'
assert 1 'int main(){ return 1<=1;}'
assert 0 'int main(){ return 2<=1;}'

# 等価，不等号の演算子が利用できる
assert 0 'int main(){ return 0==1;}'
assert 1 'int main(){ return 42==42;}'
assert 1 'int main(){ return 0!=1;}'
assert 0 'int main(){ return 42!=42;}'

# 基礎的な四則演算ができる
assert 0 'int main(){ return 0;}'
assert 42 'int main(){ return 42;}'
assert 21 'int main(){ return 5+20-4;}'
assert 41 'int main(){ return 12 + 34 - 5 ;}'
assert 47 'int main(){ 5+6*7;}'
assert 15 'int main(){ 5*(9-6);}'
assert 4 'int main(){ (3+5)/2;}'
assert 10 'int main(){ -10+20;}'

echo OK