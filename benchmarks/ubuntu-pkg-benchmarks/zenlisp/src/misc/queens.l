; zenlisp example program
; By Nils M Holm, 1998-2008
; See the file LICENSE for conditions of use.

; Solve the N-queens problem.
; (queens '#4)
; (queens '#5 '#2) ; print only two solutions

(require '~nmath)

(define (queens board-size . limit)
  (letrec
    ((column
       (lambda (x)
         (quotient x board-size)))
     (row
       (lambda (x)
         (remainder x board-size)))
     (incr
       (lambda (x)
         (+ '#1 x)))
     (decr
       (lambda (x)
         (- x '#1)))
     (can-attack-straight-p
       (lambda (x y)
         (or (= (row x) (row y))
             (= (column x) (column y)))))
     (abs-diff
       (lambda (x y)
         (cond ((< x y) (- y x))
               (t (- x y)))))
     (can-attack-diagonal-p
       (lambda (x y)
         (= (abs-diff (column x) (column y))
            (abs-diff (row x) (row y)))))
     (can-attack-p
       (lambda (x y)
         (or (can-attack-straight-p x y)
             (can-attack-diagonal-p x y))))
     (safe-place-p
       (lambda (x b)
         (cond ((null b) :t)
               ((can-attack-p (car b) x) :f)
               (t (safe-place-p x (cdr b))))))
     (next-column
       (lambda (q)
         (* (quotient (+ q board-size) board-size)
            board-size)))
     (solve
       (lambda (q c b r k)
         (cond ((equal c board-size)
                 (cond ((or (null limit)
                            (< k (car limit)))
                         (solve (incr (car b))
                                (decr c)
                                (cdr b)
                                (cons b r)
                                (+ '#1 k)))
                       (t r)))
               ((> (column q) c)
                 (cond ((null b) r)
                       (t (solve (incr (car b))
                                 (decr c)
                                 (cdr b)
                                 r
                                 k))))
               ((safe-place-p q b)
                 (solve (next-column q)
                        (incr c)
                        (cons q b)
                        r
                        k))
               (t (solve (incr q) c b r k))))))
    (map (lambda (b*)
           (map (lambda (x)
                  (remainder x board-size))
                b*))
         (reverse (solve '#0 '#0 () () '#0)))))
