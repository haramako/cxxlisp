; test loop
(define n 0)
(loop
 (if (> n 10)
     (break n)
   (display n " ") (set! n (+ n 1))))

(newline)

; test quasi-quote
(defmacro hoge (x) `(puts ,x))

(hoge 1)

; test for
(defmacro dotimes (num-expr . body)
  `(begin
    (define ,(car num-expr) 0)
    (loop
     (if (> ,(car num-expr) ,(cadr num-expr))
	 (break ,(car num-expr))
       ,@body
       (set! ,(car num-expr) (+ ,(car num-expr) 1))
       ))))


;; dotime
(dotimes (x 10) (display x " "))
(newline)

;; fib
(define (fib n)
  (if (< n 2)
      1
    (+ (fib (- n 1)) (fib (- n 2)))))

(puts "fib" (fib 10))

;; map
(puts (map (lambda (x) (+ x 1)) '(1 2 3)))



