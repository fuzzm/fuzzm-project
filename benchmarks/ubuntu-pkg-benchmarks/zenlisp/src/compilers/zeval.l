; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Evaluate a pure LISP expression in a given environment:
; (zeval '(letrec ((foo
;                    (lambda (x)
;                      (cond ((eq x ()) 'foo)
;                            (t (foo (cdr x)))))))
;           (foo '#xxxxx)) ())
; => 'foo
;
; This is basically an interpreter for zenlisp modulo DEFINE.
; Yes, it is lexically scoped and tail-recursive.

(define (zeval x e)
  (letrec
    ((initial-env
       (list (cons 'closure        'closure)
             (cons 't              ':t)
             (cons ':t             ':t)
             (cons ':f             ':f)
             (cons 'and            '(%special . and))
             (cons 'apply          '(%special . apply))
             (cons 'cond           '(%special . cond))
             (cons 'eval           '(%special . eval))
             (cons 'lambda         '(%special . lambda))
             (cons 'let            '(%special . let))
             (cons 'letrec         '(%special . letrec))
             (cons 'or             '(%special . or))
             (cons 'quote          '(%special . quote))
             (cons 'atom           (cons '%primitive atom))
             (cons 'bottom         (cons '%primitive bottom))
             (cons 'car            (cons '%primitive car))
             (cons 'cdr            (cons '%primitive cdr))
             (cons 'cons           (cons '%primitive cons))
             (cons 'defined        (cons '%primitive defined))
             (cons 'eq             (cons '%primitive eq))
             (cons 'explode        (cons '%primitive explode))
             (cons 'implode        (cons '%primitive implode))
             (cons 'recursive-bind (cons '%primitive recursive-bind))))
     (value-of
       (lambda (x e)
         (let ((v (assq x e)))
           (cond ((or (not v) (eq (cdr v) '%void))
                   (bottom 'undefined: x))
                 (t (cdr v))))))
     (ev-list
       (lambda (x e)
         (cond ((null x) ())
               ((atom x) (bottom 'improper-list-in-application: x))
               (t (cons (ev (car x) e)
                        (ev-list (cdr x) e))))))
     (check-args
       (lambda (a n more)
         (cond ((null n) (or more (null a)))
               ((null a) :f)
               (t (check-args (cdr a)
                              (cdr n)
                              more)))))
     (wrong-args
       (lambda (name args)
         (bottom 'wrong-number-of-arguments:
                 (cons name args))))
     (args-ok
       (lambda (name a n more)
         (cond ((check-args a n more) :t)
               (t (wrong-args name a)))))
     (eval-until
       (lambda (t/f a e)
         (cond ((null (cdr a)) (car a))
               ((atom a) (bottom 'improper-list-in-and/or: a))
               (t (let ((v (ev (car a) e)))
                     (cond ((eq (not v) (not t/f))
                             (list 'quote v))
                           (t (eval-until t/f (cdr a) e))))))))
     (do-and
       (lambda (a e)
         (cond ((null a) :t)
               (t (eval-until :f a e)))))
     (clause-p
       (lambda (x)
         (and (not (atom x))
              (not (atom (cdr x)))
              (null (cddr x)))))
     (do-cond
       (lambda (a e)
         (cond ((null a)
                 (bottom 'no-default-in-cond))
               ((atom a)
                 (bottom 'improper-list-in-cond))
               ((not (clause-p (car a)))
                 (bottom 'bad-clause-in-cond: (car a)))
               (t (let ((v (ev (caar a) e)))
                    (cond (v (cadar a))
                          (t (do-cond (cdr a) e))))))))
     (do-eval
       (lambda (args e)
         (and (args-ok 'eval args '#i :f)
              (ev (car args) e))))
     (lambda-args
       (lambda (a)
         (cond ((null a) ())
               ((atom a) (list a))
               (t (cons (car a)
                        (lambda-args (cdr a)))))))
     (add-free-var
       (lambda (fenv var e)
         (cond ((assq var fenv) fenv)
               (t (let ((v (assq var e)))
                    (cond (v (cons v fenv))
                          (t (cons (cons var '%void) fenv))))))))
     (capture
       (lambda (bound x e)
         (letrec
           ((collect
              (lambda (x free)
                (cond ((null x) free)
                      ((atom x)
                        (cond ((memq x bound) free)
                              (t (add-free-var free x e))))
                      (t (collect (car x)
                                  (collect (cdr x) free)))))))
           (collect x ()))))
     (do-lambda
       (lambda (args e)
         (and (args-ok 'lambda args '#ii :f)
              (list 'closure
                    (car args)
                    (cadr args)
                    (capture (lambda-args (car args))
                             (cadr args)
                             e)))))
     (do-or
       (lambda (a e)
         (cond ((null a) :f)
               (t (eval-until :t a e)))))
     (do-quote
       (lambda (args)
         (and (args-ok 'quote args '#i :f)
              (car args))))
     (make-env
       (lambda (fa aa)
         (cond ((null fa)
                 (cond ((null aa) ())
                       (t (bottom 'too-many-arguments))))
               ((atom fa)
                 (list (cons fa aa)))
               ((null aa)
                 (bottom 'too-few-arguments))
               (t (cons (cons (car fa) (car aa))
                        (make-env (cdr fa) (cdr aa)))))))
     (beta
       (lambda (expr fa aa lex-env e le fix)
         (ev2 expr e (append (fix (make-env fa aa)) lex-env le))))
     (binding-p
       (lambda (x)
         (clause-p x)))
     (do-let/rec
       (lambda (args e le fix)
         (cond ((not (args-ok 'let/letrec args '#ii :f)) :f)
               ((not (apply and (map binding-p (car args))))
                 (bottom 'bad-let/letrec-syntax: (car args)))
               (t (let ((formals (map car (car args)))
                        (actuals (map cadr (car args))))
                    (beta (cadr args)
                          formals
                          (ev-list actuals le)
                          ()
                          e le fix))))))
     (apply-fn
       (lambda (fn args e le)
         (cond ((eq (car fn) '%primitive)
                 (apply (cdr fn) args))
               ((eq (car fn) '%special)
                 (apply-special (cdr fn) args e le))
               ((eq (car fn) 'closure)
                 (beta (caddr fn)
                       (cadr fn)
                       args
                       (cadddr fn)
                       e le id))
               (t (bottom 'application-of-non-function: fn)))))
     (make-args
       (lambda (a)
         (cond ((null (cdr a))
                 (cond ((atom (car a))
                         (bottom 'improper-argument-list:
                                 (car a)))
                       (t (car a))))
               (t (cons (car a) (make-args (cdr a)))))))
     (apply-special
       (lambda (fn args e le)
         (cond ((eq fn 'and)
                 (ev2 (do-and args le) e le))
               ((eq fn 'apply) 
                 (let ((args (ev-list args le)))
                   (and (args-ok 'apply args '#ii :t)
                        (apply-fn (car args)
                                  (make-args (cdr args))
                                  e
                                  e))))
               ((eq fn 'cond)
                 (ev2 (do-cond args le) e le))
               ((eq fn 'eval)
                 (ev2 (do-eval args le) e le))
               ((eq fn 'lambda)
                 (do-lambda args le))
               ((eq fn 'let)
                 (do-let/rec args e le id))
               ((eq fn 'letrec)
                 (do-let/rec args e le recursive-bind))
               ((eq fn 'or)
                 (ev2 (do-or args le) e le))
               ((eq fn 'quote)
                 (do-quote args))
               (t (bottom 'internal:bad-special-operator: fn)))))
 
     (function-p
       (lambda (x)
         (or (eq (car x) '%primitive)
             (eq (car x) 'closure))))
     (special-p
       (lambda (x)
         (eq (car x) '%special)))
     (ev2
       (lambda (x e le)
         (cond
           ((null x) ())
           ((atom x) (value-of x le))
           (t (let ((f (ev (car x) le)))
                (cond ((eq f 'closure) x)
                      ((atom f)
                        (bottom 'application-of-non-function: f))
                      (t (let ((args (cond ((function-p f)
                                             (ev-list (cdr x) le))
                                           (t (cdr x))))
                               (new-e (cond ((special-p f) le)
                                            (t e))))
                           (apply-fn f args e new-e)))))))))
     (ev (lambda (x e)
           (ev2 x e e))))
    (ev x (append e initial-env))))
