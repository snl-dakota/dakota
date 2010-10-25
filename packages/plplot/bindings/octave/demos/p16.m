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

function p16

  t = automatic_replot;
  automatic_replot = 0;

  multiplot(1, 2);
  p1;
  
  subwindow(1, 2);
  [x, y, z] = peaks;
  colormap(bgr);
  title "Peaks";
  xlabel "";
  ylabel "";
  shade(x, y, z);

  oneplot;
  automatic_replot = t;

endfunction
