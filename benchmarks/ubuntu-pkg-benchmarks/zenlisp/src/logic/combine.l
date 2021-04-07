; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Create combinations of a set with (COMBINE*)
; and without (COMBINE) repetition:
; (combine '#2 '(a b c)) => '(#ab #ac #bc)
; (combine* '#2 '(a b c)) => '(#aa #ab #ac #bb #bc #cc)

(require '~nmath)

(define (combine3 n set rest)
  (letrec
    ((tails-of
       (lambda (set)
         (cond ((null set) ())
               (t (cons set (tails-of (cdr set)))))))
     (combinations
       (lambda (n set)
         (cond
           ((zero n) ())
           ((one n) (map list set))
           (t (apply
                append
                (map (lambda (tail)
                       (map (lambda (sub)
                              (cons (car tail) sub))
                            (combinations (- n '#1) (rest tail))))
                     (tails-of set))))))))
    (combinations n set)))

(define (combine n set)
  (combine3 n set cdr))

(define (combine* n set)
  (combine3 n set id))
