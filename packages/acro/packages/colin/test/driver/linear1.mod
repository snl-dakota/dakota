#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : 1-D linear function
#   Solution: 1.0


param N := 1;

var x{i in 1..N} >= 1.0 <= 2.0 := 1.9;

minimize obj: sum{i in 1..N} x[i];
