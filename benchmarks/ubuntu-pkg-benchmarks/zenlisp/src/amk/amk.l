; Another Micro KANREN
; By Nils M Holm, 2006-2008
;
; Based on "The Reasoned Schemer" by Daniel P. Friedman, et al.
; Inspired by "Sokuza" Mini-KANREN by Oleg Kiselyov.
;
; See the file LICENSE of the zenlisp distribution
; for conditions of use.

(define amk :t)

(require '~nmath)

(define (fail x) ())

(define (succeed x) (list x))

(define failed null)

(define (var x) (cons '? x))

(define (_) (var '_))

(define (var-p x)
  (and (not (atom x))
       (eq (car x) '?)))

(define empty-s ())

(define :bottom (var ':bottom))

(define (atomic x)
  (or (atom x)
      (eq (car x) 'closure)))

(define (ext-s x v s) (cons (cons x v) s))

(define (walk x s)
  (cond ((not (var-p x)) x)
        (t (let ((v (assq x s)))
             (cond (v (walk (cdr v) s))
                   (t x))))))

(define (unify x y s)
  (let ((x (walk x s))
        (y (walk y s)))
    (cond ((eq x y) s)
          ((var-p x) (ext-s x y s))
          ((var-p y) (ext-s y x s))
          ((or (atomic x) (atomic y)) :f)
          (t (let ((s (unify (car x) (car y) s)))
               (and s (unify (cdr x) (cdr y) s)))))))

(define (== g1 g2)
  (lambda (s)
    (let ((s2 (unify g1 g2 s)))
      (cond (s2 (succeed s2))
            (t (fail s))))))

(define (any . g*)
  (lambda (s)
    (letrec
      ((try (lambda g*
              (cond ((null g*) (fail s))
                    (t (append ((car g*) s)
                               (apply try (cdr g*))))))))
      (apply try g*))))

(define (all . g*)
  (lambda (s)
    (letrec
      ((try (lambda (g* s*)
              (cond ((null g*) s*)
                    (t (try (cdr g*)
                            (apply append
                                   (map (car g*) s*))))))))
      (try g* (succeed s)))))

(define (one . g*)
  (lambda (s)
    (letrec
      ((try (lambda g*
              (cond ((null g*) (fail s))
                    (t (let ((out ((car g*) s)))
                         (cond ((failed out)
                                 (apply try (cdr g*)))
                               (t out))))))))
      (apply try g*))))

(define (neg g)
  (lambda (s)
    (let ((out (g s)))
      (cond ((failed out) (succeed s))
            (t (fail s))))))

(define (choice x lst)
  (cond ((null lst) fail)
        (t (any (== x (car lst))
                (choice x (cdr lst))))))

(define (occurs x y s)
  (let ((v (walk y s)))
    (cond ((var-p y) (eq x y))
          ((var-p v) (eq x v))
          ((atomic v) :f)
          (t (or (occurs x (car v) s)
                 (occurs x (cdr v) s))))))

(define (circular x s)
  (let ((v (walk x s)))
    (cond ((eq x v) :f)
          (t (occurs x (walk x s) s)))))

(define (walk* x s)
  (letrec
    ((w* (lambda (x s)
           (let ((x (walk x s)))
             (cond ((var-p x) x)
                   ((atomic x) x)
                   (t (cons (w* (car x) s)
                            (w* (cdr x) s))))))))
    (cond ((circular x s) :bottom)
          ((eq x (walk x s)) empty-s)
          (t (w* x s)))))

(define (preserve-bottom s)
  (cond ((occurs :bottom s s) ())
        (t s)))

(define (reify-name n)
  (implode (append '#_, n)))

(define (reify v)
  (letrec
    ((reify-s
       (lambda (v s)
         (let ((v (walk v s)))
           (cond ((var-p v)
                   (ext-s v (reify-name (length s)) s))
                 ((atomic v) s)
                 (t (reify-s (cdr v)
                             (reify-s (car v)
                                      s))))))))
    (reify-s v empty-s)))

(define (run* x g)
  (preserve-bottom
    (map (lambda (s)
           (walk* x (append s (reify (walk* x s)))))
         (g empty-s))))

(define vp (var 'p))
(define vq (var 'q))

(define (conso a d p) (== (cons a d) p))

(define (caro p a) (conso a (_) p))

(define (cdro p d) (conso (_) d p))

(define (pairo p) (conso (_) (_) p))

(define (eqo x y) (== x y))

(define (nullo a) (eqo a ()))

(define (memo x l)
  (let ((vt (var 't)))
    (any (caro l x)
         (all (cdro l vt)
              (lambda (s)
                ((memo x vt) s))))))

(define (reverseo l r)
  (rmemqo r l))

(define (appendo x y r)
  (any (all (== x ())
            (== y r))
       (let ((vh (var 'h))
             (vt (var 't))
             (va (var 'a)))
         (all (conso vh vt x)
              (conso vh va r)
              (lambda (s)
                ((appendo vt y va) s))))))

(define (memqo x l r)
  (let ((vt (var 't)))
    (any (all (caro l x)
              (== l r))
         (all (cdro l vt)
              (lambda (s)
                ((memqo x vt r) s))))))

(define (rmemqo x l r)
  (let ((vt (var 't)))
    (any (all (cdro l vt)
              (lambda (s)
                ((rmemqo x vt r) s)))
         (all (caro l x)
              (== l r)))))

(define (dupso x r)
  (let ((vt (var 't))
        (vh (var 'h)))
    (all (conso vh vt x)
         (one (memqo vh vt r)
              (lambda (s)
                ((dupso vt r) s))))))
