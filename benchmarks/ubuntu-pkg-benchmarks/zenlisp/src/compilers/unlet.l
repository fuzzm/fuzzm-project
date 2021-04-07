; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Convert LET to LAMBDA.
; (unlet '(let ((x '(a.b))) (cdr x))) => '((lambda (x) (cdr x)) '(a.b))

(define (unlet x)
  (letrec

    ((make-lambda (lambda (let-expr)
      (let ((env (cadr let-expr)))
        (let ((formals (map car env))
              (actuals (map (lambda (x) (unlet (cadr x)))
                            env))
              (body (unlet (caddr let-expr))))
          (append (list (list 'lambda formals body))
                  actuals))))))

    (cond ((atom x) x)
      ((eq (car x) 'quote) x)
      ((eq (car x) 'let) (make-lambda x))
      ((eq (car x) 'lambda) ; handle dotted formals
        (list 'lambda (cadr x) (unlet (caddr x))))
      (t (map unlet x)))))
