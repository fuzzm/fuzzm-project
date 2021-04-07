; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Extract the tail of a list (where the first member
; of the tail has a given property) or :f.
; (some null '(a b () c d)) => '(() c d)

(define (some p x)
  (cond ((null x) :f)
        ((p (car x)) x)
        (t (some p (cdr x)))))
