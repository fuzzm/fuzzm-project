; zenlisp example program
; By Nils M Holm, 2007
; See the file LICENSE for conditions of use.

; Compile and match regular expressions of the following form:
; _          match any character
; [char...]  match character class (may contain ranges like c1-c2)
; ^          match beginning of line
; $          match end of line
; *          match zero or more instances of the preceding pattern
; +          match one or more instances of the preceding pattern
; ?          match the preceding pattern optionally
; \c         match c, even if it is special
;
; (re-match (re-compile '#[a-z]*) '#12test34) => '#test
; (re-match (re-compile '#^[a-z]*$) '#12test34) => :f

; __ means not a valid symbol
(define character-set
  '(__ !  "  __ $  %  &  __ __ __ *  +  ,  -  __ /
    0  1  2  3  4  5  6  7  8  9  :  __ <  =  >  ?
    @  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
    p  q  r  s  t  u  v  w  x  y  z  [  \  ]  ^  _
    `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
    p  q  r  s  t  u  v  w  x  y  z  __ |  __ ~  __))

(define (pair-p x) (not (atom x)))

(define (before-p c0 c1)
  (letrec
    ((lt (lambda (set)
           (cond ((null set) (bottom (list before-b c0 c1)))
                 ((eq c1 (car set)) :f)
                 ((eq c0 (car set)) :t)
                 (t (lt (cdr set)))))))
    (lt character-set)))

(define (make-range c0 cn cls)
  (letrec
    ((make
       (lambda (c cls)
         (cond ((null c)
                 (bottom 'invalid-symbol-code cn))
               ((eq (car c) cn)
                 (cons (car c) cls))
               (t (make (cdr c)
                        (cons (car c) cls)))))))
    (let ((c (memq c0 character-set)))
      (cond (c (make c cls))
            (t (bottom 'invalid-symbol-code c0))))))

(define (compile-class in out cls first)
  (cond
    ((null in) :f)
    ((eq '] (car in))
      (list (cdr in) (cons (reverse cls) out)))
    ((and first (eq '^ (car in)))
      (compile-class (cdr in) out '#] :f))
    ((and (not first)
          (not (null (cdr cls)))
          (eq '- (car in))
          (pair-p (cdr in))
          (not (eq '] (cadr in))))
      (let ((c0 (car cls))
            (cn (cadr in)))
        (cond
          ((before-p c0 cn)
            (compile-class (cddr in)
                           out
                           (make-range c0 cn (cdr cls)) :f))
            (t (compile-class (cdr in)
                              out
                              (cons '- cls) :f)))))
    (t (compile-class (cdr in)
                      out
                      (cons (car in) cls) :f))))

(define (re-compile re)
  (letrec
     ((compile
       (lambda (in out)
         (cond
           ((not in) :f)
           ((null in) (reverse out))
           (t (cond
                ((eq (car in) '\)
                  (cond ((pair-p (cdr in))
                          (compile (cddr in)
                                   (cons (cadr in) out)))
                        (t :f)))
                ((memq (car in) '#^$_)
                  (compile (cdr in)
                           (cons (list (car in)) out)))
                ((memq (car in) '#*?)
                  (compile (cdr in)
                           (cond ((null out)
                                   (cons (car in) out))
                                 (t (cons (list (car in) (car out))
                                          (cdr out))))))
                ((eq (car in) '+)
                  (compile (cdr in)
                           (cond ((null out)
                                   (cons (car in) out))
                                 (t (cons (list '* (car out)) out)))))
                ((eq (car in) '[)
                  (apply compile
                         (compile-class (cdr in) out '#[ :t)))
                (t (compile (cdr in)
                            (cons (car in) out)))))))))
    (compile re ())))

(define (match-char p c)
  (cond ((eq '_ p)
          :t)
        ((atom p)
          (eq p c))
        ((eq '[ (car p))
          (and (memq c (cdr p)) :t))
        ((eq '] (car p))
          (not (memq c (cdr p))))
        (t :f)))

(define (make-choices p s m)
  (cond
    ((or (null s)
         (not (match-char (cadar p) (car s))))
      (list (list s m)))
    (t (cons (list s m)
             (make-choices p (cdr s) (cons (car s) m))))))

(define (match-star cre s m)
  (letrec
    ((try-choices
       (lambda (c*)
         (cond ((null c*) :f)
               (t (let ((r (match-cre (cdr cre) (caar c*) (cadar c*))))
                    (cond (r (append (reverse m) r))
                          (t (try-choices (cdr c*))))))))))
    (try-choices (reverse (make-choices cre s ())))))

(define (match-cre cre s m)
  (cond
    ((null cre)
      (reverse m))
    ((null s)
      (cond ((equal cre '(#$))
              (match-cre () () m))
            ((and (pair-p (car cre))
                  (eq '* (caar cre))
                  (null (cdr cre)))
              ())
            (t :f)))
    ((pair-p (car cre))
      (cond
        ((eq '* (caar cre))
          (match-star cre s m))
        ((eq '? (caar cre))
          (cond ((match-char (cadar cre) (car s))
                  (match-cre (cdr cre) (cdr s) (cons (car s) m)))
                (t (match-cre (cdr cre) s m))))
        ((match-char (car cre) (car s))
          (match-cre (cdr cre) (cdr s) (cons (car s) m)))
        (t :f)))
    ((eq (car cre) (car s))
      (match-cre (cdr cre) (cdr s) (cons (car s) m)))
    (t :f)))

(define (try-matches cre s)
  (cond ((null s) (match-cre cre s ()))
        (t (let ((r (match-cre cre s ())))
             (cond ((or (not r) (null r))
                     (try-matches cre (cdr s)))
                   (t r))))))

(define (re-match cre s)
  (cond ((and (pair-p cre) (equal '#^ (car cre)))
          (match-cre (cdr cre) s ()))
        (t (try-matches cre s))))
