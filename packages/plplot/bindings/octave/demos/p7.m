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

function p7

  t = automatic_replot;
  automatic_replot = 0;

  [x y z]=rosenbrock;z=log(z);

  title("Shade example");
  colormap('default')
  shade(x,y,z,15)
  pause(1)

  colormap(pink);
  axis([0.5 1.2 0 1.3]);
  hold on;
  shade(x,y,z,15,3);
  pause(1)
  hold off

  shade(x,y,z,15,3);
  axis;

  automatic_replot = t;
endfunction
