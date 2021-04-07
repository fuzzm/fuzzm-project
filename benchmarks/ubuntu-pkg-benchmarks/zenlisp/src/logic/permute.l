; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Create permutations of a set with (PERMUTE*)
; and without (PERMUTE) repetition:
; (permute '#2 '(a b)) => '(#ab #ba)
; (permute* '#2 '(a b)) => '(#aa #ab #ba #bb)

(require 'combine)

(define (permute n set)
  (letrec
    ((rotate
       (lambda (x)
         (append (cdr x) (list (car x)))))
     (rotations
       (lambda (x)
         (letrec
           ((rot (lambda (x n)
                   (cond ((null n) ())
                         (t (cons x (rot (rotate x)
                                         (cdr n))))))))
           (rot x x))))
     (permutations
       (lambda (set)
         (cond
           ((null set) ())
           ((null (cdr set)) (list set))
           ((null (cddr set)) (rotations set))
           (t (apply append
                     (map (lambda (rotn)
                            (map (lambda (x)
                                   (cons (car rotn) x))
                                 (permutations (cdr rotn))))
                          (rotations set))))))))
    (apply append (map permutations (combine n set)))))

(define (permute* n set)
  (cond
    ((zero n) ())
    ((one n) (map list set))
    (t (apply append
              (map (lambda (x)
                     (map (lambda (sub)
                            (cons x sub))
                          (permute* (- n '#1) set)))
                   set)))))
