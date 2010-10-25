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

## usage: subplot (rows, columns, index)
##        subplot (rcn)
##
## Sets plot in multiplot mode and plots in location
## given by index (there are columns X rows subwindows)
##
## Input:
##
##   rows   : number of rows in subplot grid
##   columns: number of columns in subplot grid
##   index  : index of subplot where to make the next plot
##
## If only one arg, then it (crn) has to be three digit value
## specifying the location in digit 1 (rows) and 2 (columns) and digit
## 3 is the plot index
##
## The plot index runs row-wise,i.e., first all the columns in a row
## are filled and then the next row is filled
##
## For example, plot with 4 X 2 grid, will have plot indices running as
## follows:
##
##   -----------------------------------
##   |        |       |       |        |
##   |    1   |    2  |    3  |    4   |
##   |        |       |       |        |
##   -----------------------------------
##   |        |       |       |        |
##   |    5   |    6  |    7  |    8   |
##   |        |       |       |        |
##   -----------------------------------
##

## Author: Vinayak Dutt <Dutt.Vinayak@mayo.EDU>
## Adapted-By: jwe
## Modified: jc

function subplot (rows, columns, index)

  global __pl
  strm = __pl_init;

  if (nargin != 3 && nargin != 1)
    usage ("subplot (rows, columns, index) or subplot (rcn)");
  endif

  if (nargin == 1)

    if (! (is_scalar (rows) && rows >= 0))
      error ("subplot: input rcn has to be a positive scalar");
    endif

    tmp = rows;
    index = rem (tmp, 10);
    tmp = (tmp - index) / 10;
    columns = rem (tmp, 10);
    tmp = (tmp - columns) / 10;
    rows = rem (tmp, 10);

  elseif (! (is_scalar (columns) && is_scalar (rows) && is_scalar (index)))
    error ("subplot: columns, rows, and index have to be scalars");
  endif

  columns = round (columns);
  rows = round (rows);
  index = round (index);

  if (index > columns*rows)
    error ("subplot: index must be less than columns*rows");
  endif

  if (columns < 1 || rows < 1 || index < 1)
    error ("subplot: columns,rows,index must be be positive");
  endif

  if (columns*rows == 1) # switching to single plot
    plssub(1,1);
  else
    
    ## already in multiplot with same characteristics ?
    __pl.multi_cur(strm) = index;
    
    if (__pl.multi(strm) == 1 &&   
	__pl.multi_col(strm) == columns &&
	__pl.multi_row(strm) == rows)
      
      pladv(index);
      plvpor(0,1,0,1)
      plwind(0,1,0,1)
      plcol(0); plpsty(0)
      plfill([0; 1; 1; 0],[0; 0; 1; 1]);
      plflush; pleop;
    else
      __pl.multi_col(strm) = columns;
      __pl.multi_row(strm) = rows;
      __pl.multi(strm) = 1;
      plssub(columns, rows);
      pladv(index);
    endif

  endif

endfunction
