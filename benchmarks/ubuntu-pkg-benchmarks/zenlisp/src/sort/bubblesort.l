; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Sort a list using the bubblesort algorithm:
; (require '~nmath)
; (bubblesort < '(#5 #1 #7 #2 #6)) => '(#1 #2 #5 #6 #7)

(require 'orderedp)

(define (bubblesort p x)
  (letrec
    ((bubble-up
       (lambda (x)
         (cond ((or (null x) (null (cdr x)))
                 x)
               ((p (car x) (cadr x))
                 (cons (car x) (bubble-up (cdr x))))
               (t (cons (cadr x)
                        (bubble-up (cons (car x)
                                         (cddr x))))))))
     (bubble-step
       (lambda (x)
         (cond ((orderedp p x) x)
               (t (bubble-step (bubble-up x)))))))
    (bubble-step x)))
