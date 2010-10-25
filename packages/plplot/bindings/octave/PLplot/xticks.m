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

## xticks ([x_interval [, num_minor])
##
## Set the xticks interval, and the number of minor ticks between major xticks.
##
## No arguments makes it automatic (default).
##    Set x_interval = 0 for default ticks
##    Set x_interval = -1 to disable ticks (and tick labels)
##    Set x_interval = -2 to disable ticks labels,
##    Set num_minor = 0 to disable minor ticks.
##    Set num_minor = -1 for default minor ticks.

function xticks (int, num)

  global __pl

  strm = __pl_init;

  if (nargin >= 3)
    help xticks;
    return
  endif

  if (nargin == 0)
    __pl.xticks(strm,1) = 0;
    __pl.xticks(strm,2) = 0;
    __pl.xticks(strm,3) = 1;
    __pl.xticks(strm,4) = 1;
  endif

  if (nargin == 2)
    __pl.xticks(strm,2) = num+1;
  endif

  if (nargin >= 1)
    if (int == -1)
      __pl.xticks(strm,3) = 0;
    elseif (int == -2)
      __pl.xticks(strm,4) = 0;
    else
      __pl.xticks(strm,1) = int;
    endif
  endif

  if (automatic_replot)
    __pl_plotit;
  endif

endfunction
