#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : A constant function.
#   Solution: Any


function constfn;

param N := 3;

var x{i in 1..N} := i + 1.0;

minimize obj: constfn({i in 1..N} x[i]);
