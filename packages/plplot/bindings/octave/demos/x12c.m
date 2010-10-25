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

## Does a simple bar chart, using color fill.  If color fill is
## unavailable, pattern fill is used instead (automatic).

1;

function ix12c

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
  plvsta();
  plwind(1980.0, 1990.0, 0.0, 35.0);
  plbox("bc", 1.0, 0, "bcnv", 10.0, 0);
  plcol(2);
  pllab("Year", "Widget Sales (millions)", "#frPLplot Example 12");

  y0 = [5; 15; 12; 24; 28;30; 20; 8; 12; 3];

  for i=0:9
    plcol(i + 1);
    
    ## plcol1((PLFLT) ((i + 1)/10.0));

    plpsty(0);
    plfbox((1980. + i), y0(i+1));
    string=sprintf("%.0f", y0(i+1));
    plptex((1980. + i + .5), (y0(i+1) + 1.), 1.0, 0.0, .5, string);
    string=sprintf("%d", 1980 + i);
    plmtex("b", 1.0, ((i + 1) * .1 - .05), 0.5, string);
  endfor
  plend1();
endfunction

function plfbox(x0, y0)

  x(1) = x0;
  y(1) = 0.;
  x(2) = x0;
  y(2) = y0;
  x(3) = x0 + 1.;
  y(3) = y0;
  x(4) = x0 + 1.;
  y(4) = 0.;
  plfill(x', y');
  plcol(1);
  pllsty(1);
  plline(x', y');
  
endfunction

ix12c

