; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Bottles Song in Pure LISP
;
; This is purely symbolic lisp.
; There are no pre-defined numbers, not even digits.
; There are no strings.
; There is no input/output.
; There are no side effects.
;
; Numbers are represented by lists of digits.
;
; Lyrics are generated in this format:
;
; ((#99 bottles of beer on the wall)
;  (#99 bottles of beer)
;  (take one down and pass it around)
;  (#98 bottles of beer on the wall))

(define (bottles)
  (letrec
    ((beer-on-wall
      (quote (bottles of beer on the wall)))
     (beer
       (quote (bottles of beer)))
     (take-down
       (quote (take one down and pass it around)))
     (append
       (lambda (a b)
         (cond ((null a) b)
               (t (cons (car a) (append (cdr a) b))))))
     (append4
       (lambda (a b c d)
         (append a (append b (append c d)))))
     (pred (lambda (n)
       (cond ((eq n (quote 1)) (quote 0))
             ((eq n (quote 2)) (quote 1))
             ((eq n (quote 3)) (quote 2))
             ((eq n (quote 4)) (quote 3))
             ((eq n (quote 5)) (quote 4))
             ((eq n (quote 6)) (quote 5))
             ((eq n (quote 7)) (quote 6))
             ((eq n (quote 8)) (quote 7))
             ((eq n (quote 9)) (quote 8))
             (t :f))))
     (cadr
       (lambda (x)
         (car (cdr x))))
     (list
       (lambda (x)
         (cons x ())))
     (decrement
       (lambda (n)
         (cond ((pred (cadr n))
                 (cons (car n) (list (pred (cadr n)))))
               (t (cons (pred (car n)) (list (quote 9)))))))
     (zerop
       (lambda (n)
         (cond ((eq (car n) (quote 0))
                 (eq (cadr n) (quote 0)))
               (t :f))))
     (verse
       (lambda (n)
         (append4
           (list (append (list n) beer-on-wall))
           (list (append (list n) beer))
           (list take-down)
           (list (append (list (decrement n)) beer-on-wall)))))
     (count-bottles
       (lambda (n lyrics)
         (cond ((zerop n) lyrics)
               (t (count-bottles
                    (decrement n)
                    (append lyrics (list (verse n)))))))))
    (count-bottles (quote (9 9)) ())))
