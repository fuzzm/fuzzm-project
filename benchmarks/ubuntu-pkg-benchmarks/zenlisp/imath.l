; zenlisp integer math functions
; By Nils M Holm, 2007, 2008
; Feel free to copy, share, and modify this code.
; See the file LICENSE for details.

; would use REQUIRE, but REQUIRE is in BASE
(cond ((defined 'base) :f)
      (t (load base)))

(define imath :t)

(require 'nmath)

(define (integer-p a)
  (and (not (atom a))
       (or (natural-p a)
           (and (memq (car a) '#+-)
                (natural-p (cdr a))))))

(define (i-integer a)
  (cond ((eq (car a) '+) (cdr a))
        ((eq (car a) '-) a)
        ((digitp (car a)) a)
        (t (bottom (list 'i-integer a)))))

(define (i-natural a)
  (cond ((eq (car a) '+) (cdr a))
        ((digitp (car a)) a)
        (t (bottom (list 'i-natural a)))))

(define (i-normalize x)
  (cond ((eq (car x) '+)
          (n-normalize (cdr x)))
        ((eq (car x) '-)
          (let ((d (n-normalize (cdr x))))
            (cond ((n-zero d) d)
                  (t (cons '- d)))))
        (t (n-normalize x))))

(define (i-negative x) (eq (car x) '-))

(define (i-abs x)
  (cond ((i-negative x) (cdr x))
        ((eq (car x) '+) (cdr x))
        (t x)))

(define (i-zero x)
  (n-zero (i-abs x)))

(define (i-one x)
  (and (n-one (i-abs x))
       (neq (car x) '-)))

(define (i-negate x)
  (cond ((n-zero (i-abs x)) x)
        ((eq (car x) '-) (cdr x))
        ((eq (car x) '+) (cons '- (cdr x)))
        (t (cons '- x))))

(define (i+ a b)
  (cond ((and (not (i-negative a))
              (not (i-negative b)))
          (n+ (i-abs a) (i-abs b)))
        ((and (not (i-negative a))
              (i-negative b))
          (cond ((n> (i-abs a) (i-abs b))
                  (n- (natural a) (i-abs b)))
                (t (i-negate (n- (i-abs b) (natural a))))))
        ((and (i-negative a)
              (not (i-negative b)))
          (cond ((n> (i-abs a) (i-abs b))
                  (i-negate (n- (i-abs a) (natural b))))
                (t (n- (natural b) (i-abs a)))))
        (t (i-negate (n+ (i-abs a) (i-abs b))))))

(define (i- a b)
  (cond ((i-negative b)
          (i+ a (i-abs b)))
        ((i-negative a)
          (i+ a (i-negate b)))
        ((n< (i-abs a) (i-abs b))
          (i-negate (n- (i-abs b) (i-abs a))))
        (t (n- (i-abs a) (i-abs b)))))

(define (i< a b)
  (cond ((i-negative a)
          (cond ((not (i-negative b)) :t)
                (t (n< (i-abs b) (i-abs a)))))
        ((i-negative b) :f)
        (t (n< (i-abs a) (i-abs b)))))

(define (i> a b) (i< b a))

(define (i<= a b) (eq (i> a b) :f))

(define (i>= a b) (eq (i< a b) :f))

(define (i= a b)
  (equal (i-normalize a)
         (i-normalize b)))

(define (i* a b)
  (cond ((zero a) '#0)
        ((eq (i-negative a) (i-negative b))
          (n* (i-abs a) (i-abs b)))
        (t (i-negate (n* (i-abs a) (i-abs b))))))

(define (i-divide a b)
  (letrec
    ((sign
       (lambda (x)
         (cond ((eq (i-negative a) (i-negative b)) x)
               (t (cons '- x)))))
     (rsign
       (lambda (x)
         (cond ((i-negative a) (cons '- x))
               (t x))))
     (idiv
       (lambda (a b)
         (cond ((n-zero b) (bottom '(divide by zero)))
               ((n< (i-abs a) (i-abs b))
                 (list '#0 (rsign (i-abs a))))
               (t (let ((q (n-divide (i-abs a) (i-abs b))))
                    (list (sign (car q))
                          (rsign (cadr q)))))))))
    (idiv (i-integer a) (i-integer b))))

(define (i-quotient a b) (car (i-divide a b)))

(define (i-remainder a b) (cadr (i-divide a b)))

(define (modulo a b)
  (let ((rem (i-remainder a b)))
    (cond ((i-zero rem) '#0)
          ((eq (i-negative a)
               (i-negative b))
            rem)
          (t (i+ b rem)))))

(define (i-expt x y)
  (letrec
    ((i-expt
       (lambda (x y)
         (cond ((or (not (i-negative x))
                    (even y))
                 (n-expt (i-abs x) y))
               (t (i-negate (n-expt (i-abs x) y)))))))
    (i-expt (i-integer x) (natural y))))

(define (i-max . a) (apply limit i> a))

(define (i-min . a) (apply limit i< a))

(define (i-sqrt x)
  (cond ((i-negative x)
          (bottom (list 'i-sqrt x)))
        (t (n-sqrt x))))

(define (i-gcd a b)
  (n-gcd (i-abs a) (i-abs b)))

(define (i-lcm a b)
  (n-lcm (i-abs a) (i-abs b)))

(require 'iter)

(define integer i-integer)

(define * (arithmetic-iterator integer i* '#1))

(define + (arithmetic-iterator integer i+ '#0))

(define (- . x)
  (cond ((null x)
          (bottom '(too few arguments to integer -)))
        ((eq (cdr x) ())
          (i-negate (car x)))
        (t (fold (lambda (a b)
                   (i- (integer a) (integer b)))
                 (car x)
                 (cdr x)))))

(define < (predicate-iterator integer i<))

(define <= (predicate-iterator integer i<=))

(define = (predicate-iterator integer i=))

(define > (predicate-iterator integer i>))

(define >= (predicate-iterator integer i>=))

(define abs i-abs)

(define divide i-divide)

(define expt i-expt)

(define gcd (arithmetic-iterator integer i-gcd '#0))

(define lcm (arithmetic-iterator integer i-lcm '#1))

(define max i-max)

(define min i-min)

(define natural i-natural)

(define negate i-negate)

(define negative i-negative)

(define number-p integer-p)

(define one i-one)

(define quotient i-quotient)

(define remainder i-remainder)

(define sqrt i-sqrt)

(define zero i-zero)
