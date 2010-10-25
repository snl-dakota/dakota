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
## It is based on the corresponding demo function of PLplot, but doesn't
## use the tk driver.

#	Demo of multiple stream/window capability
#
## Plots several simple functions from other example programs.
#
## This version sends the output of the first 4 plots (one page) to two
## independent streams.  

1;

function ix14c

  if (!exist("plinit"))
    plplot_stub
  endif

  device = sprintf("%s",plgdev');
  if(isempty(device))
    device = "xwin";
    plsdev(device);
  endif

  xleng0 = 400; yleng0 = 300; xoff0 = 200; yoff0 = 200;
  xleng1 = 400; yleng1 = 300; xoff1 = 500; yoff1 = 500;

  ## The geometry strings MUST be in writable memory */

  geometry_master = "500x410+100+200";
  geometry_slave  = "500x410+650+200";

  printf("Demo of multiple output streams via the %s driver.\n", device);
  printf("Running with the second (right) window as slave.\n");
  printf("To advance to the next plot, press the third mouse button\n");
  printf("or the enter key in the first (left) window\n");

  fflush(stdout);

  ## Set up first stream */

  plSetOpt("geometry", geometry_master);
  plssub(2,2);
  plinit();

  ## Start next stream */

  plsstrm(1);

  ## Turn off pause to make this a slave (must follow master) */

  plSetOpt("geometry", geometry_slave);
  plspause(0);

  plsdev(device)
  plinit();

  ## Set up the data & plot */
  ## Original case */

  plsstrm(0);

  xscale = 6.;
  yscale = 1.;
  xoff = 0.;
  yoff = 0.;
  plot1(xoff,xscale,yoff,yscale);

  ## Set up the data & plot */

  xscale = 1.;
  yscale = 1.e+6;
  plot1(xoff,xscale,yoff,yscale);

  ## Set up the data & plot */

  xscale = 1.;
  yscale = 1.e-6;
  digmax = 2;
  plsyax(digmax, 0);
  plot1(xoff,xscale,yoff,yscale);

  ## Set up the data & plot */

  xscale = 1.;
  yscale = 0.0014;
  yoff = 0.0185;
  digmax = 5;
  plsyax(digmax, 0);
  plot1(xoff,xscale,yoff,yscale);

  ## To slave */
  ## The pleop() ensures the eop indicator gets lit. */

  plsstrm(1);
  plot4();
  pleop();

  ## Back to master */

  plsstrm(0);
  plot2();
  plot3();

  ## To slave */

  plsstrm(1);
  plot5();
  pleop();

  ## Back to master to wait for user to advance */

  plsstrm(0);
  pleop();

  ## Call plend1 to finish off. */
  ## close slave first
  plsstrm(1);
  plend1();
  ## close master
  plsstrm(0);
  plend1();
  
endfunction

function plot1(xoff,xscale,yoff,yscale)

  for i=0:59
    x(i+1) = xoff + xscale * (i + 1) / 60.0;
    y(i+1) = yoff + yscale * x(i+1).^2.;
  endfor

  xmin = min(x);
  xmax = max(x);
  ymin = min(y);
  ymax = max(y);

  for i=0:5
    xs(i+1) = x(i * 10 + 3 +1);
    ys(i+1) = y(i * 10 + 3 +1);
  endfor

  ## Set up the viewport and window using PLENV. The range in X is */
  ## 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are */
  ## scaled separately (just = 0), and we just draw a labelled */
  ## box (axis = 0). */

  plcol(1);
  plenv(xmin, xmax, ymin, ymax, 0, 0);
  plcol(6);
  pllab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2");

  ## Plot the data points */

  plcol(9);
  plpoin(xs', ys', 9);

  ## Draw the line through the data */

  plcol(4);
  plline(x', y');
  plflush;#pleop();
  
endfunction

function plot2()

  ## Set up the viewport and window using PLENV. The range in X is -2.0 to
  ##       10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
  ##       (just = 0), and we draw a box with axes (axis = 1).

  plcol(1);
  plenv(-2.0, 10.0, -0.4, 1.2, 0, 1);
  plcol(2);
  pllab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function");

  ## Fill up the arrays */

  for i=0:99
    x(i+1) = (i - 19.0) / 6.0;
    y(i+1) = 1.0;
    if (x(i+1) != 0.0)
      y(i+1) = sin(x(i+1)) / x(i+1);
    endif
  endfor

  ## Draw the line */

  plcol(3);
  plline(x', y');
  plflush;#pleop();
endfunction

function plot3()

  space0 = 0; mark0 = 0; space1 = 1500; mark1 = 1500;
  ## For the final graph we wish to override the default tick intervals, and
  ##       so do not use PLENV */

  pladv(0);

  ## Use standard viewport, and define X range from 0 to 360 degrees, Y range
  ##       from -1.2 to 1.2. */

  plvsta();
  plwind(0.0, 360.0, -1.2, 1.2);

  ## Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y. */

  plcol(1);
  plbox("bcnst", 60.0, 2, "bcnstv", 0.2, 2);

  ## Superimpose a dashed line grid, with 1.5 mm marks and spaces. plstyl
  ##       expects a pointer!! */

  plstyl(1, mark1, space1);
  plcol(2);
  plbox("g", 30.0, 0, "g", 0.2, 0);
  plstyl(0, mark0, space0);

  plcol(3);
  pllab("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function");

  for i = 0:100
    x(i+1) = 3.6 * i;
    y(i+1) = sin(x(i+1) * 3.141592654 / 180.0);
  endfor

  plcol(4);
  plline(x', y');
  plflush;#pleop();

endfunction

function plot4()

  dtr = 3.141592654 / 180.0;

  zz=0:360;
  x0 = cos(dtr * zz');
  y0 = sin(dtr * zz');


  ## Set up viewport and window, but do not draw box */

  plenv(-1.3, 1.3, -1.3, 1.3, 1, -2);
  
  for i = 1:10
    x = 0.1 * i * x0;
    y = 0.1 * i * y0;

    ## Draw circles for polar grid */

    plline(x, y);
  endfor
  
  plcol(2);
  for i = 0:11
    theta = 30.0 * i;
    dx = cos(dtr * theta);
    dy = sin(dtr * theta);

    ## Draw radial spokes for polar grid */

    pljoin(0.0, 0.0, dx, dy);
    text=sprintf("%d", round(theta));

    ## Write labels for angle */

    if (dx >= 0)
      plptex(dx, dy, dx, dy, -0.15, text);
    else
      plptex(dx, dy, -dx, -dy, 1.15, text);
    endif
  endfor

  ## Draw the graph */

  r = sin(dtr * (5 * zz'));
  x = x0 .* r;
  y = y0 .* r;
  
  plcol(3);
  plline(x, y);

  plcol(4);
  plmtex("t", 2.0, 0.5, 0.5,
	 "#frPLplot Example 3 - r(#gh)=sin 5#gh");
  plflush;#pleop();
endfunction

## Demonstration of contour plotting */

function plot5()

  XPTS=      35;
  YPTS=      46;
  XSPA=      2./(XPTS-1);
  YSPA=      2./(YPTS-1);

  tr=[XSPA, 0.0, -1.0, 0.0, YSPA, -1.0]';

  ## this is builtin in plplot_octave
  ## It is based on the corresponding demo function of PLplot.## 
  ## mypltr(PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, void *pltr_data)
  ## {
  ##    *tx = tr(0) * x + tr(1) * y + tr(2);
  ##    *ty = tr(3) * x + tr(4) * y + tr(5);
  ## }

  clevel =[-1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1.]';

  mark = 1500; space = 1500;

  ## Set up function arrays */

  for i = 0:XPTS-1
    xx = (i - (XPTS / 2)) / (XPTS / 2);
    for j = 0:YPTS-1
      yy = (j - (YPTS / 2)) / (YPTS / 2) - 1.0;
      z(i+1,j+1) = xx * xx - yy * yy;
      w(i+1,j+1) = 2 * xx * yy;
    endfor
  endfor

  plenv(-1.0, 1.0, -1.0, 1.0, 0, 0);
  plcol(2);
  plcont(z, 1, XPTS, 1, YPTS, clevel, tr, 0);
  plstyl(1, mark, space);
  plcol(3);
  plcont(w, 1, XPTS, 1, YPTS, clevel, tr, 0);
  plcol(1);
  pllab("X Coordinate", "Y Coordinate", "Streamlines of flow");
  plflush;#pleop();
endfunction

ix14c
