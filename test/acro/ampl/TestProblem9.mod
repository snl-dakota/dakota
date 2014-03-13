#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Constrained quadratic, with nonlinear inequality constraints + 
#			nonlinear equality constraint and nonbinding bounds
#   Solution: 2/3 2/3 2/3


param N := 3;

var x{i in 1..N} >= 0.0 <= 0.7 := 0.4;

minimize obj: sum{i in 1..N} x[i]*x[i];

#DAKOTA - changed
minimize c1: x[1]^2 + (x[2]-2)^2 + x[3]^2;
minimize c2: (x[1]-2)^2 + x[2]^2 + x[3]^2;
minimize c3: x[1] + x[2] + x[3];
#DAKOTA - original
#s.t. c1: x[1]^2 + (x[2]-2)^2 + x[3]^2 <= 3.0;
#s.t. c2: (x[1]-2)^2 + x[2]^2 + x[3]^2 <= 3.0;
#s.t. c3: x[1] + x[2] + x[3] = 2.0;

#DAKOTA - added
options nl_comments 2, auxfiles rc;
