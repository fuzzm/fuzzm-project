; zenlisp example program
; By Nils M Holm, 2007
; See the file LICENSE for conditions of use.

; Partition a list according to some predicate:
; (partition atom '(#1 alpha #2 beta)) => '((alpha beta) (#1 #2))

(define (partition p a)
  (letrec
    ((partition3
       (lambda (a r+ r-)
         (cond ((null a)
                 (list r+ r-))
               ((p (car a))
                 (partition3 (cdr a)
                             (cons (car a) r+)
                             r-))
               (t (partition3 (cdr a)
                              r+
                              (cons (car a) r-)))))))
    (partition3 (reverse a) () ())))
