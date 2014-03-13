#   Problem : Textbook problem used in DAKOTA testing
#             Unconstrained quartic, 2 continuous variables
#   Solution: x=(1, 1), obj = 0
#

# continuous variables
var x1 >=  0.5 <= 5.8 := 0.9;
var x2 >= -2.9 <= 2.9 := 1.1;

# objective function
minimize obj: (x1 - 1)^4 + (x2 - 1)^4;

# required for output of *.row and *.col files
option auxfiles rc;
