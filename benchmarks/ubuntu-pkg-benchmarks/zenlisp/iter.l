; zenlisp iterators
; By Nils M Holm, 2007, 2008
; Feel free to copy, share, and modify this code.
; See the file LICENSE for details.

(define iter :t)

(define (arithmetic-iterator conv fn neutral)
  (lambda x
    (cond ((null x) neutral)
          (t (fold (lambda (a b)
                     (fn (conv a) (conv b)))
                   (car x)
                   (cdr x))))))

(define (predicate-iterator conv fn)
  (let ((:fail (cons ':fail ())))
    (let ((comp (lambda (a b)
                  (cond ((eq a :fail) :fail)
                        ((fn (conv a) (conv b)) b)
                        (t :fail)))))
      (lambda (first . rest)
        (cond ((null rest) (bottom '(too few arguments)))
              (t (neq (fold comp first rest) :fail)))))))
