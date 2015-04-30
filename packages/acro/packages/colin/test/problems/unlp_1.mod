#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : 2-D nonlinear function
#   Solution: 1, 1


param N := 2;

var x{i in 1..N} >= 1.0 <= 2.0 := 1.9;

minimize obj: prod{i in 1..N} x[i];
