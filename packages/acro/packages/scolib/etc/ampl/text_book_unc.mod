#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Textbook problem used in DAKOTA/COLINY testing
#             Unconstrained quartic, 3 continuous variables
#   Solution: x=(1, 1, 1), obj = 0
#

param N := 3;

var x{i in 1..N} >= -10.0 <= 10.0 := i - 0.5;

minimize obj: sum{i in 1..N} (x[i]-1)^4;

#s.t. c1: x[1]^2 - 0.5*x[2] <= 0.0;

#s.t. c2: x[2]^2 - 0.5*x[1] <= 0.0;

