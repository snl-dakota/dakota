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

##	fill(x, y, c)
##
##	Draw filled 2-D polygons.
##
## 	FILL(X,Y,C) fills the 2-D polygon defined by vectors X and Y
##	with the color specified by C.  The vertices of the polygon
##	are specified by pairs of components of X and Y.  If necessary,
##	the polygon is closed by connecting the last vertex to the first.
##
##	If C is a single character string chosen from the list 'r','g','b',
##	'c','m','y','w','k', or an RGB row vector triple, [r g b], or a scalar
## 	in the range 0-15, the polygon is filled with the constant specified color.
##
##	If X and Y are matrices the same size, one polygon per column
##	is drawn. In this case, C is a row vector for "flat" polygon
##	colors, and C is a matrix for "interpolated" polygon colors.
##
##	If either of X or Y is a matrix, and the other is a column vector
##	with the same number of rows, the column vector argument is
##	replicated to produce a matrix of the required size.
##
##   eg: x=rand(3,3)+1+rand*10;y=rand(3,3)-1+rand;
##       c=round(rand(3,1)*15+1);fill(x,y,c)

function fill(x, y, c)

  global __pl
  strm = __pl_init;

  if (nargin != 3)
    error("fill: not yet.\n");
  endif

  if (is_vector(x) & is_vector(y))
    __pl_fill(x, y, c);
    
  elseif (is_matrix(x) | is_matrix(y))

    if (rows(x) == rows(y))
      if (is_vector(x))
	x = x*ones(1,rows(x));
      elseif (is_vector(y))
	y = y*ones(1,rows(y));
      endif

      if (is_scalar(c))
	c = ones(rows(x),1)*c;
      elseif (rows(c) == 1)
	c = c';
	if (rows(c) != rows(x))
	  error("fill: `c' must be scalar or have same number of rows as `x'\n");
	endif
      endif

      h_st = ishold;

      if (__pl.axis_st(strm) == 1)
	xmin = __pl.axis(strm,1); xmax = __pl.axis(strm,2);
	ymin = __pl.axis(strm,3); ymin = __pl.axis(strm,4);
      else
	xmin=min(min(x)); xmax=max(max(x));
	ymin=min(min(y)); ymax=max(max(y));
      endif
      
      ## if (__pl.axis_st(strm) == 0)
      ##   xm = min(min(x)); xM = max(max(x));
      ##   ym = min(min(y)); yM = max(max(y));
      ##   axis([xm xM ym yM]);
      ## endif
      
      if (!ishold)
	plcol(15);
	__pl_plenv(xmin, xmax, ymin, ymax, 0, -1);
	hold on;
      endif

      for i=1:rows(x)
	__pl_fill(x(i,:), y(i,:), c(i,:));
      endfor

      if (h_st == 0)
	hold off
      endif
    else
      error("fill: x and y must have same number of rows\n");
    endif
  endif

endfunction
