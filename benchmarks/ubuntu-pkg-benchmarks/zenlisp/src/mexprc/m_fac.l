; M-Expression Example Program
; By Nils M Holm, 2004-2007
; See the file LICENSE of the zenlisp distribution
; for conditions of use.

; Compute the factorial of an integer.

(require '~mexprc)

(mexpr-eval '(

  m_fac[x] := [x=0 -> 1 : m_fac[x-1] * x]

))
