; M-Expression Example Program
; By Nils M Holm, 2004-2007
; See the file LICENSE of the zenlisp distribution
; for conditions of use.

; Solve the Towers of Hanoi.

(require 'mexprc)

(mexpr-eval '(

  m_hanoi[n] :=
    solve[%LEFT, %MIDDLE, %RIGHT, n]
    where
      solve[from, to, via, n] :=
        [n=0
           -> nil:
         solve[from, via, to, n-1]
           ++ list[ list[from, to] ]
           ++ solve[via, to, from, n-1]]

))
