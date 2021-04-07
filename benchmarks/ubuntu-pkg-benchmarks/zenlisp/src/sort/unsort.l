; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Generate some entropy by un-sorting a list of
; natural numbers:
; (require '~iota)
; (unsort (iota '#1 '#10) '#4) => '(#1 #3 #10 #2 #6 #9 #8 #4 #7 #5)

(require '~nmath)
(require 'nth)

(define (unsort a seed)
  (letrec
    ((remove-nth
       (lambda (a n r)
         (cond ((zero n) 
                 (cond ((null a) (reverse r))
                       (t (append (cdr a) (reverse r)))))
               (t (remove-nth (cdr a)
                              (- n '#1)
                              (cons (car a) r))))))
     (unsort4
       (lambda (a n k r)
         (cond ((zero k) (cons (car a) r))
               (t (unsort4 (remove-nth a n ())
                           (remainder (car a) k)
                           (- k '#1)
                           (cons (car (nth n a)) r)))))))
    (unsort4 a seed (- (length a) '#1) ())))
