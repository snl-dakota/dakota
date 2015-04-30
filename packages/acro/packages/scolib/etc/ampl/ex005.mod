# ex005.mod
# Original AMPL coding by Sven Leyffer, Argonne Natl. Lab.
#
# A simple multi-objective optimization problem (p. 281):
# C.-L. Hwang & A. S. Md. Masud, Multiple Objective
# Decision Making - Methods and Applications, No. 164 in 
# Lecture Notes in Economics and Mathematical Systems,
# Springer, 1979.

# ... variables
var x1 >= -1, <= 2;
var x2 >=  1, <= 2;

# ... objective functions
minimize  f1: x1^2 - x2^2;
minimize  f2: x1/x2;
