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

## hot(m) returns an m-by-3 matrix containing a "hot" colormap.
## without arguments, cmap has lenght 64

function cmap = hot(m)

  if (nargin < 1)
    m = 64;
  endif

  n = fix(3/8*m);
  r = [(1:n)'/n; ones(m-n,1)];
  g = [zeros(n,1); (1:n)'/n; ones(m-2*n,1)];
  b = [zeros(2*n,1); (1:m-2*n)'/(m-2*n)];

  cmap = [r, g, b];

endfunction
