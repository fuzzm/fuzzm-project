; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Transform a list into a set (a list containing
; only unique elements):
; (list->set '(aa ab ac ad ab aa)) => '(aa ab ac ad)

(define (list->set a)
  (letrec
    ((l->s
       (lambda (a r)
         (cond ((null a)
                 (reverse r))
               ((member (car a) r)
                 (l->s (cdr a) r))
               (t (l->s (cdr a) (cons (car a) r)))))))
    (l->s a ())))
