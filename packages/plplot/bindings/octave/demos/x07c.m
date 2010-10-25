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

## Displays the entire "plsym" symbol (font) set.

function x07c

  if (!exist("plinit"))
    plplot_stub
  endif

  if(isempty(sprintf("%s",plgdev')))
    plsdev("xwin");
  endif

  ## Parse and process command line arguments */

  ##    (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

  ## Initialize plplot */
  plinit();

  base=[0, 200, 500, 600, 700, 800, 900, \
	2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900];
  
  plfontld(1);
  for l=0:16
    pladv(0);

    ## Set up viewport and window */

    plcol0(2);
    plvpor(0.15, 0.95, 0.1, 0.9);
    plwind(0.0, 1.0, 0.0, 1.0);

    ## Draw the grid using plbox */

    plbox("bcgt", 0.1, 0, "bcgt", 0.1, 0);

    ## Write the digits below the frame */

    plcol0(15);
    for i=0:9
      text=sprintf("%d", i);
      plmtex("b", 1.5, (0.1 * i + 0.05), 0.5, text);
    endfor

    k = 0;
    for i=0:9

      ## Write the digits to the left of the frame */

      text=sprintf("%d", base(l+1) + 10 * i);
      plmtex("lv", 1.0, (0.95 - 0.1 * i), 1.0, text);
      for j=0:9
	x = 0.1 * j + 0.05;
	y = 0.95 - 0.1 * i;

	## Display the symbols */

	plsym(x, y, base(l+1) + k);
	k = k + 1;
      endfor
    endfor

    plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 7 - PLSYM symbols");
  endfor
  plend1();
endfunction
