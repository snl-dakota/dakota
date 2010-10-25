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

function __pl_mesh(caller, varargin)

  global __pl
  global DRAW_LINEXY MAG_COLOR BASE_CONT
  global TOP_CONT SURF_CONT DRAW_SIDES FACETED MESH

  strm = __pl_init;
  
  switch (caller)
    case ("mesh")
      __pl.type(strm) = 100;
    case ("meshc")
      __pl.type(strm) = 101;
    case ("meshz")
      __pl.type(strm) = 102;
    case ("surf")
      __pl.type(strm) = 103;
    case ("surfc")
      __pl.type(strm) = 104;
    case ("surfl")
      __pl.type(strm) = 105;     
    otherwise
      error("__pl_mesh: FIXME")
  endswitch

  if (nargin == 2)
    z = varargin{1};
    if (is_matrix (z))
      [rz, cz] = size(z);
      x = (1:cz)'; 
      y = (1:rz)'; 
    else
      error ("mesh: argument must be a matrix.\n");
    endif
  elseif (nargin == 4 )
    x = varargin{1};
    y = varargin{2};
    z = varargin{3};
    if (is_vector (x) && is_vector (y) && is_matrix (z))
      xlen = length (x);
      ylen = length (y);
      if (xlen == columns (z) && ylen == rows (z))
        if (rows (y) == 1)
          y = y';
        endif
        if (rows (x) == 1)
          x = x';
        endif
      else
        msg = "mesh: rows (z) must be the same as length (x) and";
        msg = sprintf ("%s\ncolumns (z) must be the same as length (y).\n", msg);
        error (msg);
      endif
    elseif (is_matrix (x) && is_matrix (y) && is_matrix (z))			
      error("x,y and z all matrices not yet implemented.\n")
    else
      error ("mesh: x and y must be vectors and z must be a matrix.\n");
    endif
  else
    help (caller);
    return
  endif

  if (!ishold)
    __pl.items(strm) = 1;
    __pl.lxm(strm) = __pl.lym(strm) = __pl.lzm(strm) = realmax;
    __pl.lxM(strm) = __pl.lyM(strm) = __pl.lzM(strm) = -realmax;
  endif

  items = __pl.items(strm);

  ## find the max/min x/y values. Currently reset at ??
  __pl.lxm(strm) = min([__pl.lxm(strm), min(min(x))]);
  __pl.lxM(strm) = max([__pl.lxM(strm), max(max(x))]);

  __pl.lym(strm) = min([__pl.lym(strm), min(min(y))]);
  __pl.lyM(strm) = max([__pl.lyM(strm), max(max(y))]);

  __pl.lzm(strm) = min([__pl.lzm(strm), min(min(z))]);
  __pl.lzM(strm) = max([__pl.lzM(strm), max(max(z))]);

  ## kludge, use "fmt" as plot type. But __pl_plotit still uses __pl.type
    __pl.x{items, strm} = x;
    __pl.y{items, strm} = y;
    __pl.z{items, strm} = z;
    __pl.fmt{items, strm} = __pl.type(strm);

  __pl.items(strm) = __pl.items(strm) + 1;

  __pl_meshplotit;

endfunction
