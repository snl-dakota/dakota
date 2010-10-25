## Copyright (C) 2003 Joao Cardoso.
## 
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## This file is part of plplot_octave.

## usage: zg = griddata(x, y, z, xg, yg [,method [, arg]])
##
## griddata takes irregularly sampled data from three input arrays
##    x[npts], y[npts], and z[npts], reads the desired grid location from
##   input arrays xg[nptsx] and yg[nptsy], and returns the gridded data 
##   into output array zg[nptsx][nptsy].
##
##   The algorithm used to grid the data is specified with the argument 
##   method. The algorithm  can use argument arg. 
##   The available(*) algorithms are:
##
##         GRID_CSA: Bivariate Cubic Spline approximation
##         GRID_DTLI: Delaunay Triangulation Linear Interpolation 
##         GRID_NNI: Natural Neighbors Interpolation 
##         GRID_NNIDW: Nearest Neighbors Inverse Distance Weighted
##         GRID_NNLI: Nearest Neighbors Linear Interpolation 
##         GRID_NNAIDW:  Nearest Neighbors Around Inverse Distance Weighted (default) 
##
##   Some algorithms can generate NaNs, and as PLplot is not NaN aware the
##   returned data must be treated before being plotted, e.g., zg(isnan (zg)) = 0;
##
##   (*) Some algorithms might not be present in your system, depending
##    on the availability of the QHull library.
##    GRID_CSA, GRID_DTLI and GRID_NNI use the csa and nn library from 
##    Pavel Sakof, http://www.marine.csiro.au/~sakov/

function  zg = griddata(x, y, z, xg, yg, method, arg)

  __pl_init;

  global GRID_CSA GRID_DTLI GRID_NNI GRID_NNIDW GRID_NNLI GRID_NNAIDW

  if (nargin < 5)
    help griddata
    return;
  endif

  if (is_vector(x) && is_vector(y) && is_vector(z) &&
      is_vector(xg) && is_vector(yg))
    
    [err, x, y, z] = common_size(x, y, z);
    if (err)
      help griddata
      return;
    endif

    if (nargin == 5)
      method = GRID_NNAIDW;
      arg = 15;
    elseif (nargin == 6)
      arg = 0;
    endif

    if (! any(method == [GRID_CSA GRID_DTLI GRID_NNI GRID_NNIDW GRID_NNLI GRID_NNAIDW]))
      help griddata
      return
    endif

    if (rows(xg) == 1)
      xg = xg';
    endif

    if (rows(yg) == 1)
      yg = yg';
    endif

    zg = plgriddata(x, y, z, xg, yg, method, arg);
    ##if (any(any(isnan(zg))))
    ##  warning("griddata: NaNs returned.");
    ##endif

  else
    help griddata
    return
  endif 

endfunction