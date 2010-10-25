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

## plot3 (x, y, z [, fmt])
##
## PLOT3() is a three-dimensional analogue of PLOT().
## 
## PLOT3(x,y,z), where x, y and z are three vectors of the same length,
## plots a line in 3-space through the points whose coordinates are the
## elements of x, y and z.
## 
## PLOT3(X,Y,Z), where X, Y and Z are three matrices of the same size,
## plots several lines obtained from the columns of X, Y and Z.
## 
## Various plot type, symbols and colors may be obtained with
## PLOT3(X,Y,Z,s) where s is a 1, 2 or 3 character string made from
## the characters listed under the PLOT command. See __pl_opt() and plot().

function plot3 (x, y, z, fmt)

  global __pl
  strm = __pl_init;

  if (nargin != 4 & nargin != 3)
    error("plot3: not yet.\n");
  endif

  if (is_vector(x) & is_vector(y) & is_vector(y))
    
    if (nargin == 3)
      fmt = "-r";
    endif	

    __pl_plot3(x, y, z, fmt);
    
  elseif (is_matrix(x) & is_matrix(y) & is_matrix(z))
    
    if (!common_size(x,y,z))

      if (rows(x) < columns(x))
	x = x'; y = y'; z = z';
      endif
      
      if (nargin == 3)
	fmt = "-r";
	for i=2:columns(x)
	  fmt = [fmt; sprintf("%d%d", i, i)];
	endfor
      elseif (rows(fmt) == 1)
	for i=2:columns(x)
	  fmt = [fmt; fmt];
	endfor
      elseif (rows(fmt) != columns(x))
	error("plot3: `fmt' must have same length as `x'.\n");
      endif
      
      xm = min(min(x)); xM = max(max(x));
      ym = min(min(y)); yM = max(max(y));
      zm = min(min(z)); zM = max(max(z));
      
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
      else	# make axis() return current axis
	__pl.axis(strm,1) = xm; __pl.axis(strm,2) = xM;
	__pl.axis(strm,3) = ym; __pl.axis(strm,4) = yM;
	__pl.axis(strm,5) = zm; __pl.axis(strm,6) = zM;		
      endif
      
      h_st = ishold;
      
      if (!ishold)
	plcol(15);
	__pl_plenv(-1.6, 1.6, -1.6, 2.6, 0, -2);
	pllab("", "", tdeblank(__pl.tlabel(strm,:)));
	plw3d(2, 2, 2, xm, xM, ym, yM, zm, zM,...
	      __pl.alt(strm), __pl.az(strm))
	plbox3("bnstu", tdeblank(__pl.xlabel(strm,:)), 0.0, 0,...
	       "bnstu",tdeblank(__pl.ylabel(strm,:)), 0.0, 0,...
	       "bcdmnstuv", tdeblank(__pl.zlabel(strm,:)), 0.0, 0)
	hold on
      endif

      for i=1:columns(x)
	__pl_plot3(x(:,i), y(:,i), z(:,i), fmt(i,:));
      endfor

      if (h_st == 0)
	hold off
      endif
    else
      error("plot3: x and y and z must have same dimension\n");
    endif
  endif

endfunction
