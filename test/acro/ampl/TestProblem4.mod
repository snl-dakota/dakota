#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Constrained quadratic with partial lower bound constraints
#   Solution: 2-sqrt(2) 1.0 0.0


param N := 3;

var x{i in 1..N} <= 30.0 := 9.333 + i;
#var x{i in 1..N} >= if (i==2) then 1.0 else -Infinity <= 30.0 := 9.333 + i;

minimize obj: sum{i in 1..N} x[i]*x[i];

#DAKOTA - changed
minimize c1: x[1]^2 + (x[2]-2)^2 + x[3]^2;
minimize c2: (x[1]-2)^2 + x[2]^2 + x[3]^2;
#minimize lb1: x[2];
#DAKOTA - original
#s.t. c1: x[1]^2 + (x[2]-2)^2 + x[3]^2 <= 3.0;
#s.t. c2: (x[1]-2)^2 + x[2]^2 + x[3]^2 <= 3.0;
#s.t. lb1: x[2] >= 1.0;

#DAKOTA - added
options nl_comments 2, auxfiles rc;
