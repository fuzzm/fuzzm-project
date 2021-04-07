; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Sort a list using the Mergesort algorithm:
; (require '~nmath)
; (mergesort <= '(#5 #1 #3 #2 #4)) => '(#1 #2 #3 #4 #5)

(define (mergesort p a)
  (letrec
    ((split
       (lambda (a r1 r2)
         (cond ((or (null a)
                    (null (cdr a)))
                 (list (reverse r2) r1))
               (t (split (cddr a)
                         (cdr r1)
                         (cons (car r1) r2))))))
     (merge
       (lambda (a b r)
         (cond
           ((null a)
             (cond ((null b) r)
                   (t (merge a (cdr b) (cons (car b) r)))))
           ((null b)
             (merge (cdr a) b (cons (car a) r)))
           ((p (car a) (car b))
             (merge a (cdr b) (cons (car b) r)))
           (t (merge (cdr a) b (cons (car a) r))))))
     (sort
       (lambda (a)
         (cond ((or (null a)
                    (null (cdr a)))
                 a)
               (t (let ((p* (split a a ())))
                    (merge (reverse (sort (car p*)))
                           (reverse (sort (cadr p*)))
                           ())))))))
    (sort a)))
