## Copyright (C) 1996 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.

## usage: multiplot (xn, yn)
##
## Sets and resets multiplot mode
##
## If multiplot(0,0) then it will close multiplot mode and and if
## arguments are non-zero, then it will set up multiplot mode with
## xn,yn subplots along x and y axes.
##
## Author: Vinayak Dutt, Dutt.Vinayak@mayo.EDU
## Created: 3 July 95
## Adapted-By: jwe
## Modified: jc

function multiplot (xn, yn)

  global __pl
  strm = __pl_init;

  if (nargin != 2)
    usage ("multiplot (xn, yn)");
  endif

  if (! (is_scalar (xn) && is_scalar (yn)))
    error ("multiplot: xn and yn have to be scalars");
  endif

  xn = round (xn);
  yn = round (yn);

  if (xn == 0 && yn == 0)
    oneplot ();
  else
    if (xn < 1 || yn < 1)
      error ("multiplot: xn and yn have to be positive integers");
    endif

    __pl.multi(strm) = 1;
    __pl.multi_row(strm) = xn;
    __pl.multi_col(strm) = yn;
    __pl.multi_cur(strm) = 1;
    
    plssub(xn, yn);
    pladv(0);

  endif

endfunction

