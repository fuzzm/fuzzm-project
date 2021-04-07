; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Solve the Towers of Hanoi:
; (hanoi '#iii) => '(#ab #ac #bc #ab #ca #cb #ab)

(define (hanoi n)
  (letrec
    ((hanoi4
       (lambda (from to via n)
         (cond ((null n) ())
               (t (append (hanoi4 from via to (cdr n))
                          (list (list from to))
                          (hanoi4 via to from (cdr n))))))))
    (hanoi4 'a 'b 'c n)))
