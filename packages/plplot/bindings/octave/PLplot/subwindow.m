## Copyright (C) 1998-2003 Joao Cardoso.
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

## usage: subwindow ([xn, yn])
##
## Sets subwindow position in multiplot mode for next plot. The
## multiplot mode has to be previously initialized using multiplot()
## command, else this command just becomes an aliad to multiplot()
##
## With no argument, reports current subwindow

function subwindow (xn, yn)

  global __pl
  strm = __pl_init;

  if (nargin != 2 && nargin != 0)
    usage ("subwindow (xn, yn)");
  endif

  if (nargin == 0)
    __pl.multi_row(strm)
    __pl.multi_col(strm)
    return
  endif

  ## check for scalar inputs

  if (! (is_scalar (xn) && is_scalar (yn)))
    error ("subwindow: xn and yn have to be scalars");
  endif
  
  xn = round (xn);
  yn = round (yn);

  if (! __pl.multi(strm))
    multiplot (xn, yn);
    return;
  endif
  
  if (xn < 1 || xn > __pl.multi_row(strm) ||
      yn < 1 || yn > __pl.multi_col(strm))
    error ("subwindow: incorrect xn and yn");
  endif
  
  __pl.multi_cur(strm) = (yn-1)*__pl.multi_row(strm) + xn;
  pladv(__pl.multi_cur(strm));
  
endfunction
