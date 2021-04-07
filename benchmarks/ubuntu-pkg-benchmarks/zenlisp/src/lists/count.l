; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Recursively count the atoms of a list:
; (count '(a (b (c)))) => '#3

(require '~nmath)

(define (count x)
  (cond ((null x) '#0)
        ((atom x) '#1)
        (t (+ (count (car x))
              (count (cdr x))))))
