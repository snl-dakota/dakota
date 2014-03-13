#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Quadratic with nearly-binding bound constraints
#   Solution: 0.0 in each dimension


param N := 20;

var x{i in 1..N} >= -0.01 <= 30.0 := 30.0*(1.0-1.0/sqrt(i+1.0));

minimize obj: sum{i in 1..N} x[i]*x[i];

#DAKOTA - added
options nl_comments 2, auxfiles rc;
