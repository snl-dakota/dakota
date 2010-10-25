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

## usage: polar (theta, rho, fmt)
##
## Make a 2D plot given polar the coordinates theta and rho.
##
## The optional third argument specifies the line type.

## Author: jwe
## Modified: jc

function polar (x1, x2, fmt)

  global __pl
  strm = __pl_init;

  __pl.type(strm) = -1;
  if (!ishold)
    __pl.items(strm) = 1;
    __pl.lxm(strm) = __pl.lym(strm) = realmax;
    __pl.lxM(strm) = __pl.lyM(strm) = -realmax;
  endif
  
  if (nargin == 3)
    if (!isstr (fmt))
      error ("polar: third argument must be a string");
    endif
    __plr2__ (x1, x2, fmt);
  elseif (nargin == 2)
    if (isstr (x2))
      __plr1__ (x1, fmt);
    else
      fmt = "";
      __plr2__ (x1, x2, fmt);
    endif
  elseif (nargin == 1)
    fmt = "";
    __plr1__ (x1, fmt);
  else
    usage ("polar (theta, rho, fmt)");
  endif
  
  __pl_plotit;
  
endfunction
