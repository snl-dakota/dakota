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

function p5

  t = automatic_replot;
  automatic_replot = 0;

  x=-pi:0.01:pi;
  y=sin(x).+cos(3*x);

  og = grid "on";
  title("Polar example");
  polar(x,y,'y')
  grid (og);

  automatic_replot = t;

endfunction
