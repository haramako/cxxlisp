# cxxlisp

# TODO

- lexer/parser
  - ; コメント
  - ""
  - #f
  - #t
  - #;
  x #! shebang
  x #x
  * #\, #\space, #\newline, #\return, #\null
  x #""
  - ' = quote
  - ` = quasi-quote
  - , = unquote
  x リードマクロ

- executer
  - 
  
* マクロ展開
* デバッグ機能
  * pretty-print(文字数制限付き)
  * スタックトレース

# 基本方針

- あくまでC++の勉強用なので、あまり凝らない！
- Goのように、基本型だけでだいたいのことができるようにして、オブジェクト指向や関数型に凝らない

- 語彙はschemeのものを利用する
- 一部の関数は、ジェネリック対応とする(number, stringなど)
- 多値, 継続, ハイジェニックマクロ(define-syntax)は対応しない
- マクロは、伝統的マクロ(define-macro)で対応する
- stringは、immutable


# 型

nil
number(int,real)
bool
atom
?char

string
pair
custom object
?procedure
?vector(slice)
?dict
?tuple

# basic form

- define
* set!
* let

- lambda

- if
* cond(else,=>)

* define-macro
* macro-expand
* macro-expand1

# 関数


# 関数はジェネリックが基本

- deep-comparable(pair, vector)
 - equal

- equatable(number,string,atom,bool,char)
 - eq(=)

- comparable(number,string,char)
 - <,>,<=,>=

- addable(number,string)
 - +

- number
 - -,/,*

- enumerable(pair, vector, dict, ?tuple, string)
 - car(first), cdr(rest), nth(pair,vectorのみ?), length
 - map, fold, foldr

- associatable(dict, pair)
 - assoc, assoc-set!

- hashable(number,string,atom,bool,char,?tuple)
 - hash
 

# その他

- define は (define name value) or (define (func arg ...) body ...) の両方対応
- define-macro も同様

- ?vectorは、i8,i16,i32,cahr,valueなど、型がある

- ARCのhashの(hashtable key)はどうだろうか？
- JSONフレンドリー
- モジュールシステム
- 簡易lambdaの@1とか
  - (lambda (a b) (+ a b))
  - (^ (a b) (+ a b))
  - (^ (+ @1 @2))
  - {+ @1 @2}

- httpサーバーを組み込む
