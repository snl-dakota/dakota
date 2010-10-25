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

## usage: contour ([x, y,] z [, n])
##
##	Draws a contour plot of matrix z versus vectors x and y.
##	If n is a scalar, then n contour lines will be ploted
##	If n is a vector, then length(n) contour lines will be plot at the
##	value specified by n.
##
## See also: plot, semilogx, semilogy, loglog, polar, mesh, contour,
##           bar, stairs, gplot, gsplot, replot, xlabel, ylabel, title

## Author: jwe
## Modified: jc

function contour (z, n, x, y)

  global __pl
  __pl_init;
  
  if (nargin == 1)
    n = 8;
  endif
  if (nargin == 3)
    lines = 8;
  endif
  
  ## XXX FIXME XXX -- these plot states should really just be set
  ## temporarily, probably inside an unwind_protect block, but there is
  ## no way to determine their current values.

  if (nargin == 1 || nargin == 2)
    if (is_matrix (z))
      __pl_contour(1:columns(z), 1:rows(z), z', n);      
    else
      error ("contour: argument must be a matrix");
    endif
  elseif (nargin == 3 || nargin == 4)
    if (nargin == 4 && !is_scalar(n))	# matlab syntax
      temp1 = z; lines = y; y = n; z = x; x = temp1;
    elseif (nargin == 4 && is_scalar(n))
      lines = n;
    elseif (nargin == 3)	# matlab syntax
      temp1 = z; y = n; z = x; x = temp1;
    endif	
    if (is_vector (x) && is_vector (y) && is_matrix (z))
      if (length (x) == columns (z) && length (y) == rows (z))
	if (rows (x) == 1)
	  x = x';
	endif
	if (rows (y) == 1)
	  y = y';
	endif
	
	__pl_contour(x, y, z', lines);
      else
        msg = "contour: rows (z) must be the same as length (x) and";
        msg = sprintf ("%s\ncolumns (z) must be the same as length (y)", msg);
        error (msg);
      endif
    else
      error ("contour: x and y must be vectors and z must be a matrix");
    endif
  else
    usage ("contour (z [, levels [, x, y]]) or contour (x, y, z [, levels])");
  endif

endfunction
