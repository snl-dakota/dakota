## Copyright (C) 2000-2003 Joao Cardoso.
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

## plimage(img, xi, xe, yi, ye)
##
## Plot an image, assuming that it is in octave image format, where each
##   matrix element is an index in the colormap.
##   if xi, xe, yi and ye exists, and are smaller than the image dimension
##   in pixels, than only that portion of the image will be shown.
##   No smoothing will be performed!
## Preliminary! see p17.m
##
## ex:
##   [img, map] = loadimage("default.img"); # "lena.img" can also be used
##   colormap(map);
##   plimage(img);

function plimage(img, x1, x2, y1, y2)

  global __pl
  strm = __pl_init;
  
  [nr, nc] = size(img);
  if (nargin == 1)
    xi = yi = 1;
    xf = nc;
    yf = nr;
  elseif (nargin == 5)
    if (x1 < 1 || x1 > nc || x2 < 1 || x2 > nc ||
	y1 < 1 || y1 > nr || y1 < 1 || y2 > nr)
      usage "plimage";
      return;
    else
      xi = min([x1, x2]); 
      xf = max([x1, x2]); 
      yi = min([y1, y2]); 
      yf = max([y1, y2]); 
    endif
  else
    usage "plimage";
    return;
  endif

  __pl_plenv(xi, xf, yi, yf, 1, -1);

  pplimage (fliplr(img'),
	    1, nc, 1, nr,
	    0, 0,
	    xi, xf, yi, yf);

  pllab(tdeblank(__pl.xlabel(strm,:)),
	tdeblank(__pl.ylabel(strm,:)),
	tdeblank(__pl.tlabel(strm,:)));
  plflush;pleop;

  __pl.items(strm) = 1; # for now!

endfunction
