; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Convert arithmetic expressions in infix notation
; to S-expressions:
; (infix->prefix '#12+34*56^[7+8])
; => '(+ '#12 (* '#34 (expt '#56 (+ '#7 '#8))))

; Infix expressions are represented by flat lists of
; variables (atoms) operators (atoms) and zenlisp-style
; numbers (eg #5).

; The following operators are recognized: +, - (both
; unary and binary), *, /, ^. Brackets ([, ]) are
; recoginzed as parentheses. XX is equal to X*X if
; X is a symbol.

(require '~rmath)

(define (infix->prefix x)
  (letrec
    ((symbol-p
       (lambda (x)
         (and (memq x '#abcdefghijklmnopqrstuvwxyz) :t)))
     (number
       (lambda (x r)
         (cond ((or (null x)
                    (not (digitp (car x))))
                 (list (list 'quote (reverse r)) x))
               (t (number (cdr x) (cons (car x) r))))))
     (symbol
       (lambda (x)
         (list (car x) (cdr x))))
     (expr car)
     (rest cadr)
     (car-of-rest caadr)
     (cdr-of-rest cdadr)
     ; factor := [ sum ]
     ;         | - factor
     ;         | Number
     ;         | Symbol
     (factor
       (lambda (x)
         (cond ((null x)
                 (bottom 'syntax 'error 'at: x))
               ((eq (car x) '[)
                 (let ((xsub (sum (cdr x))))
                   (cond ((null (rest xsub))
                           (bottom 'missing-right-paren))
                         ((eq (car-of-rest xsub) '])
                           (list (expr xsub) (cdr-of-rest xsub)))
                         (t (bottom 'missing-right-paren)))))
               ((eq (car x) '-)
                 (let ((fac (factor (cdr x))))
                   (list (list '- (expr fac)) (rest fac))))
               ((digitp (car x))
                 (number x ()))
               ((symbol-p (car x))
                 (symbol x))
               (t (bottom 'syntax 'error 'at: x)))))
     ; power := factor
     ;        | factor ^ power
     (power (lambda (x)
       (let ((left (factor x)))
         (cond ((null (rest left)) left)
               ((eq (car-of-rest left) '^)
                 (let ((right (power (cdr-of-rest left))))
                   (list (list 'expt (expr left) (expr right))
                         (rest right))))
               (t left)))))
     ; term := power
     ;       | power Symbol
     ;       | power * term
     ;       | power / term
     (term2
       (lambda (out in)
         (cond ((null in) (list out in))
               ((symbol-p (car in))
                 (let ((right (power in)))
                   (term2 (list '* out (expr right))
                          (rest right))))
               ((eq (car in) '*)
                 (let ((right (power (cdr in))))
                   (term2 (list '* out (expr right))
                          (rest right))))
               ((eq (car in) '/)
                 (let ((right (power (cdr in))))
                   (term2 (list '/ out (expr right))
                          (rest right))))
               (t (list out in)))))
     (term
       (lambda (x)
         (let ((left (power x)))
           (term2 (expr left) (rest left)))))
     ; sum := term
     ;      | term + sum
     ;      | term - sum
     (sum2
       (lambda (out in)
         (cond ((null in) (list out in))
               ((eq (car in) '+)
                 (let ((right (term (cdr in))))
                   (sum2 (list '+ out (expr right))
                         (rest right))))
               ((eq (car in) '-)
                 (let ((right (term (cdr in))))
                   (sum2 (list '- out (expr right))
                         (rest right))))
               (t (list out in)))))
     (sum
       (lambda (x)
         (let ((left (term x)))
           (sum2 (expr left) (rest left))))))
  (let ((px (sum x)))
    (cond ((not (null (rest px)))
            (bottom (list 'syntax 'error 'at: (cadr px))))
          (t (expr px))))))
