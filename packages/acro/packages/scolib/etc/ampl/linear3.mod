#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : 9-D linear function with binary, integer and real values
#   Solution: 9.0


param N := 3;

var z{j in 1..N}, integer >= 1 <= 3 := 3;
var y{k in 1..N}, binary;
var x{i in 1..N} >= 1.0 <= 3.0 := 2.9;

minimize obj: sum{i in 1..N} (x[i] + y[i] + z[i]);
