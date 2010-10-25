## Copyright (C) 1996, 1997 John W. Eaton
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

## usage: is_strmatrix (x)
##
## Return 1 if X is a matrix.  Otherwise, return 0.
##
## See also: size, rows, columns, length, is_scalar, is_vector

## Author: jwe
## modified: jc

function retval = is_strmatrix (x)

  if (nargin == 1)
    [nr, nc] = size (x);
    retval = (nr > 0 && nc > 0);
  else
    usage ("is_strmatrix (x)");
  endif

endfunction
