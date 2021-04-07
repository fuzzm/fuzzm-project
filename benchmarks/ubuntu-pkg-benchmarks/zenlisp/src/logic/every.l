; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Check whether all members of a list have a given property:
; (every atom '(a b c)) => 't

(define (every p x)
  (cond ((null x) :t)
        ((null (cdr x)) (p (car x)))
        ((p (car x)) (every p (cdr x)))
        (t :f)))
