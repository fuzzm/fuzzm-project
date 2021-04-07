; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Create a generator:
; (load ~nmath)
; (generator '#0 (lambda (x) (+ x '#1))) => '(#0 . (closure ()))
; ((cdr **)) => '(#1 . (closure ()))
; ((cdr **)) => '(#2 . (closure ()))
; ((cdr **)) => '(#3 . (closure ()))
; ...

(define (value g) (car g))
(define (next g) ((cdr g)))

(define (generator start step)
  (lambda ()
    (cons start
          (generator (step start) step))))
