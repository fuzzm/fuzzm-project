; zenlisp rational math functions
; By Nils M Holm, 2007, 2008
; Feel free to copy, share, and modify this code.
; See the file LICENSE for details.

; would use REQUIRE, but REQUIRE is in BASE
(cond ((defined 'base) :f)
      (t (load base)))

(define rmath :t)

(require 'imath)

(define (numerator x)
  (reverse (cdr (memq '/ (reverse x)))))

(define (denominator x) (cdr (memq '/ x)))

(define (rational-p x)
  (and (listp x)
       (memq '/ x)
       (integer-p (numerator x))
       (integer-p (denominator x))))

(define (r-number-p x)
  (or (integer-p x)
      (rational-p x)))

(define (make-rational num den)
  (append num '#/ den))

(define (rational x)
  (cond ((rational-p x) x)
        (t (make-rational x '#1))))

(define (r-zero x)
  (cond ((rational-p x) (r= x '#0))
        (t (i-zero x))))

(define (r-one x)
  (cond ((rational-p x) (r= x '#1))
        (t (i-one x))))

(define (%least-terms x)
  (let ((cd (gcd (numerator x) (denominator x))))
    (cond ((r-one cd) x)
          (t (make-rational (quotient (numerator x) cd)
                            (quotient (denominator x) cd))))))

(define (%decay x)
  (cond ((r-one (denominator x))
          (numerator x))
        (t x)))

(define r-normalize
  (let ()
    (lambda (x)
      (letrec
        ((norm-sign (lambda (x)
          (let ((num (numerator x))
                (den (denominator x)))
            (let ((pos (eq (i-negative num)
                           (i-negative den))))
              (make-rational (cond (pos (i-abs num))
                                   (t (cons '- (i-abs num))))
                      (i-abs den)))))))
        (cond ((rational-p x)
                (%decay (%least-terms (norm-sign x))))
              (t (i-normalize x)))))))

(define (r-integer x)
  (let ((xlt (+ '#0 x)))
    (cond ((rational-p xlt)
            (bottom (list 'r-integer x)))
          (t xlt))))

(define (r-natural x)
  (i-natural (r-integer x)))

(define (r-abs x)
  (cond ((rational-p x)
          (make-rational (i-abs (numerator x))
                         (i-abs (denominator x))))
        (t (i-abs x))))

(define (%equalize a b)
  (let ((num-a (numerator a))
        (num-b (numerator b))
        (den-a (denominator a))
        (den-b (denominator b)))
    (let ((cd (gcd den-a den-b)))
      (cond
        ((r-one cd)
          (list (make-rational (i* num-a den-b)
                               (i* den-a den-b))
                (make-rational (i* num-b den-a)
                               (i* den-b den-a))))
        (t (list (make-rational (quotient (i* num-a den-b) cd)
                                (quotient (i* den-a den-b) cd))
                 (make-rational (quotient (i* num-b den-a) cd)
                                (quotient (i* den-b den-a) cd))))))))

(define r+
  (let ()
    (lambda (a b)
      (let ((factors (%equalize (rational a) (rational b)))
            (radd
              (lambda (a b)
                (r-normalize
                  (make-rational (i+ (numerator a) (numerator b))
                                 (denominator a))))))
        (radd (car factors) (cadr factors))))))

(define r-
  (let ()
    (lambda (a b)
      (let ((factors (%equalize (rational a) (rational b)))
            (rsub
              (lambda (a b)
                (r-normalize
                  (make-rational (i- (numerator a) (numerator b))
                                 (denominator a))))))
        (rsub (car factors) (cadr factors))))))

(define (r* a b)
  (let ((rmul
          (lambda (a b)
            (r-normalize
              (make-rational (i* (numerator a) (numerator b))
                             (i* (denominator a) (denominator b)))))))
    (rmul (rational a) (rational b))))

(define (r/ a b)
  (let ((rdiv
          (lambda (a b)
            (r-normalize
              (make-rational (i* (numerator a) (denominator b))
                             (i* (denominator a) (numerator b)))))))
    (cond ((r-zero b) (bottom (list 'r/ a b)))
          (t (rdiv (rational a) (rational b))))))

(define r<
  (let ()
    (lambda (a b)
      (let ((factors (%equalize (rational a) (rational b))))
        (i< (numerator (car factors))
            (numerator (cadr factors)))))))

(define (r> a b) (r< b a))

(define (r<= a b) (eq (r> a b) :f))

(define (r>= a b) (eq (r< a b) :f))

(define r=
  (let ()
    (lambda (a b)
      (cond ((or (rational-p a) (rational-p b))
              (equal (%least-terms (rational a))
                     (%least-terms (rational b))))
            (t (i= a b))))))

(define (r-expt x y)
  (letrec
    ((rx (cond ((i-negative (r-integer y))
                 (r/ '#1 (rational x)))
               (t (rational x))))
     (square
       (lambda (x)
         (r* x x)))
     (exp
       (lambda (x y)
         (cond ((r-zero y) '#1)
               ((even y)
                 (square (exp x (quotient y '#2))))
               (t (r* x (square (exp x (quotient y '#2)))))))))
    (exp rx (i-abs (r-integer y)))))

(define (r-negative x)
  (cond ((rational-p x)
          (i-negative (numerator (r-normalize x))))
        (t (i-negative x))))

(define (r-negate x)
  (cond ((rational-p x)
          (let ((nx (r-normalize x)))
            (make-rational (i-negate (numerator nx))
                           (denominator nx))))
        (t (i-negate x))))

(define (r-max . a) (apply limit r> a))

(define (r-min . a) (apply limit r< a))

(define (r-sqrt square precision)
  (let ((e (make-rational '#1 (r-expt '#10 (r-natural precision)))))
    (letrec
      ((sqr (lambda (x)
              (cond ((r< (r-abs (r- (r* x x) square))
                         e)
                      x)
                    (t (sqr (r/ (r+ x (r/ square x))
                                '#2)))))))
      (sqr (n-sqrt (r-natural square))))))

(require 'iter)

(define * (arithmetic-iterator rational r* '#1))

(define + (arithmetic-iterator rational r+ '#0))

(define (- . x)
  (cond ((null x)
          (bottom '(too few arguments to rational -)))
        ((eq (cdr x) ()) (r-negate (car x)))
        (t (fold (lambda (a b)
                   (r- (rational a) (rational b)))
                 (car x)
                 (cdr x)))))

(define (/ . x)
  (cond ((null x)
          (bottom '(too few arguments to rational /)))
        ((eq (cdr x) ())
          (/ '#1 (car x)))
        (t (fold (lambda (a b)
                   (r/ (rational a) (rational b)))
                 (car x)
                 (cdr x)))))

(define < (predicate-iterator rational r<))

(define <= (predicate-iterator rational r<=))

(define = (predicate-iterator rational r=))

(define > (predicate-iterator rational r>))

(define >= (predicate-iterator rational r>=))

(define abs r-abs)

(define *epsilon* '#10)

(define expt r-expt)

(define integer r-integer)

(define max r-max)

(define min r-min)

(define natural r-natural)

(define negate r-negate)

(define negative r-negative)

(define number-p r-number-p)

(define one r-one)

(define (sqrt x) (r-sqrt x *epsilon*))

(define zero r-zero)
