#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : 1-D linear function
#   Solution: -2.0


param N := 1;

var x{i in 1..N} >= -2.0 <= -1.0 := -1.1;

minimize obj: sum{i in 1..N} x[i];
