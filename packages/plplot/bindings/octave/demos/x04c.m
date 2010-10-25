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

## Illustration of logarithmic axes, and redefinition of window.

## this file defines several functions:
1;

function ix04c

  ## Parse and process command line arguments */

  ## (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

  ## Initialize plplot */
  if (!exist("plinit"))
    plplot_stub
  endif

  if(isempty(sprintf("%s",plgdev')))
    plsdev("xwin");
  endif

  plinit();
  plfont(2);

  ## Make log plots using two different styles. */

  plot1(0);
  plot1(1);
  plend1();
endfunction

## Log-linear plot.

function plot1(type)

  pladv(0);

  ## Set up data for log plot */

  f0 = 1.0;
  i=1:100;
  freql = -2.0 + i / 20.0;
  freq = 10.0 .^ freql;
  ampl = 20.0 * log10(1.0 ./ sqrt(1.0 + (freq ./ f0).^ 2.));
  phase = -(180.0 / 3.141592654) * atan(freq ./ f0);

  plvpor(0.15, 0.85, 0.1, 0.9);
  plwind(-2.0, 3.0, -80.0, 0.0);

  ## Try different axis and labelling styles. */

  plcol0(1);
  switch (type)

    case 0
      plbox("bclnst", 0.0, 0, "bnstv", 0.0, 0);

    case 1
      plbox("bcfghlnst", 0.0, 0, "bcghnstv", 0.0, 0);

  endswitch

  ## Plot ampl vs freq */

  plcol0(2);
  plline(freql', ampl');
  plcol0(1);
  plptex(1.6, -30.0, 1.0, -20.0, 0.5, "-20 dB/decade");

  ## Put labels on */

  plcol0(1);
  plmtex("b", 3.2, 0.5, 0.5, "Frequency");
  plmtex("t", 2.0, 0.5, 0.5, "Single Pole Low-Pass Filter");
  plcol0(2);
  plmtex("l", 5.0, 0.5, 0.5, "Amplitude (dB)");

  ## For the gridless case, put phase vs freq on same plot */

  if (type == 0) 
    plcol0(1);
    plwind(-2.0, 3.0, -100.0, 0.0);
    plbox("", 0.0, 0, "cmstv", 30.0, 3);
    plcol0(3);
    plline(freql', phase');
    plcol0(3);
    plmtex("r", 5.0, 0.5, 0.5, "Phase shift (degrees)");
  endif

endfunction


ix04c


