; Another Micro KANREN Example Program
; By Nils M Holm, 2006,2007
; See the file LICENSE of the ArrowLISP distribution
; for conditions of use.

; Solve the Zebra puzzle:
; zl -n 1024K
; (load zebra)
; (zebra)

(require 'amk)

(define (lefto x y l)
  (let ((vt (var 't)))
    (any (all (caro l x)
              (cdro l vt)
              (caro vt y))
         (all (cdro l vt)
              (lambda (s)
                ((lefto x y vt) s))))))

(define (nexto x y l)
  (any (lefto x y l)
       (lefto y x l)))

(define (zebra)
  (let ((h (var 'h)))
    (run*
      h
      (all (== h (list (list 'norwegian (_) (_) (_) (_))
                       (_)
                       (list (_) (_) 'milk (_) (_))
                       (_)
                       (_)))
           (memo  (list 'englishman (_) (_) (_) 'red) h)
           (lefto (list (_) (_) (_) (_) 'green)
                  (list (_) (_) (_) (_) 'ivory) h)
           (nexto (list 'norwegian (_) (_) (_) (_))
                  (list (_) (_) (_) (_) 'blue) h)
           (memo  (list (_) 'kools (_) (_) 'yellow) h)
           (memo  (list 'spaniard (_) (_) 'dog (_)) h)
           (memo  (list (_) (_) 'coffee (_) 'green) h) 
           (memo  (list 'ukrainian (_) 'tea (_) (_)) h)
           (memo  (list (_) 'luckystrikes 'orangejuice (_) (_)) h)
           (memo  (list 'japanese 'parliaments (_) (_) (_)) h)
           (memo  (list (_) 'oldgolds (_) 'snails (_)) h)
           (nexto (list (_) (_) (_) 'horse (_))
                  (list (_) 'kools (_) (_) (_)) h)
           (nexto (list (_) (_) (_) 'fox (_))
                  (list (_) 'chesterfields (_) (_) (_)) h)
;           (memo  (list (_) (_) 'water (_) (_)) h)
           (memo  (list (_) (_) (_) 'zebra (_)) h)))))
