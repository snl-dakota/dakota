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

s.t. c1: x[1]^2 - 0.5*x[2] <= 0.0;

s.t. c2: x[2]^2 - 0.5*x[1] <= 0.0;

