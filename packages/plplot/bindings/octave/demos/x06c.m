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

## Displays the entire "plpoin" symbol (font) set.

function x06c

  ## Initialize plplot */
  if (!exist("plinit"))
    plplot_stub
  endif

  if(isempty(sprintf("%s",plgdev')))
    plsdev("xwin");
  endif

  ## Parse and process command line arguments */

  ## (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

  ## Initialize plplot */
  plinit();

  pladv(0);

  ## Set up viewport and window */

  plcol0(2);
  plvpor(0.1, 1.0, 0.1, 0.9);
  plwind(0.0, 1.0, 0.0, 1.3);

  ## Draw the grid using plbox */

  plbox("bcgt", 0.1, 0, "bcgt", 0.1, 0);

  ## Write the digits below the frame */

  plcol0(15);
  for i=0:9
    text=sprintf("%d", i);
    plmtex("b", 1.5, (0.1 * i + 0.05), 0.5, text);
  endfor

  k = 0;
  for i=0:12

    ## Write the digits to the left of the frame */

    text=sprintf("%d", 10 * i);
    plmtex("lv", 1.0, (1.0 - (2 * i + 1) / 26.0), 1.0, text);
    for j=0:9
      x = 0.1 * j + 0.05;
      y = 1.25 - 0.1 * i;

      ## Display the symbols (plpoin expects that x and y are arrays so */
      ## pass pointers) */

      if (k < 128)
	plpoin(x, y, k);
      endif
      k = k + 1;
    endfor
  endfor

  plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols");
  plend1();
endfunction
