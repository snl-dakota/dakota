#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Textbook problem used in DAKOTA/COLINY testing
#             Constrained quartic, 3 continuous variables
#   Solution: x=(0.5, 0.5, 1), obj = .125, c1 = 0, c2 = 0
#

param N := 3;

var x{i in 1..N} >= -10.0 <= 10.0 := i - 0.5;

minimize obj: sum{i in 1..N} (x[i]-1)^4;

subject to c1: x[1]^2 - 0.5*x[2] <= 0;

subject to c2: x[2]^2 - 0.5*x[1] <= 0;

#DAKOTA - added
options nl_comments 2, auxfiles rc;
