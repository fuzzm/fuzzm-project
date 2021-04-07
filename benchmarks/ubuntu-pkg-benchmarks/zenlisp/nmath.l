; zenlisp natural math functions
; By Nils M Holm, 2007
; Feel free to copy, share, and modify this code.
; See the file LICENSE for details.

; would use REQUIRE, but REQUIRE is in BASE
(cond ((defined 'base) :f)
      (t (load base)))

(define nmath :t)

(define 0 '0)
(define 1 '1)
(define 2 '2)
(define 3 '3)
(define 4 '4)
(define 5 '5)
(define 6 '6)
(define 7 '7)
(define 8 '8)
(define 9 '9)

(define *digits* '#0123456789)

(define (digitp x) (and (memq x *digits*) :t))

(define (succ x)
  (cond ((eq x 0) 1)
        ((eq x 1) 2)
        ((eq x 2) 3)
        ((eq x 3) 4)
        ((eq x 4) 5)
        ((eq x 5) 6)
        ((eq x 6) 7)
        ((eq x 7) 8)
        ((eq x 8) 9)
        ((eq x 9) :f)
        (t (bottom '(not a digit:) x))))

(define (pred x)
  (cond ((eq x 1) 0)
        ((eq x 2) 1)
        ((eq x 3) 2)
        ((eq x 4) 3)
        ((eq x 5) 4)
        ((eq x 6) 5)
        ((eq x 7) 6)
        ((eq x 8) 7)
        ((eq x 9) 8)
        ((eq x 0) :f)
        (t (bottom '(not a digit:) x))))

(define *sums-of-digits* '(
  ((0.0) (1.0) (2.0) (3.0) (4.0) (5.0) (6.0) (7.0) (8.0) (9.0) (0.1))
  ((1.0) (2.0) (3.0) (4.0) (5.0) (6.0) (7.0) (8.0) (9.0) (0.1) (1.1))
  ((2.0) (3.0) (4.0) (5.0) (6.0) (7.0) (8.0) (9.0) (0.1) (1.1) (2.1))
  ((3.0) (4.0) (5.0) (6.0) (7.0) (8.0) (9.0) (0.1) (1.1) (2.1) (3.1))
  ((4.0) (5.0) (6.0) (7.0) (8.0) (9.0) (0.1) (1.1) (2.1) (3.1) (4.1))
  ((5.0) (6.0) (7.0) (8.0) (9.0) (0.1) (1.1) (2.1) (3.1) (4.1) (5.1))
  ((6.0) (7.0) (8.0) (9.0) (0.1) (1.1) (2.1) (3.1) (4.1) (5.1) (6.1))
  ((7.0) (8.0) (9.0) (0.1) (1.1) (2.1) (3.1) (4.1) (5.1) (6.1) (7.1))
  ((8.0) (9.0) (0.1) (1.1) (2.1) (3.1) (4.1) (5.1) (6.1) (7.1) (8.1))
  ((9.0) (0.1) (1.1) (2.1) (3.1) (4.1) (5.1) (6.1) (7.1) (8.1) (9.1))
))

(define *diffs-of-digits* '(
  ((0.0) (9.1) (8.1) (7.1) (6.1) (5.1) (4.1) (3.1) (2.1) (1.1) (0.1))
  ((1.0) (0.0) (9.1) (8.1) (7.1) (6.1) (5.1) (4.1) (3.1) (2.1) (1.1))
  ((2.0) (1.0) (0.0) (9.1) (8.1) (7.1) (6.1) (5.1) (4.1) (3.1) (2.1))
  ((3.0) (2.0) (1.0) (0.0) (9.1) (8.1) (7.1) (6.1) (5.1) (4.1) (3.1))
  ((4.0) (3.0) (2.0) (1.0) (0.0) (9.1) (8.1) (7.1) (6.1) (5.1) (4.1))
  ((5.0) (4.0) (3.0) (2.0) (1.0) (0.0) (9.1) (8.1) (7.1) (6.1) (5.1))
  ((6.0) (5.0) (4.0) (3.0) (2.0) (1.0) (0.0) (9.1) (8.1) (7.1) (6.1))
  ((7.0) (6.0) (5.0) (4.0) (3.0) (2.0) (1.0) (0.0) (9.1) (8.1) (7.1))
  ((8.0) (7.0) (6.0) (5.0) (4.0) (3.0) (2.0) (1.0) (0.0) (9.1) (8.1))
  ((9.0) (8.0) (7.0) (6.0) (5.0) (4.0) (3.0) (2.0) (1.0) (0.0) (9.1))
))

(define (%nth-item d lst)
  (cond ((eq d 0) (car lst))
        (t (%nth-item (pred d) (cdr lst)))))

(define %d+
  (let ((%nth-item %nth-item))
    (lambda (a b carry)
      (let ((row (%nth-item b *sums-of-digits*)))
        (cond ((eq carry 1) (%nth-item a (cdr row)))
              (t (%nth-item a row)))))))

(define %d-
  (let ()
    (lambda (a b carry)
      (let ((row (%nth-item a *diffs-of-digits*)))
        (cond ((eq carry 1) (%nth-item b (cdr row)))
              (t (%nth-item b row)))))))

(define (%d< a b)
  (letrec
    ((dless
       (lambda (set)
         (cond ((null set)
                 (bottom '(not digits:) a b))
               ((eq a (car set))
                 (not (eq b (car set))))
               ((eq b (car set)) :f)
               (t (dless (cdr set)))))))
    (dless *digits*)))

(define (natural-p x)
  (letrec
    ((lod-p
       (lambda (x)
         (cond ((null x) :t)
               ((atom x) :f)
               (t (and (digitp (car x))
                       (lod-p (cdr x))))))))
    (and (not (atom x))
         (lod-p x))))

(define n-natural id)

(define (n-normalize x)
  (cond ((null (cdr x)) x)
        ((eq (car x) 0)
          (n-normalize (cdr x)))
        (t x)))

(define n<
  (let ()
    (lambda (a b)
      (letrec
        ((d> (lambda (a b)
               (%d< b a)))
         (lt (lambda (a b r)
               (cond ((and (null a) (null b)) r)
                     ((null a) :t)
                     ((null b) :f)
                     (t (lt (cdr a)
                            (cdr b)
                            (cond ((%d< (car a) (car b)) :t)
                                  ((d> (car a) (car b)) :f)
                                  (t r))))))))
        (lt (reverse a) (reverse b) :f)))))

(define (n> a b) (n< b a))

(define (n<= a b) (eq (n> a b) :f))

(define (n>= a b) (eq (n< a b) :f))

(define (n= a b)
  (equal (n-normalize a)
         (n-normalize b)))

(define n+
  (let ()
    (lambda (a b)
      (letrec
        ((add
           (lambda (a b c r)
             (cond ((null a)
                     (cond
                       ((null b)
                         (cond ((eq c 0) r)  ; no carry
                               (t (cons 1 r))))
                       (t (let ((sum (%d+ 0 (car b) c)))
                            (add ()
                                 (cdr b)
                                 (cdr sum)
                                 (cons (car sum) r))))))
                   ((null b)
                     (let ((sum (%d+ (car a) 0 c)))
                       (add (cdr a)
                            ()
                            (cdr sum)
                            (cons (car sum) r))))
                   (t (let ((sum (%d+ (car a) (car b) c)))
                        (add (cdr a)
                             (cdr b)
                             (cdr sum)
                             (cons (car sum) r))))))))
        (add (reverse a) (reverse b) 0 ())))))

(define n-
  (let ()
    (lambda (a b)
      (letrec
        ((diff
           (lambda (a b c r)
             (cond ((null a)
                     (cond
                       ((null b)
                         (cond ((eq c 0) r)
                               (t (bottom '(negative difference)))))
                       (t (bottom '(negative difference)))))
                   ((null b)
                     (cond ((eq c 0)
                             (append (reverse a) r))
                           (t (diff a '(1) 0 r))))
                   (t (let ((delta (%d- (car a) (car b) c)))
                        (diff (cdr a)
                              (cdr b)
                              (cdr delta)
                              (cons (car delta) r))))))))
        (n-normalize (diff (reverse a) (reverse b) 0 ()))))))

(define (n-zero x)
  (and (eq (car x) 0)
       (null (cdr x))))

(define (n-one x)
  (and (eq (car x) 1)
       (null (cdr x))))

(define (n* a b)
  (letrec
    ((*10
       (lambda (x)
         (append x '#0)))
     (add-n-times
       (lambda (a b r)
         (cond ((n-zero (list b)) r)
               (t (add-n-times a (pred b) (n+ a r))))))
     (times
       (lambda (a b r)
         (cond ((null b) r)
               (t (times (*10 a)
                         (cdr b)
                         (add-n-times a (car b) r)))))))
    (cond ((n-zero a) '#0)
          (t (times a (reverse b) '#0)))))

(define (n-divide a b)
  (letrec
    ; Equalize the divisor B by shifting it to the left
    ; (multiplying it by 10) until it has the same number
    ; of digits as the dividend A.
    ; Return: (new divisor . base 1 shift count)
    ((eql
       (lambda (a b r s)
         (cond ((null a)
                 (cons (reverse r) s))
               ((null b)
                 (eql (cdr a)
                      ()
                      (cons 0 r)
                      (cons 'i s)))
               (t (eql (cdr a)
                       (cdr b)
                       (cons (car b) r)
                       s)))))
     ; Divide with quotient < 10
     ; Return (A/B*B . A/B)
     (div10
       (lambda (a b r)
         (cond ((n< (car r) a)
                 (div10 a b (cons (n+ (car r) b)
                                  (n+ (cdr r) '#1))))
               ((equal (car r) a) r)
               (t (cons (n- (car r) b)
                        (n- (cdr r) '#1))))))
     ; X / 10
     (d10
       (lambda (x)
         (reverse (cdr (reverse x)))))
     (div
       (lambda (a b r)
         (cond ((null (cdr b))
                 (list (n-normalize r) a))
               (t (let ((quot (div10 a (car b) (cons '#0 '#0))))
                    (div (n- a (car quot))
                         (cons (d10 (car b)) (cddr b))
                         (append r (cdr quot)))))))))
    (cond ((n-zero b) (bottom 'divide-by-zero))
          ((n< a b) (list '#0 a))
          (t (div a (eql a b () '#i) '#0)))))

(define (n-quotient a b) (car (n-divide a b)))

(define (n-remainder a b) (cadr (n-divide a b)))

(define (even x)
  (and (memq (car (reverse x)) '#02468) :t))

(define (odd x) (eq (even x) :f))

(define (n-expt x y)
  (letrec
    ((square
       (lambda (x)
         (n* x x)))
     (n-expt1
       (lambda (y)
         (cond ((n-zero y) '#1)
               ((even y)
                 (square (n-expt1 (n-quotient y '#2))))
               (t (n* x (square (n-expt1 (n-quotient y '#2)))))))))
    (n-expt1 (n-natural y))))

(define (n-sqrt square)
  (letrec
    ((sqr
       (lambda (x last)
         (cond ((equal last x) x)
               ((equal last (n+ x '#1))
                 (cond ((n> (n* x x) square) (n- x '#1))
                       (t x)))
               (t (sqr (n-quotient (n+ x (n-quotient square x))
                                   '#2)
                       x))))))
    (sqr square '#0)))

(define (length x)
  (letrec
    ((len (lambda (x r)
            (cond ((null x) r)
                  (t (len (cdr x) (n+ r '#1)))))))
    (len x '#0)))

(define (n-gcd a b)
  (cond ((n-zero b) a)
        ((n-zero a) b)
        ((n< a b) (n-gcd a (n-remainder b a)))
        (t (n-gcd b (n-remainder a b)))))

(define (n-lcm a b)
  (let ((cd (n-gcd a b)))
    (n* cd (n* (n-quotient a cd)
               (n-quotient b cd)))))

(define (limit op a . b)
  (letrec
    ((lim (lambda (a)
            (cond ((null (cdr a)) (car a))
                  ((op (car a) (cadr a))
                    (lim (cons (car a) (cddr a))))
                  (t (lim (cdr a)))))))
    (lim (cons a b))))

(define (n-max . a) (apply limit n> a))

(define (n-min . a) (apply limit n< a))

(require 'iter)

(define natural n-natural)

(define * (arithmetic-iterator n-natural n* '#1))

(define + (arithmetic-iterator n-natural n+ '#0))

(define (- . x)
  (cond ((or (null x) (null (cdr x)))
          (bottom '(too few arguments to n-natural -)))
        (t (fold (lambda (a b)
                   (n- (n-natural a) (n-natural b)))
                 (car x)
                 (cdr x)))))

(define < (predicate-iterator natural n<))

(define <= (predicate-iterator natural n<=))

(define = (predicate-iterator natural n=))

(define > (predicate-iterator natural n>))

(define >= (predicate-iterator natural n>=))

(define divide n-divide)

(define expt n-expt)

(define gcd (arithmetic-iterator natural n-gcd '#0))

(define lcm (arithmetic-iterator natural n-lcm '#1))

(define max n-max)

(define min n-min)

(define number-p natural-p)

(define one n-one)

(define quotient n-quotient)

(define remainder n-remainder)

(define sqrt n-sqrt)

(define zero n-zero)
