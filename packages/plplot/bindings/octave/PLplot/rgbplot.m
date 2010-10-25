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

## rgbplot(map)
##
## Plot color map.
## rgbplot(map) plots a color map, i.e. an m-by-3 matrix which
## is appropriate input for colormap. The three columns of the
## colormap matrix are plotted in red, green, and blue lines.

function rgbplot(map)

  if (columns(map) != 3)
    error('map must be a 3-column colormap matrix.');
  endif

  m = 1:size(map,1);
  leg = ['r;Red;'; 'g;Green;'; 'b;Blue;'];

  plot(m, map, leg);

endfunction
