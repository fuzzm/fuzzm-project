; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; R6RS Scheme-style FOR-ALL. This function is like EVERY,
; but accepts multiple list arguments:
; (require '~nmath)
; (for-all < '(#1 #1 #1) '(#2 #2 #2)) => :t

(define (for-all p . a*)
  (letrec
    ((car-of
       (lambda (a)
         (map car a)))
     (cdr-of
       (lambda (a)
         (map cdr a)))
     (forall*
       (lambda (a*)
         (cond ((null (car a*)) :t)
               ((null (cdar a*))
                 (apply p (car-of a*)))
               (t (and (apply p (car-of a*))
                       (forall* (cdr-of a*))))))))
    (forall* a*)))
