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

## zoom(arg)
##
## Using the mouse iteratively select an area for setting the axis.
## If 'arg' == "off" set axis to "autoscale"

function zoom(arg)
  
  global __pl
  strm = __pl_init;

  if (nargin == 1)
    if (strcmp(arg, "off"))
      axis;
      return
    endif
  endif

  [x1, y1, x2, y2] = plrb;

  if (__pl.type(strm) == 10 || __pl.type(strm) == 30)
    x1 = 10^x1; x1 = 10^x2;
  endif
  if (__pl.type(strm) == 20 || __pl.type(strm) == 30)
    y1 = 10^y1; y2 = 10^y2;
  endif
  axis([min(x1, x2), max(x1, x2), min(y1, y2), max(y1, y2)]);

endfunction
