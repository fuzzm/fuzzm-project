; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Extract members matching a given predicate:
; (filter atom '(abc #123 def #456 ghi)) => '(abc def ghi)

(define (filter p a)
  (letrec
    ((filter2
       (lambda (a b)
         (cond ((null a) b)
               ((p (car a))
                 (filter2 (cdr a) (cons (car a) b)))
               (t (filter2 (cdr a) b))))))
    (filter2 (reverse a) ())))

; Using PARTITION:
; (require 'partition)
; (define (filter p a)
;   (car (partition p a)))
