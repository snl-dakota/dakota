#
#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : Linear + sinusoidal product + bound constraints


param pi := 4*atan(1);
param N := 2;

var x{i in 1..N} >= 10.0 <= 20.0 := 15.0;

minimize obj: -(sum{i in 1..N} (x[i]-0.5))* abs(prod{i in 1..N} (cos(2*pi*x[i])-1.0)/2.0);
