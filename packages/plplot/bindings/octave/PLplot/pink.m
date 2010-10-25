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

## cmap = pink(m)
##
## Pastel shades of pink color map.
## pink(M) returns an M-by-3 matrix containing a "pink" colormap.
## without arguments, cmap has lenght 64

function cmap = pink(m)

  if (nargin < 1)
    m=64;
  endif

  cmap = sqrt((2*gray(m) + hot(m))/3);

endfunction
