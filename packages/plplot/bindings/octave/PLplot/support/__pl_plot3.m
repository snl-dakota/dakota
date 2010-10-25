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

function __pl_plot3(x, y, z, c)

  ## __pl_plot3(x, y, z [, c])

  global __pl

  strm = plgstrm+1;

  if (is_vector(x) & is_vector(y) & is_vector(z))

    if (columns(x) != 1)
      x = x';
      y = y';
      z = z';
    endif

    if (length(x) != length(y) || length(x) != length(z))
      error("__pl_plot3: x and y and z must have same size\n");
    endif

    [style, color, symbol, key_title] = __pl_opt (c);
    
    xm = min(x); xM = max(x);
    ym = min(y); yM = max(y);
    zm = min(z); zM = max(z);
    
    if (__pl.axis_st(strm))
      xm = __pl.axis(strm,1); xM = __pl.axis(strm,2);	# at least x always exist
      
      if (length(__pl.axis) >= 4)	
	ym = __pl.axis(strm,3); yM = __pl.axis(strm,4);
      else
	__pl.axis(strm,3) = ym; __pl.axis(strm,4) = yM;
      endif
      if (length(__pl.axis) == 6)
	zm = __pl.axis(strm,5); zM = __pl.axis(strm,6);
      else
	__pl.axis(strm,5) = zm; __pl.axis(strm,6) = zM;		
      endif
    else # make axis() return current axis
      __pl.axis(strm,1) = xm; __pl.axis(strm,2) = xM;
      __pl.axis(strm,3) = ym; __pl.axis(strm,4) = yM;
      __pl.axis(strm,5) = zm; __pl.axis(strm,6) = zM;		
    endif
    
    if (!ishold)
      plcol(15);
      __pl_plenv(-1.6, 1.6, -1.6, 2.6, 0, -2);
      pllab("", "", tdeblank(__pl.tlabel(strm,:)));
      plw3d(2, 2, 2, xm, xM, ym, yM, zm, zM,...
	    __pl.alt(strm), __pl.az(strm))
      plbox3("bnstu", tdeblank(__pl.xlabel(strm,:)), 0.0, 0,...
	     "bnstu",tdeblank(__pl.ylabel(strm,:)), 0.0, 0,...
	     "bcdmnstuv", tdeblank(__pl.zlabel(strm,:)), 0.0, 0)
    endif

    plcol(color)

    switch (style)
      case (1)
	plpoin3(x, y, z, symbol);
	plline3(x, y, z);
      case (2)
	plline3(x, y, z);
      case (5)
	plpoin3(x, y, z, symbol);
      case (6)
	plpoin3(x, y, z, symbol);
    endswitch

    plflush;pleop;
    __pl.items(strm) = 1; # for now!
  endif

endfunction
