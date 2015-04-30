#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : A constant function with bound constraints
#   Solution: Any


function constfn;

param N := 3;

var x {i in 1..N} >= -1.0 <= 30.0 := 1.333 + i;

minimize obj: constfn({i in 1..N} x[i]);
