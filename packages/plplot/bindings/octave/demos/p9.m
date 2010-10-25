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

function p9

  t = automatic_replot;
  automatic_replot = 0;

  as = autostyle;
  autostyle ("off");

  ls = legend;
  legend "off"

  step = 0.2;
  x = -2:step:2;
  y = -2:step:2;
  [xx,yy] = meshgrid(x,y);
  z = xx .* exp(-xx.^2 - yy.^2);
  [gy, gx] = gradn(z,step,step);
  title("Vector field example");
  contour(x,y,z)
  hold on;
  arrows(yy,xx,gx./2,gy./2);
  hold off
  plflush;

  legend(ls);
  autostyle(as); 
  automatic_replot = t;

endfunction
