; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Return the first member having a given property or :f.
; (any atom '((a b c) x (d e f))) => 't

(define (any p x)
  (cond ((null x) :f)
        ((p (car x)) (car x))
        (t (any p (cdr x)))))
