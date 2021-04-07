; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Convert arithmetic expressions from prefix to
; infix notation:
; (prefix->infix '(+ '#+2 (* '#+3 (expt '#+4 (+ '#+5 '#+6)))))
; => '#2+3*4^[5+6]

; Prefix expressions may contain variables
; (single-char symbols like X), numbers (like '#57),
; and these functions: +, - (unary or binary), *, /,
; EXPT. PREFIX->INFIX will insert parentheses
; ([ and ]) where necessary.

(define (prefix->infix x)
  (letrec
    ((ops '((+ . +) (- . -) (* . *) (/ . /) (expt . ^)))
     (left '#+-*/)
     (precedence '(high ([]) (expt) (* /) (+ -) low))
     (function-p
       (lambda (x)
         (and (memq x '(+ - * / expt)) :t)))
     (left-assoc-p
       (lambda (x)
         (and (memq x left))))
     (symbol-p
       (lambda (x)
         (and (memq x '#abcdefghijklmnopqrstuvwxyz) :t)))
     (numeric-p
       (lambda (x)
         (and (not (atom x))
              (eq (car x) 'quote))))
     (atomic-p
       (lambda (x)
         (or (function-p x)
             (symbol-p x)
             (numeric-p x))))
     (unary-p
       (lambda (x)
         (and (not (null (cdr x)))
              (null (cddr x)))))
     (higher-prec-p
       (lambda (x y)
         (letrec
           ((hpp (lambda (x y prec)
                   (cond ((atom prec) :f)
                         ((memq x (car prec))
                           (not (memq y (car prec))))
                         ((memq y (car prec)) :f)
                         (t (hpp x y (cdr prec)))))))
           (cond ((atomic-p x) (not (atomic-p y)))
                 ((atomic-p y) :f)
                 ((unary-p x) (not (unary-p y)))
                 ((unary-p y) :f)
                 (t (hpp (car x) (car y) (cdr precedence)))))))
     (paren
       (lambda (x)
         (cond ((atomic-p x) x)
               (t (list '[] x)))))
     (add-parens
       (lambda (x)
         (cond
           ((atomic-p x) x)
           (t (let ((x (map add-parens x)))
                (cond ((unary-p x)
                        (cond ((atomic-p (cadr x)) x)
                              ((unary-p (cadr x)) x)
                              (t (list (car x)
                                       (paren (cadr x))))))
                      ((left-assoc-p (car x))
                        (list (car x)
                              (cond ((higher-prec-p x (cadr x))
                                      (paren (cadr x)))
                                    (t (cadr x)))
                              (cond ((higher-prec-p (caddr x) x)
                                      (caddr x))
                                    (t (paren (caddr x))))))
                      (t (list (car x)
                               (cond ((higher-prec-p (cadr x) x)
                                       (cadr x))
                                     (t (paren (cadr x))))
                               (cond ((higher-prec-p x (caddr x))
                                       (paren (caddr x)))
                                     (t (caddr x)))))))))))
     (infix
       (lambda (x)
         (cond
           ((numeric-p x)
             (cadr x))
           ((symbol-p x)
             (list x))
           ((and (eq (car x) '-)
                 (not (atom (cdr x)))
                 (null (cddr x)))
             (append '#- (infix (cadr x))))
           ((and (eq (car x) '[])
                 (not (atom (cdr x)))
                 (null (cddr x)))
             (append '#[ (infix (cadr x)) '#]))
           ((and (not (atom x))
                 (not (atom (cdr x)))
                 (not (atom (cddr x)))
                 (null (cdddr x))
                 (function-p (car x)))
             (append (infix (cadr x))
                     (list (cdr (assq (car x) ops)))
                     (infix (caddr x))))
           (t (bottom (list 'syntax 'error: x)))))))
    (infix (add-parens x))))
