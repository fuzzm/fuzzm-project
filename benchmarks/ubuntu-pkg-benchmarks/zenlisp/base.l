; zenlisp base functions
; By Nils M Holm, 2007, 2008
; Feel free to copy, share, and modify this code.
; See the file LICENSE for details.

(define base :t)

(define (null x) (eq x ()))

(define (id x) x)

(define (list . x) x)

(define (not a) (eq a :f))

(define (neq x y) (eq (eq x y) :f))

(define (caaaar x) (car (car (car (car x)))))
(define (caaadr x) (car (car (car (cdr x)))))
(define (caadar x) (car (car (cdr (car x)))))
(define (caaddr x) (car (car (cdr (cdr x)))))
(define (cadaar x) (car (cdr (car (car x)))))
(define (cadadr x) (car (cdr (car (cdr x)))))
(define (caddar x) (car (cdr (cdr (car x)))))
(define (cadddr x) (car (cdr (cdr (cdr x)))))
(define (cdaaar x) (cdr (car (car (car x)))))
(define (cdaadr x) (cdr (car (car (cdr x)))))
(define (cdadar x) (cdr (car (cdr (car x)))))
(define (cdaddr x) (cdr (car (cdr (cdr x)))))
(define (cddaar x) (cdr (cdr (car (car x)))))
(define (cddadr x) (cdr (cdr (car (cdr x)))))
(define (cdddar x) (cdr (cdr (cdr (car x)))))
(define (cddddr x) (cdr (cdr (cdr (cdr x)))))

(define (caaar x) (car (car (car x))))
(define (caadr x) (car (car (cdr x))))
(define (cadar x) (car (cdr (car x))))
(define (caddr x) (car (cdr (cdr x))))
(define (cdaar x) (cdr (car (car x))))
(define (cdadr x) (cdr (car (cdr x))))
(define (cddar x) (cdr (cdr (car x))))
(define (cdddr x) (cdr (cdr (cdr x))))

(define (caar x) (car (car x)))
(define (cadr x) (car (cdr x)))
(define (cdar x) (cdr (car x)))
(define (cddr x) (cdr (cdr x)))

(define (fold f x a)
  (letrec
    ((fold2
       (lambda (a res)
         (cond ((null a) res)
               (t (fold2 (cdr a)
                         (f res (car a))))))))
    (fold2 a x)))

(define (fold-r f x a)
  (letrec
    ((fold2
       (lambda (a)
         (cond ((null a) x)
               (t (f (car a)
                     (fold2 (cdr a))))))))
    (fold2 a)))

(define (reverse a)
  (letrec
    ((reverse2
       (lambda (a b)
         (cond ((null a) b)
               (t (reverse2 (cdr a)
                            (cons (car a) b)))))))
    (reverse2 a ())))

(define (append . a)
  (letrec
    ((append2
       (lambda (a b)
         (cond ((null a) b)
               (t (append2 (cdr a) (cons (car a) b)))))))
    (fold (lambda (a b) (append2 (reverse a) b))
          ()
          a)))

(define (equal a b)
  (cond ((eq a b) :t)
        ((or (atom a) (atom b))
          (eq a b))
        (t (and (equal (car a) (car b))
                (equal (cdr a) (cdr b))))))

(define (assoc x a)
  (cond ((null a) :f)
        ((equal (caar a) x) (car a))
        (t (assoc x (cdr a)))))

(define (assq x a)
  (cond ((null a) :f)
        ((eq (caar a) x) (car a))
        (t (assq x (cdr a)))))

(define (listp x)
  (or (null x)
      (and (not (atom x))
           (listp (cdr x)))))

(define (map f . a)
  (letrec
    ((map-car
       (lambda (f a r)
         (cond ((null a) (reverse r))
               (t (map-car f (cdr a) (cons (f (car a)) r))))))
     (car-of
       (lambda (a)
         (map-car car a ())))
     (cdr-of
       (lambda (a)
         (map-car cdr a ())))
     (any-null
       (lambda (a)
         (apply or (map-car null a ()))))
     (map2
       (lambda (a b)
         (cond ((any-null a) (reverse b))
               (t (map2 (cdr-of a)
                        (cons (apply f (car-of a)) b)))))))
    (cond ((null a) (bottom '(too few arguments to map)))
          (t (map2 a ())))))

(define (member x a)
  (cond ((null a) :f)
        ((equal (car a) x) a)
        (t (member x (cdr a)))))

(define (memq x a)
  (cond ((null a) :f)
        ((eq (car a) x) a)
        (t (memq x (cdr a)))))

(define (require x)
  (letrec
    ((require2
       (lambda (sym file)
         (cond ((defined sym) :f)
               (t (apply load (list file)))))))
    (let ((xx (explode x)))
      (cond ((eq (car xx) '~)
              (require2 (implode (cdr xx)) x))
            (t (require2 x x))))))
