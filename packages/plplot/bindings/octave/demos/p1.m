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


function p1
    
  t = automatic_replot;
  automatic_replot = 0;

  xscale = 6.;
  yscale = 1.;
  xoff = 0.;
  yoff = 0.;

  m=60;
  x = xoff + xscale * ((1:m) + 1) / m;
  y = yoff + yscale * (x.^2);

  n=5;
  xs = x((1:n) * 10 + 3);
  ys = y((1:n) * 10 + 3);

  ## Plot the data points */

  xlabel("(x)");
  ylabel("(y)");
  title("#frPLplot Example 1 - y=x#u2");

  plot(x, y, xs, ys, 'og; ;');

  automatic_replot = t;
  
endfunction

