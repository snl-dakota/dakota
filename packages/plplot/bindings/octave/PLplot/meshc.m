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

## usage: meshc ([x, y,] z)
##
## Surface plot with contour at base xy plane.
##
## If x, y, and z are matrices with the same dimensions,
## then corresponding elements represent vertices of the plot.
##
## If x and y are vectors, then a typical vertex is (x(j), y(i), z(i,j)).  Thus,
## columns of z correspond to different x values and rows of z correspond
## to different y values.

function meshc (varargin)

  __pl_mesh ("meshc", varargin{:});
  
endfunction
