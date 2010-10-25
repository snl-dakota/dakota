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

## __pl_fill(x, y, c)
##
## Draw filled 2-D polygons.
##
## FILL(X,Y,C) fills the 2-D polygon defined by vectors X and Y
## with the color specified by C.  The vertices of the polygon
## are specified by pairs of components of X and Y.  If necessary,
## the polygon is closed by connecting the last vertex to the first.
##
## If C is a single character string chosen from the list 'r','g','b',
## 'c','m','y','w','k', or an RGB row vector triple, [r g b], or a scalar
## in the range 0-15, the polygon is filled with the constant specified color.

function __pl_fill(x, y, c)

  global __pl

  strm = plgstrm+1;

  if (is_vector(x) & is_vector(y))
    if (columns(x) != 1)
      x = x';
      y = y';
    endif

    if (length(x) != length(y))
      error("__pl_fill: x and y must have same size\n");
    endif
    
    plpsty(0); # solid fill

    if (__pl.axis_st(strm) == 1)
      xmin = __pl.axis(strm,1); xmax = __pl.axis(strm,2);
      ymin = __pl.axis(strm,3); ymin = __pl.axis(strm,4);
    else
      xmin=min(x); xmax=max(x); ymin=min(y); ymax=max(y);
    endif
    
    if (!ishold)
      plcol(15);
      __pl_plenv(min(x), max(x), min(y), max(y), 0, -1);
    endif
    
    if (isstr(c) & length(c) == 1)
      ## map to plplot color
      coln = ['r','g','b','c','m','y','w','k'];
      coli = [2, 4, 10, 12, 14, 3, 16, 1];
      col = coli(find(c == coln))-1;
      plcol(col);
    elseif (is_scalar(c))
      if (ceil(c) == c)	#integer
	plcol(c);
      else
	plcol1(c);
      endif
    elseif (is_vector(c) & length(c) == 3)
      ## FIXME -- color 15 became permanently set!
      plscol0(15, c(1), c(2), c(3));
      plcol(15);
    elseif (is_vector(c) & length(x) == length(x))
      
      ## If C is a vector the same length as X and Y, its elements are
      ## scaled by CAXIS and used as indices into the current COLORMAP to
      ## specify colors at the vertices; the color within the polygon is
      ## obtained by bilinear interpolation in the vertex colors.
      
      error("__pl_fill: gradient fill, not yet.\n");
    else
      error("__pl_fill: color ????.\n");
    endif

    plfill(x,y)
    plflush;pleop;

  endif

endfunction
