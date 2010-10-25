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

function __pl_store(x, y, fmt)

  global __pl
  strm = __pl_init;

  if (!struct_contains(__pl, "items"))
    __pl.items(strm) = 1;
  endif

  items = __pl.items(strm);

  ## find the max/min x/y values. Currently reset at __plt__
  ## for log plots use only positive values

  if (__pl.type(strm) == 10 || __pl.type(strm) == 30)
    minx = min(min(x(x>0))); maxx = max(max(x(x>0)));
  else
    minx = min(min(x)); maxx = max(max(x));
  endif

  if (__pl.type(strm) == 20 || __pl.type(strm) == 30)		
    miny = min(min(y(y>0))); maxy = max(max(y(y>0)));
  else
    miny = min(min(y)); maxy = max(max(y));
  endif

  __pl.lxm(strm) = min([__pl.lxm(strm), minx]);
  __pl.lxM(strm) = max([__pl.lxM(strm), maxx]);

  __pl.lym(strm) = min([__pl.lym(strm), miny]);
  __pl.lyM(strm) = max([__pl.lyM(strm), maxy]);

  __pl.x{items, strm} = x;
  __pl.y{items, strm} = y;
  __pl.fmt{items, strm} = fmt;

  __pl.items(strm)++;

endfunction
