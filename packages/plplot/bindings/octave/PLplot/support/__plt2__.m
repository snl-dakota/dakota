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

function __plt2__ (x1, x2, fmt)

  if (nargin < 2 || nargin > 3)
    usage ("__plt2__ (x1, x2, fmt)");
  endif

  if (nargin == 2)
    fmt = "";
  endif

  if (! isstr (fmt))
    error ("__plt2__: fmt must be a string");
  endif

  if (any (any (imag (x1))))
    x1 = real (x1);
  endif
  if (any (any (imag (x2))))
    x2 = real (x2);
  endif
  if (is_scalar (x1))
    if (is_scalar (x2))
      __plt2ss__ (x1, x2, fmt);
    endif
  elseif (is_vector (x1))
    if (is_vector (x2))
      __plt2vv__ (x1, x2, fmt);
    elseif (is_matrix (x2))
      __plt2vm__ (x1, x2, fmt);
    endif
  elseif (is_matrix (x1))
    if (is_vector (x2))
      __plt2mv__ (x1, x2, fmt);
    elseif (is_matrix (x2))
      __plt2mm__ (x1, x2, fmt);
    endif
  endif

endfunction
