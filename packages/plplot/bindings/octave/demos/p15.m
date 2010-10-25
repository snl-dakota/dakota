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

    
function p15

  t = automatic_replot;
  automatic_replot = 0;

  title "Lighted and shaded surface";
  xlabel "";
  ylabel "";
  [x, y, z] = rosenbrock; z = log(z);
  set_view(30, -60);
  colormap(gray);
  shading("flat");
  surfl(x, y, z);
  pause(1);
  set_view(50, 40);
  colormap(bgr);
  shading("faceted");
  surf(x, y, z);
  colormap('default');

  automatic_replot = t;

endfunction

