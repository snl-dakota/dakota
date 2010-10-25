#   Problem : Textbook problem used in DAKOTA testing
#             Unconstrained quartic, 3 continuous, 2 discrete variables
#   Solution: x=(1, 1, 1), y=(1 1), obj = 0
#

# Continuous variables
var x1 >= -10.0 <= 10.0 := 0.5;
var x2 >= -10.0 <= 10.0 := 1.5;
var x3 >= -10.0 <= 10.0 := 2.0;

# Discrete variables
var y1 integer >= 1 <= 4 := 2;
var y2 integer >= 1 <= 9 := 2;

minimize obj: (x1 - 1)^4 + (x2 - 1)^4 + (x3 - 1)^4 + 
              (y1 - 1)^4 + (y2 - 1)^4;

