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

## Does a series of mesh plots for a given data set, with different
## viewing options in each plot.

1;

function cmap1_init()
  i = [0; 1];           # left boundary, right boundary

  h = [240; 0];       # blue -> green -> yellow -> red
  l = [0.6; 0.6];
  s = [0.8; 0.8];

  plscmap1n(256);
  plscmap1l(0, i, h, l, s, zeros(2,1));
endfunction

function ix11c

  if (!exist("plinit"))
    plplot_stub
  endif

  if(isempty(sprintf("%s",plgdev')))
    plsdev("xwin");
  endif

  XPTS = 35;		## Data points in x */
  YPTS = 46;		## Datat points in y */

  DRAW_LINEXY = 3;
  MAG_COLOR = 4;
  BASE_CONT = 8;

  opt = [DRAW_LINEXY,  DRAW_LINEXY ]';
  alt = [33, 17]';
  az = [24, 115]';

  title = ["#frPLplot Example 11 - Alt=33, Az=33, Opt=3",
	   "#frPLplot Example 11 - Alt=24, Az=115, Opt=3"];

  ## Parse and process command line arguments */

  ## (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

  ## Initialize plplot */
  plinit();

  xx = linspace(-3, 3, XPTS);
  yy = linspace(-3, 3, YPTS);
  [x,y] = meshgrid (xx,yy);

  z = 3 * (1-x).^2 .* exp(-(x.^2) - (y+1).^2) - ...
      10 * (x/5 - x.^3 - y.^5) .* exp(-x.^2 - y.^2)- ...
      1/3 * exp(-(x+1).^2 - y.^2);

  nlevel = 10;
  zmax = max(max(z));
  zmin = min(min(z));
  step = (zmax-zmin)/(nlevel+1);
  clevel = linspace(zmin+step, zmax-step, nlevel)';

  cmap1_init;

  for k=1:2
    for i=0:3
      pladv(0);
      plcol0(1);
      plvpor(0, 1, 0, 0.9);
      plwind(-1, 1, -1, 1.5);
      
      plw3d(1, 1, 1.2, -3, 3, -3, 3, zmin, zmax, alt(k), az(k));
      plbox3("bnstu", "x axis", 0, 0,
	     "bnstu", "y axis", 0, 0,
	     "bcdmnstuv", "z axis", 0, 4);
      
      plcol0(2);

      switch (i)
	case 0
	  plmesh(xx', yy', z', opt(k));
	case 1
	  plmesh(xx', yy', z', opt(k) + MAG_COLOR);
	case 2
	  plot3d(xx', yy', z', opt(k) + MAG_COLOR, 1);
	case 3
	  plmeshc(xx', yy', z', opt(k) + MAG_COLOR + BASE_CONT, clevel);
      endswitch

      plcol0(3);
      plmtex("t", 1.0, 0.5, 0.5, title(k,:));
    endfor
  endfor
  plend1();
endfunction

ix11c
