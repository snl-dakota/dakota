## Copyright (C) 1998-2003 Joao Cardoso.
## 
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2 of the License, or (at your
## option) any later version.
## 
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## This file is part of plplot_octave.

function p11

  t = automatic_replot;
  automatic_replot = 0;

  title("Stripchart demo 1 (sliding)");
  xlabel("Time");
  ylabel("Sliding");

  legend("on", 0.75,0.75);

  id1 = stripc(0, 10, 0.3, -1, 1, "sin", "cos", "sin+noi", "cos+noi", 0, 0);

  y1 = 0.0;

  for t = 0:0.1:5*pi
    for i=1:1000;endfor    # small delay
    noise = randn;
    y1 = sin(t);
    y2 = cos(t);
    y3 = 1+y1 + noise/3;
    y4 = -1+y2 + noise/4;
    
    ## there is no need for all pens to have the same number of points
    ## or beeing equally time spaced.

    stripc_add(id1, 0, t, y1);
    stripc_add(id1, 1, t, y2);
    stripc_add(id1, 2, t, y3);
    stripc_add(id1, 3, t, y4);			
    pleop;	
  endfor

  ## Destroy strip chart and it's memory */

  stripc_del(id1);
  automatic_replot = t; 

endfunction
