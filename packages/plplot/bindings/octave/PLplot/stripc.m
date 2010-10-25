## Copyright (C) 1998-2003 Joao Cardoso.
## Copyright (C) 2004  Rafael Laboissiere
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

## id = stripc (xmin, xmax, xjump, ymin, ymax,
##	leg1, leg2, leg3, leg4,
##      acc, autoy)
##
## creates a four pen stripchar. Points are added calling stripc_add().
## the scripchart *must* be deleted after use calling stripc_del()
##
## xmin, ymin, xmax and ymax are initial rasonable values for the chart
## xjump is the percentage of the window that skips when the plot lines
## arrive at the right.
## leg[i] is the legend of plot pen 'i'
## acc if '1' means to accumulate values instead of losting it, as a normal
## stripchart does (well, paper is always available)
## autoy if '1' means to autoscale the y axis *between* xjumps. After a
## xjump, the y scale is always rescaled.
## The current values of xlabel, ylabel, and legend position are used.
##
## see also: stripc_add, stripc_del

function id = stripc (xmin, xmax, xjump, ymin, ymax, leg1, leg2, leg3, leg4, acc, autoy)

  global __pl
  strm = __pl_init;

  colbox =  collab = 15;
  styline(1)  = 1;	# pens line style
  styline(2) = 1;
  styline(3) = 1;
  styline(4) = 1;    

  colline(1) = 1;	# pens color
  colline(2) = 9;
  colline(3) = 3;
  colline(4) = 4;    

  id = plstripc("bcnst", "bcnstv",
		xmin, xmax, xjump, ymin, ymax,
		__pl.legend_xpos(strm), __pl.legend_ypos(strm), 
		autoy, acc, 15, 15,
		colline', styline', leg1, leg2, leg3, leg4,
		tdeblank(__pl.xlabel(strm,:)),
		tdeblank(__pl.ylabel(strm,:)),
		tdeblank(__pl.tlabel(strm,:)));

endfunction
