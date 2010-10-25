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

## __pl_plenv(xm, xM, ym, yM, scale, axis)
##
## this is equivalent to plenv, except that no padv(0) is performed.
## this is important for multiplot, as pladv(i) advances the subwindow,
## and pladv(0) advances the page, clearing all plots. However, not
## calling pladv(0), keeps all plots in the plot buffer,
## so "plreplot" replots all plots since the last call to
## pladv(0)/plbop. Instead, the (sub)window is cleared, and the
## subplot  position remains the same. Of course, it is not fast.
## Changed: The above described behaviour is only valid in multiplot mode,
## else, pladv(0) is issued.
## x/y ticks can be disable.

function __pl_plenv(xm, xM, ym, yM, scale, axis)

  global __pl
  strm = plgstrm + 1;

  if (__pl.multi(strm) == 1)	# multiplot, erase current subwindow
    plclear;
  else
    pladv(0);
  endif

  if (__pl.aspect(strm) == 2) ## square
    scale = 2;
  elseif (__pl.aspect(strm) == 1) ## equal
    scale = 1;
  endif
  
  if (scale == 1 || scale == 2)
    [t, chrht]= plgchr;
    lb = 8.0 * chrht;
    rb = 5.0 * chrht;
    tb = 5.0 * chrht;
    bb = 5.0 * chrht;
    dx = abs(xM - xm);
    dy = abs(yM - ym);
    [spxmin, spxmax, spymin, spymax] = plgspa;
    xsize = spxmax - spxmin;
    ysize = spymax - spymin;

    if (scale == 1 ) ## equal
      xscale = dx / (xsize - lb - rb);
      yscale = dy / (ysize - tb - bb);
      scale = max([xscale, yscale]);
      vpxmin = max([lb, 0.5 * (xsize - dx / scale)]);
      vpxmax = vpxmin + (dx / scale);
      vpymin = max([bb, 0.5 * (ysize - dy / scale)]);
      vpymax = vpymin + (dy / scale);
      plsvpa(vpxmin, vpxmax, vpymin, vpymax);
    elseif (scale == 2) ## square
      size = min([xsize-lb-rb, ysize-tb-bb]);
      vpxmin = lb;
      vpxmax = lb+size;
      vpymin = bb;
      vpymax = bb+size;
      plsvpa(vpxmin, vpxmax, vpymin, vpymax);
    endif
  endif
  
  if (scale == 0) ## normal
    plvsta;
  endif

  xrg = yrg = 0;
  if (__pl.margin(strm))
    xrg = (xM-xm)/50; yrg = (yM-ym)/50;
  endif
  
  plwind(xm-xrg, xM+xrg, ym-yrg, yM+yrg);
  
  ## axis=-2 : draw no box, axis or labels
  ## axis=-1 : draw box only
  ## axis= 0 : Draw box and label with coordinates
  ## axis= 1 : Also draw the coordinate axes
  ## axis= 2 : Draw a grid at major tick positions
  ## axis=10 : Logarithmic X axis, Linear Y axis, No X=0 axis
  ## axis=11 : + coordinate axis
  ## axis=12 : + grid
  ## axis=13 : + minor ticks grid
  ## axis=20 : Linear X axis, Logarithmic Y axis, No Y=0 axis
  ## axis=21 : + coordinate axis
  ## axis=22 : + grid
  ## axis=23 : + minor ticks grid
  ## axis=30 : Logarithmic X and Y axes
  ## axis=31 : + coordinate axes
  ## axis=32 : + grid
  ## axis=33 : + minor ticks grid

  switch(axis)
    case -2
      xopt=""; yopt="";
    case -1
      xopt="bc"; yopt="bc";
    case 0
      xopt="bcnst"; yopt="bcnstv";
    case 1
      xopt="bcnsta"; yopt="bcnstva";
    case 2
      xopt="bcnstag"; yopt="bcnstvag";
    case 3
      xopt="bcnstagh"; yopt="bcnstvagh";		
    case 10
      xopt="bcnstl"; yopt="bcnstv";
    case 11
      xopt="bcnstl"; yopt="bcnstva";
    case 12
      xopt="bcgnstl"; yopt="bcnstvag";
    case 13
      xopt="bcgnstlh"; yopt="bcnstvagh";		
    case 20
      xopt="bcnst"; yopt="bcnstvl";
    case 21
      xopt="bcnst"; yopt="bcnstval";
    case 22
      xopt="bcnstg"; yopt="bcnstvagl";
    case 23
      xopt="bcnstgh"; yopt="bcnstvaglh";		
    case 30
      xopt="bcnstl"; yopt="bcnstvl";
    case 31
      xopt="bcnstl"; yopt="bcnstval";
    case 32
      xopt="bcngstl"; yopt="abcnstvgl";
    case 33
      xopt="bcngstlh"; yopt="abcnstvglh";
  endswitch

  ## disable ticks
  if (__pl.xticks(strm,3) == 0)
    xopt = strrep (xopt, "t", "");
  endif

  ## disable tick labels
  if (__pl.xticks(strm,4) == 0)
    xopt = strrep (xopt, "n", "");
  endif

  if (__pl.yticks(strm,3) == 0)
    yopt = strrep (yopt, "t", "");
  endif

  if (__pl.yticks(strm,4) == 0)
    yopt = strrep (yopt, "n", "");
  endif

  plcol(15); pllsty(1);
  plbox(xopt, __pl.xticks(strm,1), __pl.xticks(strm,2),
	yopt, __pl.yticks(strm,1), __pl.yticks(strm,2));            
  
endfunction
