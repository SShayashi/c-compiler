#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
      echo "$input => $actual"
    else
      echo "$input => $expected expected, but got $actual"
      exit 1
    fi
}

# 基礎的な子息演算ができる
assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'

# 等価，不等号の演算子が利用できる
assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

# <と<=が区別できる
assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

# >と>=が区別できる
assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

# 一文字の変数が利用できる
assert 2 '
a = 1; 
b = 1; 
a+b;
'
assert 14 '
a = 3; 
b = 5 * 6 - 8; 
a+b / 2;
'


# 1文字以上の変数が利用できる
assert 2 'hoge=2;'

# 変数のために用意している領域は動的に確保できる
assert 10 '
hoge=3;
huga=4;
a=1;
b=1;
c=1;
d=1;
e=1;
f=1;
g=1;
h=1;
i=1;
j=1;
k=1;
l=1;
n=1;
m=1;
o=1;
p=1;
q=1;
r=1;
s=1;
t=1;
u=1;
v=1;
w=1;
x=1;
y=1;
z=1;
aa=1;
ab=1;
ac=1;
hoge+huga+3;
'

# returnが利用できる．returnは関数内のそれ以下を処理しない
assert 1 '
return 1;
hoge = 2;
fuga = 3;'

# if文が利用できる
assert 1 '
if (1 < 2)
  return 1;
return 3;
'
assert 3 '
if (1 > 2)
  return 1;
return 3;
'

# if else 文が利用できる
assert 3 '
if (1 > 2)
  return 2;
else
  return 3;
'
assert 2 '
if (1 < 2)
  return 2;
else
  return 3;
'

echo OK