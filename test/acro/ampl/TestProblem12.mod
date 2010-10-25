#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Linear + sinusoidal + bound constraints
#   Solution: 10.8954 in each dimension


param N := 10;

var x{i in 1..N} >= 10.0 <= 50.0 := 10.0;

minimize obj: sum{i in 1..N} (x[i] + 10.0 * sin( x[i] )+1.0);

#DAKOTA - added
options nl_comments 2, auxfiles rc;
