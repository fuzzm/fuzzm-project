; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Compute the union of some sets:
; (union '(aa ab) '(ab ac) '(ac ad) ()) => '(aa ab ac ad)

(require '~unique)

(define (union . a)
  (unique (apply append a)))
