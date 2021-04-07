; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Rename variables of lambda expressions
; (perform alpha convfersion):
; (lambda-rename '(lambda (x) (lambda (x) x)))
; => (lambda (x:0) (lambda (x:1) x:1))

(require '~nmath)

(define (map-car-i f a)
  (cond ((null a) ())
        ((atom a) (f a))
        (t (cons (f (car a))
                 (map-car-i f (cdr a))))))

(define (lambda-rename expr)
  (letrec
    ((add
       (lambda (name level)
         (implode (append (explode name) '#: level))))
     (ext-env (lambda (env vars level)
       (cond ((null vars) env)
             ((atom vars)
               (cons (cons vars (add vars level)) env))
             (t (ext-env (cons (cons (car vars)
                                     (add (car vars) level))
                               env)
                         (cdr vars)
                         level)))))
     (envst (lambda (name env)
       (let ((v (assq name env)))
         (cond (v (cdr v))
               (t name)))))
     (rename-vars (lambda (expr env level)
       (cond ((atom expr) (envst expr env))
             ((eq (car expr) 'quote) expr)
             ((eq (car expr) 'lambda)
               (let ((args (cadr expr))
                     (body (caddr expr)))
                 (let ((new-env (ext-env env args level)))
                   (list 'lambda
                         (rename-vars args new-env level)
                         (rename-vars body new-env (+ '#1 level))))))
             (t (map-car-i (lambda (x) (rename-vars x env level))
                           expr))))))
    (rename-vars expr () '#0)))

(define (subst name sub)
  (let ((v (assq name sub)))
      (cond (v (cdr v))
		(t name))))

(define (subst-vars expr sub)
  (cond ((atom expr) (subst expr sub))
        ((eq (car expr) 'quote) expr)
        (t (map-car-i (lambda (x)
                        (subst-vars x sub))
                      expr))))

(define (beta-reduce app)
  (let ((app (lambda-rename app)))
    (let ((vars (cadar app))
          (args (cdr app))
          (body (caddar app)))
      (subst-vars body (map cons vars args)))))

