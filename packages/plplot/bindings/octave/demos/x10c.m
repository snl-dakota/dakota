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

## Demonstrates absolute positioning of graphs on a page.


function x10c

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
  plvpor(0.0, 1.0, 0.0, 1.0);
  plwind(0.0, 1.0, 0.0, 1.0);
  plbox("bc", 0.0, 0, "bc", 0.0, 0);

  plsvpa(50.0, 150.0, 50.0, 100.0);
  plwind(0.0, 1.0, 0.0, 1.0);
  plbox("bc", 0.0, 0, "bc", 0.0, 0);
  plptex(0.5, 0.5, 1.0, 0.0, 0.5, "BOX at (50,150,50,100)");
  plend1();    
endfunction


