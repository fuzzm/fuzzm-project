; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Factor integers down to their constituent primes:
; (factors '#24) => '((#3 #1) (#2 #3))

(require '~nmath)

(define (factors n)
  (letrec
    ((quotient+exponent
       (lambda (n m)
         (letrec
           ((div (lambda (n m r)
                   (let ((qr (divide n m)))
                     (cond ((zero (cadr qr))
                             (div (car qr) m (+ '#1 r)))
                           (t (cons n r)))))))
           (div n m '#0))))
     (add-expt
       (lambda (b e r)
         (cond ((zero e) r)
               (t (cons (list b e) r)))))
     (factorize
       (lambda (n d r)
         (let ((lim (sqrt n)))
           (letrec
             ((factorize3
                (lambda (n d r)
                  (let ((rest/exp (quotient+exponent n d)))
                    (let ((q (car rest/exp)) 
                          (e (cdr rest/exp)))
                      (cond
                        ((< q '#2) (add-expt d e r))
                        ((> d lim) (add-expt n '#1 r))
                        (t (factorize3
                             q
                             (cond ((= d '#2) '#3)
                                   (t  (+ d '#2)))
                             (add-expt d e r)))))))))
             (factorize3 n d r))))))
    (cond
      ((< n '#1) (bottom 'operand-not-positive n))
      ((= n '#1) '#1)
      (t (factorize n '#2 ())))))
