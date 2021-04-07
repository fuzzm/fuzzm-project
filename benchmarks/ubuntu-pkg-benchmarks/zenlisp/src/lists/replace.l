; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Replace OLD by NEW in EXPR:
; (replace 'x 'z '(a x (x) (x z))) => '(a z #z #zz)

(define (replace old new form)
  (cond ((equal form old) new)
        ((atom form) form)
        (t (cons (replace old new (car form))
                 (replace old new (cdr form))))))
