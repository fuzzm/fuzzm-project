; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Check if an expression X is equal to the tail
; of an expression Y:
; (tailp '(d e f) '(a b c d e f)) => 't

(require 'headp)

(define (tailp x y)
  (headp (reverse x) (reverse y)))

; Without using HEADP:
; (define (tailp x y)
;   (cond ((null y) (null x))
;         (t (or (equal x y)
;                (tailp x (cdr y))))))
