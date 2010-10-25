## Copyright (C) 2001-2003 Joao Cardoso.
## 
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## This file is part of plplot_octave.

## usage: surfl ([x, y,] z [, s])
##
## Shaded Surface plot with lighting.
##
## If x, y, and z are matrices with the same dimensions,
##    then corresponding elements represent vertices of the plot.
##
## If x and y are vectors, then a typical vertex is (x(j), y(i), z(i,j)).  Thus,
##    columns of z correspond to different x values and rows of z correspond
##    to different y values.
##
## `s', if existent, set the light source in cartesian coordinates [Sx Sy Sz]
##    or [Saz Sel] in spherical coordinates; in this case, the light source will
##    be at a distance 10 from the origin.
##    Ilumination can also be set by pllightsource(Sx, Sy, Sz)

function surfl (x, y, z, s)

  global __pl

  flag = 0;
  if (nargin == 2)
    s = y; flag = 1;
  elseif (nargin == 1 || nargin == 3)
    s = [10, 10, 10];
  elseif (nargin == 4)
    flag = 1;
  endif

  if (is_vector(s))
    if (length(s) == 2) ## [az, alt]
      s = [10*cos(s(1)); 10*sin(s(1)); 10*sin(s(2))];
    endif
  else
    error("surfl: s must be a vector\n");
  endif

  __pl.light = s;

  if (nargin == 1  || (nargin == 2 && flag))
    __pl_mesh ("surfl", x);
  elseif (nargin == 3 || (nargin == 4 && flag))
    __pl_mesh ("surfl", x, y, z);
  else
    help "surfl"
  endif

endfunction
