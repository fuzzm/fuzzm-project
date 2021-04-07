; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Compute the intersection of some sets:
; (intersection '#abcd '#bcde '#cdef) => '#cd

(define (intersection . a)
  (letrec
    ((intersection3 (lambda (a b r)
      (cond ((null a)
              (reverse r))
            ((member (car a) b)
              (intersection3 (cdr a) b (cons (car a) r)))
            (t (intersection3 (cdr a) b r))))))
    (fold (lambda (a b)
            (intersection3 a b ()))
          (car a)
          a)))
