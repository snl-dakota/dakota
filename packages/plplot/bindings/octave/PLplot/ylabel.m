## Copyright (C) 1998-2003 Joao Cardoso.
##
## This program  is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program  is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## This file is part of plplot_octave.

## usage: ylabel (text)
##
## Defines a label for the y-axis of the next plot
##
## See also: title, xlabel, ylabel, zlabel

function text = ylabel (text)

  global __pl

  strm = __pl_init;

  if (nargin > 1)
    usage ("ylable (text)");
  endif
  
  if (nargin == 1 && isempty(text))
    text = " ";
  endif
  
  if (nargin == 0)
    text = __pl.ylabel(strm,:);
  else
    __pl.ylabel = __pl_matstr(__pl.ylabel, text, strm);
  endif

  if (automatic_replot)
    __pl_plotit;
  endif

endfunction
