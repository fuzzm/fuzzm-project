; zenlisp example program
; By Nils M Holm, 2007
; See the file LICENSE for conditions of use.

; R6RS Scheme-style FOLD-LEFT. This function is like FOLD,
; but accepts multiple list arguments:
; (fold-left list '0 '(a b c) '(d e f)) => '((#0ad b e) c f)

(define (fold-left f b . a*)
  (letrec
    ((car-of
       (lambda (a)
         (map car a)))
     (cdr-of
       (lambda (a)
         (map cdr a)))
     (fold
       (lambda (a* r)
         (cond ((null (car a*)) r)
               (t (fold (cdr-of a*)
                        (apply f r (car-of a*))))))))
    (cond ((null a*) (bottom 'too-few-arguments))
          (t (fold a* b)))))
