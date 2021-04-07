; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Extract the last member of a list:
; (last '(a b c d e f)) => 'f

(define (last x) (car (reverse x)))
