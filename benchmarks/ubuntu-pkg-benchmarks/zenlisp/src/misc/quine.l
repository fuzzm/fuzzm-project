; zenlisp example program
; By Nils M Holm, 1998-2007
; See the file LICENSE for conditions of use.

; A zenlisp quine.

((lambda #x (list x (list 'quote x)))
'(lambda #x (list x (list 'quote x))))
