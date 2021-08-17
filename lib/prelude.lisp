(define (caar x) (car (car x)))
(define (cadr x) (car (cdr x)))
(define (cdar x) (cdr (car x)))
(define (cddr x) (cdr (cdr x)))

(define string<=? <=)
(define string<? <)
(define string>=? >=)
(define string>? >)
(define string=? =)

(define (newline) (display "\n"))

;; List functions.
(define (list? li)
  (if (pair? li)
      (list? (cdr li))
    (null? li)))

(define (list-tail li n)
  (cond
   ((<= n 0) li)
   ((pair? (cdr li)) (list-tail (cdr li) (- n 1)))
   ((null? (cdr li)) li)
   (else li)))

(define (list-ref li n)
  (car (list-tail li n)))

(define (list-set! li n v)
  (set-car! (list-tail li n) v))


(define (map proc ls . lol)
  (define (map1 proc ls res)
    (if (pair? ls)
        (map1 proc (cdr ls) (cons (proc (car ls)) res))
      (reverse res)))
  (define (mapn proc lol res)
    (if (every pair? lol)
        (mapn proc
              (map1 cdr lol '())
              (cons (apply proc (map1 car lol '())) res))
      (reverse res)))
  (if (null? lol)
      (map1 proc ls '())
    (mapn proc (cons ls lol) '())))

(define (for-each f ls . lol)
  (define (for1 f ls) (if (pair? ls) (begin (f (car ls)) (for1 f (cdr ls)))))
  (if (null? lol) (for1 f ls) (begin (apply map f ls lol) (if #f #f))))

(define (any pred ls . lol)
  (define (any1 pred ls)
    (if (pair? (cdr ls))
        ((lambda (x) (if x x (any1 pred (cdr ls)))) (pred (car ls)))
        (pred (car ls))))
  (define (anyn pred lol)
    (if (every pair? lol)
        ((lambda (x) (if x x (anyn pred (map cdr lol))))
         (apply pred (map car lol)))
        #f))
  (if (null? lol) (if (pair? ls) (any1 pred ls) #f) (anyn pred (cons ls lol))))

(define (every pred ls . lol)
  (define (every1 pred ls)
    (if (null? (cdr ls))
        (pred (car ls))
        (if (pred (car ls)) (every1 pred (cdr ls)) #f)))
  (if (null? lol)
      (if (pair? ls) (every1 pred ls) #t)
    (not (apply any (lambda xs (not (apply pred xs))) ls lol))))

