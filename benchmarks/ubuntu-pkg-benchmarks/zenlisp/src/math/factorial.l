; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Compute X!:
; (factorial '#5) => '#120

(require '~nmath)

(define (factorial n)
  (letrec
    ((r* (lambda (n k)
           (cond ((< k '#2) n)
                 (t (let ((l (quotient k '#2)))
                      (* (r* n l)
                         (r* (+ n l) (- k l)))))))))
    (r* '#1 (natural n))))
