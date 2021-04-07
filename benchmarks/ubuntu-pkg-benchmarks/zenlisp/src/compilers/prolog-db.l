; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

(define *database* '(
  ((female cathy))
  ((female denise))
  ((male eric))
  ((female fanny))

  ((parent bertram eric))     ; Parent relationship facts
  ((parent cathy eric))
  ((parent anthony cathy))
  ((parent eric denise))
  ((parent anthony fanny))

  ((mother (? a) (? b))       ; A is mother of B if
    (female (? a))            ;  A is female and
    (parent (? a) (? b)))     ;  A is parent of B

  ((father (? a) (? b))       ; A is father of B if
    (male (? a))              ;  A is male and
    (parent (? a) (? b)))     ;  A is parent of B

  ((wife (? a) (? b))         ; A is (often) wife of B if
    (mother (? a) (? x))      ;  A is mother of X and
    (father (? b) (? x)))     ;  B is father of X

  ((child (? a) (? b))        ; A is child of B if
    (parent (? b) (? a)))     ;  B is parent of A

  ((descendant (? a) (? b))   ; A is descendant of B if
    (child (? a) (? b)))      ;  A is child of B

  ((descendant (? a) (? b))   ; or if
    (child (? a) (? x))       ;  A is child of X and
    (descendant (? x) (? b))) ;  X is descendant of B
))

; Sample queries:
; Make the output of QUERY visible:
; (trace print)
; (query '(father anthony (? child)))   ; whose father is Anthony?
; (query '(parent (? parent) eric))     ; who are Eric's parents?
; (query '(descendant (? descendant) bertram))  ; list descendants of Bertram.
; (query '(wife (? wife) (? husband)))  ; who is who's wife?
; (query '((? relation) cathy (? person)))  ; which relations does Cathy have?
