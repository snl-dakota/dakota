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

function __plr1__ (theta, fmt)

  if (nargin != 2)
    usage ("__plr1__ (theta, fmt)");
  endif

  [nr, nc] = size (theta);
  if (nr == 1)
    theta = theta';
    tmp = nr;
    nr = nc;
    nc = tmp;
  endif
  theta_i = imag (theta);
  if (any (theta_i))
    rho = theta_i;
    theta = real (theta);
  else
    rho = theta;
    theta = (1:nr)';
  endif

  __plr2__ (theta, rho, fmt);

endfunction
