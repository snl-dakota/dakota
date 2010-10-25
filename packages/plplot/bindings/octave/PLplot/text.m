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

##   text(x, y, string);
##
##   Place text on a 2-D graph at x y positions.
##	x and y can be scalar or vectors; if vectors, string can be either
##	a one row string or a matrix string with the same number of rows as x.

function text(x,y,string)

  if ( nargin != 3)
    help text
    return
  endif

  __pl_init;

  [xr, xc] = size(x);
  [yr, yc] = size(y);

  if (xc > xr)
    x = x'; y = y';
    xr = xc; yr = yc;
  endif

  if (xr != 1 || yr != 1)
    if (xr != yr)
      error("text: x and y must have same number of rows")
    else
      if (rows(string) == xr)
	for i=1:xr
	  plptex(x(i), y(i), 0, 0, 0, string(i,:));
	endfor
      elseif (rows(string) == 1)
	for i=1:xr
	  plptex(x(i), y(i), 0, 0, 0, string);
	endfor
      else
	error("text: string must have same number of rows as x and y")
      endif
    endif
  else
    plptex(x, y, 0, 0, 0, string);
  endif

  plflush;pleop;

endfunction

