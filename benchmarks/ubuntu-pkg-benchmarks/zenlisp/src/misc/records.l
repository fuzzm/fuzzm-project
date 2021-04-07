; zenlisp example program
; By Nils M Holm, 2007
; See the file LICENSE for conditions of use.

; Emulate the ML-style record datatype
; without mutation and with only rudimentary type checking.
; (record '(a test) '(b #17) (c (a list)))
;   => '((%record) (a test) (b #17) (c (a list)))
; (define r **)
; (record-ref r 'b) => '#17
; (record-equal r r) => :t
; (record-signature r) => '((%record) (a atom) (b number) (c pair))
; (record-set r 'b '#25) => '((%record) (a test) (b #25) (c (a list)))
; (record-set r 'b 'xyz) => bottom ; type check

(or (defined 'nmath)
    (defined 'imath)
    (defined 'rmath)
    (load ~rmath))

(define record-tag (list '%record))

(define (pair-p x) (not (atom x)))

(define (boolean-p x)
  (or (eq x :t)
      (eq x :f)))

(define (closure-p x)
  (and (pair-p x)
       (eq (car x) 'closure)))

(define (record-p x)
  (and (pair-p x)
       (eq (car x) record-tag)))

(define (list->record a)
  (letrec
    ((valid-fields-p
       (lambda (a)
         (or (null a)
             (and (pair-p (car a))
                  (atom (caar a))
                  (pair-p (cdar a))
                  (null (cddar a))
                  (valid-fields-p (cdr a)))))))
    (cond ((valid-fields-p a) (cons record-tag a))
          (t (bottom 'bad-record-structure a)))))

(define (record . x) (list->record x))

(define (record->list r)
  (cond ((record-p r) (cdr r))
        (t (bottom 'expected-record-got r))))

(define (record-field r tag)
  (let ((v (assq tag (record->list r))))
    (cond (v v)
          (t (bottom 'no-such-tag
                     (list 'record: r 'tag: tag))))))

(define (record-ref r tag) (cadr (record-field r tag)))

(define (type-of x)
  (cond ((boolean-p x)  'boolean)
        ((null x)       'pair)
        ((atom x)       'symbol)
        ((number-p x)   'number)
        ((record-p x)   'record)
        ((closure-p x)  'function)
        ((pair-p x)     'pair)
        (t (bottom 'unknown-type x))))

(define (record-equal r1 r2)
  (letrec
    ((equal-fields-p
       (lambda (r1 r2)
         (cond ((null r1) :t)
               (t (let ((x (assq (caar r1) r2)))
                    (and x
                         (equal (cadar r1) (cadr x))
                         (equal-fields-p (cdr r1) r2))))))))
    (let ((lr1 (record->list r1))
          (lr2 (record->list r2)))
      (and (= (length lr1) (length lr2))
           (equal-fields-p lr1 lr2)))))

(define (equal a b)
  (cond ((eq a b) :t)
        ((and (pair-p a) (pair-p b))
          (and (equal (car a) (car b))
               (equal (cdr a) (cdr b))))
        ((record-p a)
          (and (record-p b)
               (record-equal a b)))
        (t :f)))

(define (record-signature r)
  (letrec
    ((make-sig
       (lambda (x)
         (map (lambda (x)
                (cond ((record-p (cadr x))
                        (list (car x)
                              (list (type-of (cadr x))
                                    (record-signature (cadr x)))))
                      (t (list (car x) (type-of (cadr x))))))
              x))))
    (list->record (make-sig (record->list r)))))

(define (record-set r tag v)
  (letrec
    ((subst
       (lambda (r old new)
         (cond ((null r) ())
               ((eq old (car r))
                 (cons new (cdr r)))
               (t (cons (car r)
                        (subst (cdr r) old new))))))
     (type-mismatch
       (lambda ()
         (bottom 'type-mismatch
                 (list 'record: r 'tag: tag 'value: v)))))
    (let ((f (record-field r tag)))
      (let ((b (cdr f)))
        (cond ((eq (type-of (car b)) (type-of v))
                (cond ((or (not (record-p v))
                           (record-equal
                             (record-signature (car b))
                             (record-signature v)))
                        (subst r f (list (car f) v)))
                      (t (type-mismatch))))
                (t (type-mismatch)))))))

(define (record-type-matches-p sig r)
  (record-equal sig (record-signature r)))

(define (assert-record-type sig r)
  (cond ((not (record-type-matches-p sig r))
          (bottom 'record-type-assertion-failed
                  (list 'signature: sig 'record: r)))
        (t r)))
