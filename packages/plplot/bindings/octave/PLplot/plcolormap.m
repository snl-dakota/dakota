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

## cmap = plcolormap([map]), an alias to colormap().
## 
## Set colormap1 to map; map is an n row by 3 columns matrix, representing
##   red, green and blue components in the range [0..1]
##
## if map == 'default', returns and sets the default hsl colormap
## if map don't exist, return the current colormap

function ccmap = plcolormap(map)
  warning("plcolormap: use colormap instead.");
  ccmap = colormap(map);
endfunction
