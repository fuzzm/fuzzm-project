; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Check whether the members of a list
; are in a given monotonic order:
; (require '~nmath)
; (orderedp > '(#9 #8 #7 #6 #5)) => 't

(define (orderedp p x)
  (letrec
    ((orderedp1
       (lambda (x)
         (cond ((or (null x) (null (cdr x))) :t)
               (t (and (p (car x) (cadr x))
                       (orderedp1 (cdr x))))))))
    (orderedp1 x)))
