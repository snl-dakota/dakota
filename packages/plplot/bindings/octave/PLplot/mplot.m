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

## usage: mplot (x, y)
##        mplot (x1, y1, x2, y2, ...)
##        mplot (x, y, fmt)
##
## This is a modified version of plot() command to work with
## multiplot to plot multiple plots per page.
## This plot version automatically updates the plot position to
## next plot position after making the plot in the given subplot
## position.
##
## See command plot() for the various options to this command
## as this is just mulitplot version of the same command.

function mplot (varargin)

  global __pl
  strm = __pl_init;

  __pl.multi_cur(strm) = rem(__pl.multi_cur(strm)+1,
				  __pl.multi_row(strm) * 
				  __pl.multi_col(strm)+1);
  if (__pl.multi_cur(strm) == 0 )
    __pl.multi_cur(strm) = 1;
  endif
  
  pladv(__pl.multi_cur(strm))
  __plt__ ("plot", varargin{:});

endfunction
