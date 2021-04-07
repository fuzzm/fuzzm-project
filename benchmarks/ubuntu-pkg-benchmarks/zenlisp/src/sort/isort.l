; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Sort a list using insertion sort:
; (require '~nmath)
; (isort > '(#1 #3 #0 #2 #4)) => '(#4 #3 #2 #1 #0)

(require 'insert)

(define (isort p a)
  (Letrec
    ((sort
       (lambda (a r)
         (cond ((null a) r)
               (t (sort (cdr a)
                        (insert p (car a) r)))))))
    (sort a ())))
