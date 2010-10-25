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

## cmap = colormap([map])
## 
## Set colormap1 to map; map is an n row by 3 columns matrix, representing
##   red, green and blue components in the range [0..1]
##
## if map == 'default', returns and sets the default hsl colormap
## if map don't exist, return the current colormap

function ccmap = colormap(map)

  global __pl
  __pl_init;

  if (nargin == 0)
    if (struct_contains(__pl, "colormap"))
      cmap = __pl.colormap;
    else
      n = 64;
      r = [zeros(n/2,1); linspace(0,1,n/2)'];
      g = zeros(n,1);
      b = [linspace(1,0,n/2)'; zeros(n/2,1)];
      cmap = __pl.colormap = [r, g, b];
    endif

    if (nargout)
      ccmap = cmap;
    endif
    return
  endif

  if (nargin == 1 && isstr(map) && strcmp(map, 'default'))
    plscmap1n(0);
    n = 64;
    r = [zeros(n/2,1); linspace(0,1,n/2)'];
    g = zeros(n,1);
    b = [linspace(1,0,n/2)'; zeros(n/2,1)];
    map = __pl.colormap = [r, g, b];
    if (nargout)
      ccmap = map;
    endif
  endif

  [r, c] = size(map);
  if( c != 3)
    help colormap
    return
  endif

  __pl.colormap = map;
  cmap = map;
  map = map.*255;

  plscmap1n(rows(map)); # number of colors
  plscmap1(map(:,1), map(:,2), map(:,3));
  plflush;#pleop;
  
  if (automatic_replot)
    ## can cause problems on plot scripts that dont support automatic _replot
    __pl_plotit; 
  endif

  if (nargout)
    ccmap = cmap;
  endif

endfunction
