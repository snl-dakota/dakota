#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Constrained quadratic.
#   Solution: 1-1/sqrt(2) 1-1/sqrt(2) 1.0


param N := 3;

var x{i in 1..N} := 9.333 + i;

minimize obj: sum{i in 1..N} x[i]*x[i];

s.t. c1: x[1]^2 + (x[2]-2)^2 + x[3]^2 <= 3.0;

s.t. c2: (x[1]-2)^2 + x[2]^2 + x[3]^2 <= 3.0;
