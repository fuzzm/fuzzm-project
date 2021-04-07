; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; Extract the tail of a list starting at the n'th
; member of that list:
; (nth '#2 '#abcdef) => '#cdef

(require '~nmath)

(define (nth n x)
  (cond ((zero n) x)
        ((null x) :f)
        (t (nth (- n '#1)
                (cdr x)))))
