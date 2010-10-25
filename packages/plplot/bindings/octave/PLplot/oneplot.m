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

## usage: oneplot
##
## Switches from multiplot (if in  multiplot mode) to single plot
## mode

function oneplot ()

  global __pl
  strm = __pl_init;

  __pl.multi_row(strm)=1;
  __pl.multi_col(strm)=1;
  __pl.multi_cur(strm)=1;
  __pl.multi(strm)=0;
  
  plssub(1,1);
  pladv(0);

endfunction
