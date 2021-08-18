(define test-count 0)
(define test-err-count 0)
(defmacro expect-with (eq must expr msg)
  `(let ((v ,expr))
     (set! test-count (+ test-count 1))
     (if (,eq v ,must)
	 (display ".")
       (set! test-err-count (+ test-err-count 1))
       (display "\nFAIL: " ,@msg ". expect " ,must " but " v ", expr = " ',expr "\n"))))

(defmacro expect (must expr . msg)
  `(expect-with equal? ,must ,expr ,msg))


(define (test-finish)
  (display "\n" test-count " tests finished.\n")
  (if (> test-err-count 0)
      (display test-err-count " tests failed.\n")))

