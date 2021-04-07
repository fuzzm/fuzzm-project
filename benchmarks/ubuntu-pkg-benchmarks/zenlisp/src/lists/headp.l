; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Check whether an expression X is the head of an expression Y:
; (headp '(a b c) '(a b c d e f)) => 't

(define (headp x y)
  (cond ((null y) (null x))
        ((null x) :t)
        (t (and (equal (car x) (car y))
                (headp (cdr x) (cdr y))))))
