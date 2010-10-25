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
## Does a series of 3-d plots for a given data set, with different
## viewing options in each plot.

1;

function cmap1_init(g)
  i = [0; 1];           # left boundary, right boundary
  if (g)
    h = [0; 0]; 	# hue -- low: red (arbitrary if s=0) -- high: red (arbitrary if s=0)
    l = [0.5; 1];	# lightness -- low: half-dark -- high: light
    s = [0; 0];		# minimum saturation 
  else
    h = [240; 0];       # blue -> green -> yellow -> red
    l = [0.6; 0.6];
    s = [0.8; 0.8];
  endif

  plscmap1n(256);
  plscmap1l(0, i, h, l, s, zeros(2,1));
endfunction

  function ix08c

    if (!exist("plinit"))
      plplot_stub
    endif

    if(isempty(sprintf("%s",plgdev')))
      plsdev("xwin");
    endif

    XPTS=35;		## Data points in x
    YPTS=46;		## Datat points in y

    DRAW_LINEXY = 3;
    MAG_COLOR = 4;
    BASE_CONT = 8;
    SURF_CONT = 32;
    FACETED = 128;

    alt=[60.0, 20.0];
    az =[30.0, 60.0];

    title=["#frPLplot Example 8 - Alt=60, Az=30",
	   "#frPLplot Example 8 - Alt=20, Az=60"];

    ## Parse and process command line arguments

    ## (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

    ## Initialize plplot
    plinit();

    rosen = 1;

    x = ((0:XPTS-1) - (XPTS / 2)) / (XPTS / 2);
    y = ((0:YPTS-1) - (YPTS / 2)) / (YPTS / 2);
    if (rosen)
      x = x * 1.5;
      y = y + 0.5;
    endif
    [xx, yy] = meshgrid(x,y);

    if (rosen)
      z = (1 - xx) .^ 2 + 100 .* (yy - xx .^ 2) .^ 2;
      if exist ("do_fortran_indexing")
      of = do_fortran_indexing;
      do_fortran_indexing = 1;
      endif
      z(z <= 0) = exp(-5); # make sure the minimum after applying log() is -5
      if exist ("do_fortran_indexing")      
      do_fortran_indexing = of;
      endif
      z = log(z);
    else
      r = sqrt(xx .* xx + yy .* yy);
      z = exp(-r .* r) .* cos(2.0 * 3.141592654 .* r);
    endif

    pllightsource(1.,1.,1.);

    n_col = 100;
    plscmap1n(n_col);
    
    nlevel = 10;
    zmax = max(max(z));
    zmin = min(min(z));
    step = (zmax-zmin)/(nlevel+1);
    clevel = linspace(zmin+step, zmax-step, nlevel)';

    for k=1:2
      for ifshade=0:3

	pladv(0);
	plvpor(0.0, 1.0, 0.0, 0.9);
	plwind(-1.0, 1.0, -0.9, 1.1);
	plcol0(3);
	plmtex("t", 1.0, 0.5, 0.5, title(k,:));
	plcol0(1);
	if (rosen)
          plw3d(1.0, 1.0, 1.0, -1.5, 1.5, -0.5, 1.5, zmin, zmax, \ 
                alt(k), az(k));
	else
	  plw3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, zmin, zmax, \
		alt(k), az(k));
	endif

	plbox3("bnstu", "x axis", 0.0, 0,
	       "bnstu", "y axis", 0.0, 0,
	       "bcdmnstuv", "z axis", 0.0, 0);      
	plcol0(2);

	switch(ifshade)
	  case 0
	    cmap1_init(1);
	    plsurf3d(x', y', z', 0, 0, 0);
	  case 1
	    cmap1_init(0);
	    plsurf3d(x', y', z', MAG_COLOR, 0, 0);
	  case 2
	    plsurf3d(x', y', z', MAG_COLOR + FACETED, 0, 0);
	  otherwise
	    plsurf3d(x', y', z', MAG_COLOR + SURF_CONT + BASE_CONT, clevel);
	endswitch
      endfor
    endfor
    plend1();
  endfunction

ix08c
