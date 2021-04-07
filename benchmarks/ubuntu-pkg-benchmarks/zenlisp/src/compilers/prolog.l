; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; A simple PROLOG interpreter.

; This program is heavily based on a tiny PROLOG
; interpreter written in MACLISP by Ken Kahn.

(define (prolog q db)
  (letrec
    ((variable-p
       (lambda (x)
         (and (not (atom x))
              (eq (car x) '?))))
     (new-scope
       (lambda (env ident)
         (cond ((atom env) env)
               ((variable-p env) (append env ident))
               (t (cons (new-scope (car env) ident)
                        (new-scope (cdr env) ident))))))
     (new-env-id
       (lambda (x)
         (list (cons 'i (car x)))))
     (value-of
       (lambda (x env)
         (cond ((variable-p x)
                 (let ((v (assoc x env)))
                   (cond (v (value-of (cdr v) env))
                         (t x))))
               (t x))))
     (unify (lambda (x y env)
       (let ((x (value-of x env))
             (y (value-of y env)))
         (cond ((variable-p x) (cons (cons x y) env))
               ((variable-p y) (cons (cons y x) env))
               ((atom x) (cond ((eq x y) env) (t ())))
               ((atom y) (cond ((eq x y) env) (t ())))
               (t (let ((new (unify (car x) (car y) env)))
                    (cond ((null new) ())
                          (t (unify (cdr x) (cdr y) new)))))))))
     (try-rules (lambda (rules goals env ident result)
       (cond
         ((null rules) result)
         (t (let ((thisrule (new-scope (car rules) ident)))
              (let ((newenv (unify (car goals)
                                   (car thisrule) env)))
                (cond
                  ((null newenv)
                    (try-rules (cdr rules) goals env ident result))
                  (t (let ((res (prove (append (cdr thisrule)
                                               (cdr goals))
                                       newenv
                                       (new-env-id ident))))
                       (try-rules (cdr rules) goals env ident
                                  (append result res)))))))))))
     (list-env (lambda (env)
       (letrec
         ((ls-env
            (lambda (e res)
              (cond ((null (cdr e)) (list res))
                    ((null (caddr (caar e)))
                      (ls-env
                        (cdr e)
                        (cons (cons (cadr (caar e))
                                    (value-of (caar e) env))
                              res)))
                    (t (ls-env (cdr e) res))))))
         (ls-env env ()))))
     (prove
       (lambda (goals env ident)
         (cond ((null goals) (list-env env))  
               (t (try-rules db goals env ident ()))))))
    (prove (list (new-scope q '(())))
           '((()))
           '((i)))))
