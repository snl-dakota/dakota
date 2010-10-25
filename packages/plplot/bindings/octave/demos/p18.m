## Copyright (C) 2002 Joao Cardoso.
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

function p18

  t = automatic_replot;
  automatic_replot = 0;

  plsetopt "db";
  fig( free_fig); #  unused plot window

  [x, y, z] = peaks;
  colormap(bgr);

  title("Set and drag button 1");
  xlabel "Button 2 to finish";
  ylabel "Button 3 to restart";
  meshc( x, y, z);

  set_view("meshc", x, y, z);
  closefig

  automatic_replot = 0;

endfunction