#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : A function whose values are always improving
#   Solution: Any


#
# This links to a function that simply returns the number of function evals
#
function ncall;

param N := 3;

var x{i in 1..N} >= 0.0 <= 10.0 := i + 1.0;

minimize obj: -ncall({i in 1..N} x[i]);
