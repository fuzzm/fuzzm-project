; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Build an association list from two lists:
; (pair '(1 2 3) '(i ii iii)) => '((1 . i) (2 . ii) (3 . iii))

(define (pair a b) (map cons a b))
