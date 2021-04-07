; M-Expression Example Program
; By Nils M Holm, 2004-2007
; See the file LICENSE of the zenlisp distribution
; for conditions of use.

; Append two lists.

(require '~mexprc)

(mexpr-eval '(

  m_append[a,b] :=
    r_append[reverse[a], b]
    where
      r_append[a,b] :=
        [null[a]
           -> b:
         r_append[cdr[a], car[a]::b]]

))
