; zenlisp example program
; By Nils M Holm, 2007
; See the file LICENSE for conditions of use.

; Create the number-theoretic partitions of an integer N:
; (make-partitions '#3) => '((#3) (#1 #2) (#1 #1 #1))

(require '~nmath)
(require 'iota)

(define (make-partitions n)
  (letrec
    ((partition
       (lambda (n)
         (cond
           ((zero n) '(()))
           ((one n) '((#1)))
           (t (apply append
                     (map (lambda (x)
                            (map (lambda (p) (cons x p))
                                 (partition (- n x))))
                          (iota '#1 n)))))))
     (filter-descending
       (lambda (p)
         (cond ((null (cdr p)) p)
               ((apply >= (car p))
                 (cons (car p) (filter-descending (cdr p))))
               (t (filter-descending (cdr p)))))))
    (reverse (filter-descending (partition n)))))

