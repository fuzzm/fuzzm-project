; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Compute A hyperN B:
; (hyper '#4 '#3 '#3) => '#7625597484987
; A, B, and N must all be natural.

(require '~nmath)

(define (hyper n a b)
  (cond ((equal n '#0) (+ '#1 a))
        ((equal n '#1) (+ a b))
        ((one b) a)
        ((equal n '#2) (* a b))
        ((equal n '#3) (expt a b))
        ((equal n '#4) (expt a (hyper n a (- b '#1))))
        ((> n '#4) (hyper (- n '#1) a (hyper n a (- b '#1))))))
