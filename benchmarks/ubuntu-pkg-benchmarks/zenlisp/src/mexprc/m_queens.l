; M-Expression Example Program
; By Nils M Holm, 2004-2007
; See the file LICENSE of the zenlisp distribution
; for conditions of use.

; Solve the N-Queens puzzle.

(require 'mexprc)

(mexpr-eval '(

  m_queens[size] :=
    n_queens[0, 0, nil]

    where n_queens[q, c, b] :=
      [c = size
         -> reverse[b]:
       column[q] <> c
         -> [null[b]
               -> nil:
             n_queens[car[b]+1, c-1, cdr[b]]]:
       safe_place[q, b]
         -> n_queens[next_column[q], c+1, q::b]:
       n_queens[q+1, c, b]]

    and column[x] := x // size

    and row[x] := x \\ size

    and safe_place[x,b] :=
      [null[b]
         -> true:
       connected[car[b], x]
         -> false:
       safe_place[x, cdr[b]]]

    and connected[x,y] :=
      common_h_v[x,y] \/ common_dia[x,y]

    and common_h_v[x,y] :=
      row[x] = row[y] \/ column[x] = column[y]

    and common_dia[x,y] :=
      abs[column[x]-column[y]] = abs[row[x]-row[y]]

    and next_column[q] := [q+size] // size * size

))

