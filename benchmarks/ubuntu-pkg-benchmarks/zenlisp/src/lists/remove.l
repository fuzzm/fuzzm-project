; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Remove members from a list.
; (remove atom '(abc #123 def #456 ghi)) => '(#123 #456)

(define (remove p x)
  (filter (lambda (x) (not (p x)))
          x))
