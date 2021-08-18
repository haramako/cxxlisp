(load "lib/test.lisp")

(expect 6 (+ 1 2 3))
(expect 0 (- 3 2 1))
(expect "1" (number->string 1))
(expect -1 (string->number "-1"))

;; Test string functions.
(expect 'hoge (string->symbol "hoge"))
(expect "hoge" (symbol->string 'hoge))
(expect '(97 98 99) (string->list "abc"))
(expect "123" (string-append "1" "2" "3"))
(expect "bc" (substring "abcd" 1 3))

;; Test list functions.
(expect "ABC" (list->string '(65 66 67)))
(expect #t (list? '(1 2)))
(expect #f (list? '(1 . 2)))
(expect '(2) (list-tail '(0 1 2) 2))
(expect 3 (let ((li '(0 1 2))) (list-set! li 1 3) (list-ref li 1)))

;; Test macro-expand.
(defmacro x (a) `(puts ,a))
(expect '(puts 1) (macroexpand '(x 1)))
(expect '(puts (puts 1)) (macroexpand '(x (x 1))))
(expect '(puts (x 1)) (macroexpand-1 '(x (x 1))))

(test-finish)
