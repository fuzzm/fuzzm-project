; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Compute the sum of a sequence of positive numbers:
; (sum '#2 '#5) => '#14

(require '~nmath)

(define (sum n m)
  (letrec
    ((s (lambda (n m)
          (let ((x (+ '#1 (- m n))))
            (+ (quotient (+ (* x x) x)
                         '#2)
               (* (- n '#1) x))))))
    (let ((nn (natural n))
          (nm (natural m)))
      (cond ((or (zero nn) (zero nm))
              (bottom 'sum n m))
            (t (s (min nn nm) (max nn nm)))))))
