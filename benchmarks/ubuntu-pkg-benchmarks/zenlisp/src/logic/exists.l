; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; R6RS Scheme-style EXISTS. This function is like ANY,
; but accepts multiple list arguments:
; (require '~nmath)
; (exists < '(#1 #2 #3) '(#1 #1 #4)) => :t

(define (exists p . a*)
  (letrec
    ((car-of
       (lambda (a)
         (map car a)))
     (cdr-of
       (lambda (a)
         (map cdr a)))
     (exists*
       (lambda (a*)
         (cond ((null (car a*)) :f)
               (t (or (apply p (car-of a*))
                      (exists* (cdr-of a*))))))))
    (exists* a*)))
