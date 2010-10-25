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

function p19

  t = automatic_replot;
  automatic_replot = 0;

  f0=figure;
  multiplot(1,2);

  title("Stripchard demo 1 (sliding)");
  xlabel("Time");
  ylabel("Sliding");

  legend("on", 0.75,0.75);

  id1 = stripc(0, 10, 0.3, -1, 1, "sum", "sin", "sin*noi", "sin+noi", 0, 0);

  subwindow(1,2);

  title("Stripchard demo 1 (accumulating)");
  xlabel("Time");
  ylabel("Accum");

  legend("on", 0.75,0.75);

  id2 = stripc(0, 10, 0.3, -1, 1, "sum", "sin", "sin*noi", "sin+noi", 1, 0);

  subwindow(1,1);

  f1=figure(free_fig);

  oneplot;

  title("Stripchard demo 2 (acumulating)");
  xlabel("Time");
  ylabel("Acumulating");

  legend("on", 0, 1);

  id3 = stripc(0, 10, 0.3, -1, 1, "sin+noise", "", "", "", 1, 0);

  figure(f0);

  y1 = 0.0;

  for t = 0:0.1:10*pi
    noise = randn;
    y1 = y1 + noise/10;
    y2 = sin(t);
    y3 = y2 * noise/3;
    y4 = y2 + noise/4;
    
    ## there is no need for all pens to have the same number of points
    ## or beeing equally time spaced.

    if (rem(t,2))	
      stripc_add(id1, 0, t, y1);
    endif
    if rem(t,3)
      stripc_add(id1, 1, t, y2);
    endif

    if rem(t,4)
      subwindow(1,2);
      stripc_add(id2, 2, t, y3);
      subwindow(1,1);
    endif

    if rem(t,5)
      figure(f1);
      stripc_add(id3, 0, t, y4);
      plflush;pleop;
      figure(f0);
    endif
    
    plflush;pleop;
  endfor

  ## Destroy strip chart and it's memory */

  stripc_del(id1);
  stripc_del(id2);
  stripc_del(id3);
  
  closefig(f1);
  oneplot;

  automatic_replot = t;

endfunction
