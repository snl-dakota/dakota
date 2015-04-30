#**************************
# SET UP THE INITIAL DATA *
#**************************
#   Problem : MINLP function
#   Solution: 


var x1 := 0;
var x2 >= -10 <= 10 := 1;
var y1 integer >= -20 <= 20 := 5;
var y2 binary := 0;
var y3 integer >= 0 <= 2 := 4;

minimize obj: (x1-1)^2 + (x2-1.5)^2 + (y1+1)^2 + 100*y2 + 50*y3;

subject to c1: x1^2 + y1^2 + x2 <= 100;
subject to c2: x2 + 10*y2 == 10;
subject to c3: -x1 + 5*y3 <= 10;