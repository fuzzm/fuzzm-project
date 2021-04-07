; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Substitute variables in S-expressions.
; (substitute '(+ 1 2) '((1.i) (2.ii))) => '(+ i ii)

(define (substitute x env)
  (letrec
    ((value-of
       (lambda (x)
         (let ((v (assq x env)))
           (cond (v (cdr v))
                 (t x)))))
     (subst
       (lambda (x)
         (cond ((null x) ())
               ((atom x) (value-of x))
               (t (cons (subst (car x))
                        (subst (cdr x))))))))
    (subst x)))
