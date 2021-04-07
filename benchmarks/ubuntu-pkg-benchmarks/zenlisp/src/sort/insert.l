; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Insert a member into an ordered list:
; (require '~nmath)
; (insert < '#4 '(#3 #5)) => '(#3 #4 #5)

(define (insert p x a)
  (letrec
    ((ins
       (lambda (a r)
         (cond ((or (null a) (p x (car a)))
                 (append (reverse (cons x r)) a))
               (t (ins (cdr a) (cons (car a) r)))))))
    (ins a ())))
