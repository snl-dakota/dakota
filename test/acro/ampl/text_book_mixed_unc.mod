#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Textbook problem used in DAKOTA/COLINY testing
#             Unconstrained quartic, 3 continuous, 2 discrete variables
#   Solution: x=(1, 1, 1), y=(1 1), obj = 0
#

param N := 3;
param M := 2;

var x{i in 1..N} >= -10.0 <= 10.0 := i - 0.5;
var y{j in 1..M} integer >= 1 <= 4*j := 2;

minimize obj: sum{i in 1..N} (x[i]-1)^4 + sum{j in 1..M} (y[j]-1)^4;

#DAKOTA - added
options nl_comments 2, auxfiles rc;
