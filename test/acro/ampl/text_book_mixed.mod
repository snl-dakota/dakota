#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Textbook problem used in DAKOTA/COLINY testing
#             Constrained quartic, 3 continuous, 2 discrete variables
#   Solution: x=(0.5, 0.5, 1) y=(1 1), obj = .125, c1 = 0, c2 = 0
#

param N := 3;
param M := 2;

var x{i in 1..N} >= -10.0 <= 10.0 := i - 0.5;
var y{j in 1..M} integer >= 1 <= 4*j := 2;

minimize obj: sum{i in 1..N} (x[i]-1)^4 + sum{j in 1..M} (y[j]-1)^4;

minimize c1: x[1]^2 - 0.5*x[2];

minimize c2: x[2]^2 - 0.5*x[1];

#DAKOTA - added
options nl_comments 2, auxfiles rc;
