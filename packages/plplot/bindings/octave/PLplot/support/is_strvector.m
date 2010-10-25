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

## usage: is_strvector (x)
##
## Return 1 if X is a vector.  Otherwise, return 0.
##
## See also: size, rows, columns, length, is_scalar, is_matrix

## Author: jwe
## Modified: jc

function retval = is_strvector (x)

  if (nargin == 1)
    [nr, nc] = size (x);
    retval = ((nr == 1 && nc >= 1) || (nc == 1 && nr >= 1));
  else
    usage ("is_strvector (x)");
  endif

endfunction
