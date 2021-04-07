; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Compute the depth of a list. The depth
; of a list is the maximum number of
; lists enclosing any atom of the list.
;
; (depth '(a b (c (d) e) ((f)))) => '#3

(require '~nmath)

(define (depth a)
  (cond ((atom a) '#0)
        (t (+ '#1 (apply max (map depth a))))))
