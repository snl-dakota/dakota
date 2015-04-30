#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Bound constrained quadratic.
#   Solution: 3.0 3.0 3.0


param N := 3;

var x{i in 1..N} >= 3.0 <= 30.0 := 9.333 + i;

minimize obj: sum{i in 1..N} x[i]*x[i];
