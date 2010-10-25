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

## usage: zlabel (text)
##
## Defines a label for the z-axis of the next plots
##
## See also: xlabel, ylabel, zlabel

function text = zlabel (text)

  global __pl

  strm =__pl_init;

  if (nargin > 1)
    usage ("zlable (text)");
  endif
  
  if (isempty(text))
    text = " ";
  endif
  
  if (nargin == 0)
    text = __pl.zlabel(strm,:);
  else
    __pl.zlabel = __pl_matstr(__pl.zlabel, text, strm);
  endif

  if (automatic_replot)
    __pl_plotit;
  endif

endfunction
