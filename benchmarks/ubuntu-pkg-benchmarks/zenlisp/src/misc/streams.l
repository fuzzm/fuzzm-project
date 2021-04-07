; zenlisp example program
; By Nils M Holm, 1998-2008
; See the file LICENSE for conditions of use.

(define (stream v first filter rest lim final)
  (letrec
    ((find
      (lambda (x)
        (cond ((lim x) x)
              ((filter (first x)) x)
              (t (find (rest x))))))
     (make-stream
       (lambda (v)
         (lambda ()
           (let ((nf (find v)))
             (cond ((lim nf) final)
                   (t (cons (first nf)
                            (make-stream (rest nf))))))))))
    ((make-stream v))))

(define (all x) :t)

(define (none x) :f)

(define (value s) (car s))

(define (next s) ((cdr s)))

(define pass not)

(define (list->stream v)
  (stream v car all cdr null :f))

(define (stream->list s)
  (letrec
    ((s->l
       (lambda (s lst)
         (cond (s (s->l (next s)
                        (cons (value s) lst)))
               (t (reverse lst))))))
    (s->l s ())))

(define (stream-member p s d)
  (cond ((eq s d) d)
        ((p (value s)) s)
        (t (stream-member p (next s) d))))

(define (map-stream f s)
  (stream s (lambda (s) (f (value s))) all next pass :f))

(define (filter-stream p s)
  (stream s value p next pass :f))

(define (append-streams s1 s2)
  (stream s1 value all next pass s2))
