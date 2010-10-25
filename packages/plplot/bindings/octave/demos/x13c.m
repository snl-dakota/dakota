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
# Does a simple pie chart.

function x13c

  if (!exist("plinit"))
    plplot_stub
  endif

  if(isempty(sprintf("%s",plgdev')))
    plsdev("xwin");
  endif

  text =[
	 "Maurice",
	 "Geoffrey",
	 "Alan",
	 "Rafael",
	 "Vince"
	 ];

  per(1) = 10.;
  per(2) = 32.;
  per(3) = 12.;
  per(4) = 30.;
  per(5) = 16.;

  ## Parse and process command line arguments */

  ## (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

  ## Initialize plplot */
  plinit();

  plenv(0., 10., 0., 10., 1, -2);
  plcol(2);

  theta0 = 0.;
  dthet = 2 * pi / 500;
  for i=0:4
    x = [];
    y = [];
    j = 0;
    x(j+1) = 5.;
    y(j+1) = 5.;
    j=j+1;
    theta1 = theta0 + 2 * pi * per(i+1) / 100.;
    if (i == 4)
      theta1 = 2 * pi;
    endif
    for theta = theta0:dthet:theta1
      x(j+1) = 5 + 3 * cos(theta);
      y(j+1) = 5 + 3 * sin(theta);
      j=j+1;
    endfor
    plcol(i+1);
    plpsty(rem((i + 3), 8) + 1);
    x(j+1:rows(x))=[];
    y(j+1:rows(y))=[];
    plfill(x', y');
    plcol(1);
    plline(x', y');
    just = (theta0 + theta1) / 2.;
    dx = .25 * cos(just); dy = .25 * sin(just);
    if ((just < pi / 2) || (just > 3 * pi / 2) )
      just = 0.;
    else 
      just = 1.;
    endif
    plptex((x((j / 2) +1) + dx), (y((j / 2) +1) + dy), 1.0, 0.0, just, text(i+1,:));
    theta0 = theta - dthet;
  endfor
  plfont(2);
  plschr(0., 1.3);
  plptex(5.0, 9.0, 1.0, 0.0, 0.5, "Percentage of Sales");
  plend1();
endfunction
