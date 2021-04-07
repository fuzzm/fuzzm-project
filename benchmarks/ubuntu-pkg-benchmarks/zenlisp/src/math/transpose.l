; zenlisp example program
; By Nils M Holm, 2007
; See the file LICENSE for conditions of use.

; Transpose (swap rows and columns of) a matrix:
; (transpose '(#abc #def)) => '(#ad #be #cf)

(define (transpose x)
  (apply map list x))
