## Copyright (C) 2003 Joao Cardoso.
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

function __pl_meshplotit()

  global __pl
  global DRAW_LINEXY MAG_COLOR BASE_CONT
  global TOP_CONT SURF_CONT DRAW_SIDES FACETED MESH

  strm = __pl_init;

  xm = __pl.lxm(strm); xM = __pl.lxM(strm); 
  ym = __pl.lym(strm); yM = __pl.lyM(strm); 
  zm = __pl.lzm(strm); zM = __pl.lzM(strm);

  if (xm == xM)
    xM = xm +10*eps;
  endif
  if (ym == yM)
    yM = ym +10*eps;
  endif
  if (zm == zM)
    zM = zm +10*eps;
  endif

  if (__pl.axis_st(strm))
    xm = __pl.axis(strm,1); xM = __pl.axis(strm,2);	# at least x always exist

    if (length(__pl.axis) >= 4)	
      ym = __pl.axis(strm,3); yM = __pl.axis(strm,4);
    else
      __pl.axis(strm,3) = ym; __pl.axis(strm,4) = yM;
    endif
    if (length(__pl.axis) == 6)
      zm = __pl.axis(strm,5); zM = __pl.axis(strm,6);
    else
      __pl.axis(strm,5) = zm; __pl.axis(strm,6) = zM;		
    endif
  else	# make axis() return current axis
    __pl.axis(strm,1) = xm; __pl.axis(strm,2) = xM;
    __pl.axis(strm,3) = ym; __pl.axis(strm,4) = yM;
    __pl.axis(strm,5) = zm; __pl.axis(strm,6) = zM;		
  endif
  
  plcol(15);pllsty(1);
  if (__pl.multi(strm) == 1)	# multiplot, erase current subwindow
    plclear;
  else
    pladv(0);
  endif

  plvpor(0, 1, 0, 0.95);
  plwind(-1.6, 1.6, -1.3, 2.2);
  plw3d(2, 2, 2, xm, xM, ym, yM, zm, zM, __pl.alt(strm), __pl.az(strm))
  plbox3("bnstu", tdeblank(__pl.xlabel(strm,:)), 0.0, 0,
	 "bnstu", tdeblank(__pl.ylabel(strm,:)), 0.0, 0,
	 "bcmnstuv", tdeblank(__pl.zlabel(strm,:)), 0.0, 0);
  plcol(1)

  nlev = 10;
  st = (zM-zm)/nlev/2;
  clevel = (linspace(zm+st, zM-st, nlev))';
  levs = 0;

  shad = tdeblank(__pl.shading(strm,:));
  switch (shad)
    case "flat"
      sh = 0;
    case "faceted"
      sh = FACETED;
    case "contour"
      sh = SURF_CONT;
      levs = clevel;
  endswitch

  for items=1:__pl.items(strm)-1
    x = __pl.x{items, strm};
    y = __pl.y{items, strm};
    z = __pl.z{items, strm};

    ## kludge, use "fmt" as plot type. But __pl_plotit still uses __pl.type
    type = __pl.fmt{items, strm};

  switch (type)
    case 100  ## mesh
      plmesh(x, y, z', DRAW_LINEXY + MAG_COLOR);
    case 101  ## meshc
      plmeshc(x, y, z', DRAW_LINEXY + MAG_COLOR + BASE_CONT, clevel);
    case 102  ## meshz
      plot3d(x, y, z', DRAW_LINEXY + MAG_COLOR, 1); 
    case 103  ## surf
      plsurf3d(x, y, z', MAG_COLOR + sh, clevel);
    case 104   ## surfc
      plsurf3d(x, y, z', MAG_COLOR + BASE_CONT + sh, clevel) 
    case 105   ## surfl
      pllightsource(__pl.light(1), __pl.light(2), __pl.light(3));
      plsurf3d(x, y, z', sh, levs);
  endswitch	

  endfor
  
  plcol(15);
  plmtex("t", 1, 0.5,0.5, tdeblank(__pl.tlabel(strm,:)));
  plflush;

endfunction