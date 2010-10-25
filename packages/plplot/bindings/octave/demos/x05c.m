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

## Draws a histogram from sample data.

function x05c

  ## Initialize plplot */
  if (!exist("plinit"))
    plplot_stub
  endif

  if(isempty(sprintf("%s",plgdev')))
    plsdev("xwin");
  endif

  plinit;

  NPTS=2047;

  ## Parse and process command line arguments */

  ## (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

  ## Fill up data points */

  delta = 2.0 * pi /  NPTS;
  i=1:NPTS;
  data = sin(i * delta);
  
  plcol0(1);
  plhist(data', -1.1, 1.1, 44, 0);
  plcol0(2);
  pllab("#frValue", "#frFrequency",
	"#frPLplot Example 5 - Probability function of Oscillator");
  plend1();
endfunction
