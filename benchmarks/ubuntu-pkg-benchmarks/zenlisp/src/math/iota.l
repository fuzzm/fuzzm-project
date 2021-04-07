; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Generate a sequence of integer numbers:
; (iota '#5 '#10) => '(#5 #6 #7 #8 #9 #10)

(require '~imath)

(define (iota lo hi)
  (letrec
    ((j (lambda (x r)
          (cond ((< x lo) r)
                (t (j (- x '#1) (cons x r)))))))
    (j (integer hi) ())))
