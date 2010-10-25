## Copyright (C) 1998, 1999, 2000 Joao Cardoso.
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

#	Multiple window and color map 0 demo.

function x02c

  ## Initialize plplot */
  if (!exist("plinit"))
    plplot_stub
  endif

  if(isempty(sprintf("%s",plgdev')))
    plsdev("xwin");
  endif

  plinit();

  ## Divide screen into 16 regions */
  plssub(4, 4);

  ## Parse and process command line arguments */

  ## (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

  plschr(0.0, 3.5);
  plfont(4);

  for i=0:15
    plcol0(i);
    text=sprintf("%d", i);
    pladv(0);
    vmin = 0.1;
    vmax = 0.9;
    for j=0:2
      plwid(j + 1);
      plvpor(vmin, vmax, vmin, vmax);
      plwind(0.0, 1.0, 0.0, 1.0);
      plbox("bc", 0.0, 0, "bc", 0.0, 0);
      vmin = vmin + 0.1;
      vmax = vmax - 0.1;
    endfor
    plwid(1);
    plptex(0.5, 0.5, 1.0, 0.0, 0.5, text);
  endfor

  plend1();

endfunction
