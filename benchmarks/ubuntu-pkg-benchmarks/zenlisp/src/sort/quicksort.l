; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Sort a list using the Quicksort algorithm:
; (require '~nmath)
; (quicksort <= '(#5 #1 #3 #2 #4)) => '(#1 #2 #3 #4 #5)

(require 'partition)

(define (quicksort p a)
  (letrec
    ((sort
       (lambda (a)
         (cond ((or (null a) (null (cdr a))) a)
               (t (let ((p* (partition (lambda (x) (p (car a) x))
                                       (cdr a))))
                    (append (sort (cadr p*))
                            (list (car a))
                            (sort (car p*)))))))))
    (sort a)))
