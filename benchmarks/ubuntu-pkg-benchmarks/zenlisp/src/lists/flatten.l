; zenlisp example program
; By Nils M Holm, 2007
; See the file LICENSE for conditions of use.

; Flatten a tree:
; (flatten '((a) (b (c)) (d (e (f))))) => '#abcdef

(define (flatten x)
  (letrec
    ((f (lambda (x r)
          (cond ((null x) r)
                ((atom x) (cons x r))
                (t (f (car x)
                      (f (cdr x) r)))))))
    (f x ())))
