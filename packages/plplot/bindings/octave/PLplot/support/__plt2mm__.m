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

## Author: jwe
## Modified: jc

function __plt2mm__ (x, y, fmt)

  if (nargin < 2 || nargin > 3)
    msg = sprintf ("__plt2mm__ (x, y)\n");
    msg = sprintf ("%s              __plt2mm__ (x, y, fmt)", msg);
    usage (msg);
  elseif (nargin == 2)
    fmt = "";
  endif

  [x_nr, x_nc] = size (x);
  [y_nr, y_nc] = size (y);

  ##  if (x_nr == y_nr && x_nc == y_nc)
  if (x_nc > 0)
    __pl_store(x, y, fmt);
  else
    error ("__plt2mm__: arguments must be a matrices");
  endif
  ##  else
  ##    error ("__plt2mm__: matrix dimensions must match");
  ##  endif

endfunction
