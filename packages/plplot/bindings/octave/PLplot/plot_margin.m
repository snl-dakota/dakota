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

## st = plot_margin([st])
##
## set/return the status of the plot margin. The plot margin is small
## region near the box that surrouunds the plot; it is usefull not to
## plot in this region for scatter plot, or the plotted points can be
## ploted over the box.

function ost = plot_margin(st)

  global __pl
  n = __pl_init;

  ost = __pl.margin(n);

  if (nargin == 1)
    __pl.margin(n) = st;
  endif

  if (automatic_replot)
    __pl_plotit;
  endif

endfunction
