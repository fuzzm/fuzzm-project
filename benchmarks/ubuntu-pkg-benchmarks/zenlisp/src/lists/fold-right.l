; zenlisp example program
; By Nils M Holm, 2007
; See the file LICENSE for conditions of use.

; R6RS Scheme-style FOLD-RIGHT. This function is like FOLD-R,
; but accepts multiple list arguments:
; (fold-right list '0 '(a b c) '(d e f)) => '(a d (b e #cf0))

(define (fold-right f b . a*)
  (letrec
    ((car-of
       (lambda (a)
         (map car a)))
     (cdr-of
       (lambda (a)
         (map cdr a)))
     (foldr
       (lambda (a* r)
         (cond ((null (car a*)) r)
               (t (foldr (cdr-of a*)
                         (apply f (append (car-of a*)
                                          (list r)))))))))
    (cond ((null a*) (bottom 'too-few-arguments))
          (t (foldr (map reverse a*) b)))))
