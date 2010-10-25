## Copyright (C) 1998, 1999, 2000, 2001, 2002 Joao Cardoso.
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

function [x, y, z] = rosenbrock ()

# [x y z] = rosenbrock()
#
#  Returns arrays ready for a mesh() or contour() plot of rosenbrock's function.
#  The unequal number of points and ranges, as well as the assimetry of the
#  function are usefull for debugging mesh(), contour(), etc :)
#
#  You might find log(z) more interesting to plot than z itself.
#
#  For a reference, see the enclosed rosen.gnu file, ready for execution under
#  gnuplot. You can execute it under octave as `rosen'
      

	nx = 46;
	ny = 56;
	x = linspace (-1.5, 1.5, nx);
	y = linspace (-0.5, 1.5, ny);
	[xx, yy] = meshgrid (x, y);
	z = ((1 - xx) .^ 2 + 100 .* (yy - xx .^ 2) .^ 2);
	z = z;
endfunction
              
