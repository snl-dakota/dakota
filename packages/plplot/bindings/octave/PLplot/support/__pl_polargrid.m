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

function __pl_polargrid(__pl_lxM, __pl_lyM, __pl_lxm, __pl_lym)

  rM = max (abs([__pl_lxM, __pl_lyM, __pl_lxm, __pl_lym]));	# max rho
  ## rm = min (abs([__pl_lxM, __pl_lyM, __pl_lxm, __pl_lym]));

  [o_ch, o_sc] = plgchr;
  o_sc = o_sc/o_ch;
  plschr(0, 0.5);	# get smaller characters

  ## Draw circles for polar grid
  
  dtr = pi / 180.0;

  res=0:10:360; x0 = cos(dtr .* res); y0 = sin(dtr .* res);
  ## if (rm <= 0) rm = rM/10; endif
  for m=linspace(rM/10, rM, 10)
    xx = m .* x0; yy = m .* y0;
    text = sprintf("%.2f",m); # Write labels for circles (rho)
    plptex(xx(10), yy(10), 0, 0, 0, text);
    plline(xx', yy');
  endfor
  
  ## Draw radial spokes for polar grid
  
  for m=0:24
    theta = 15 * m;
    dx = rM * cos(dtr * theta);
    dy = rM * sin(dtr * theta);
    pljoin(0, 0, dx, dy);
    text=sprintf("%d", theta); # Write labels for angle (theta)
    if (dx >= 0)
      plptex(dx, dy, dx, dy, 1, text);
    else
      plptex(dx, dy, -dx, -dy, 0, text);
    endif
  endfor
  plschr(0, o_sc);    # restore character weight

endfunction
