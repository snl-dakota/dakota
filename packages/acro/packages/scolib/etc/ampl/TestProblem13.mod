#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Quadratic with partial bound constraints
#   Solution: 0.0 3.0 0.0


param N := 3;

var x{i in 1..N} <= 30.0 := 9.333 + i;

minimize obj: sum{i in 1..N} x[i]*x[i];

s.t. c1:
	x[2] >= 3.0;
