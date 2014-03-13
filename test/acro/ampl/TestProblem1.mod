#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Unconstrained quadratic.
#   Solution: 0.0 0.0 0.0


param N := 3;

var x{i in 1..N} := 9.333 + i;

minimize obj: sum{i in 1..N} x[i]*x[i];

#DAKOTA - added
options nl_comments 2, auxfiles rc;
