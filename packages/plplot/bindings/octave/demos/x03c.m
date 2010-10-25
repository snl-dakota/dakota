## Copyright (C) 1998, 1999, 2000  Joao Cardoso
## Copyright (C) 2004  Rafael Laboissiere
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
## It is based on the corresponding demo function of PLplot.

## Generates polar plot, with 1-1 scaling.

function x03c

  ## Initialize plplot */
  if (!exist("plinit"))
    plplot_stub
  endif

  if(isempty(sprintf("%s",plgdev')))
    plsdev("xwin");
  endif

  plinit;

  dtr = pi / 180.0;

  i=1:360;
  x0 = cos(dtr * i);
  y0 = sin(dtr * i);
  
  ## Parse and process command line arguments */

  ##    (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

  ## Set up viewport and window, but do not draw box */

  plenv(-1.3, 1.3, -1.3, 1.3, 1, -2);
  for i=1:10
    x(:) = 0.1 * i * x0(:);
    y(:) = 0.1 * i * y0(:);
    
    ## Draw circles for polar grid */
    plline(x', y');
  endfor

  plcol0(2);
  for i=0:11
    theta = 30.0 * i;
    dx = cos(dtr * theta);
    dy = sin(dtr * theta);

    ## Draw radial spokes for polar grid */

    pljoin(0.0, 0.0, dx, dy);
    text=sprintf("%d", theta);

    ## Write labels for angle */

    if (dx >= 0)
      plptex(dx, dy, dx, dy, -0.15, text);
    else
      plptex(dx, dy, -dx, -dy, 1.15, text);
    endif
  endfor

  ## Draw the graph */

  i=1:360;
  r = sin(dtr * (5 * i));
  x = x0 .* r;
  y = y0 .* r;

  plcol0(3);
  plline(x', y');

  plcol0(4);
  plmtex("t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh");

  ## Close the plot at end */
  plend1();
endfunction

