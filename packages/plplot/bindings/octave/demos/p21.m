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


function p21

  global GRID_CSA GRID_DTLI GRID_NNI GRID_NNIDW GRID_NNLI GRID_NNAIDW

  ##ocmap = colormap;
  colormap(bgr);

  t = automatic_replot;
  automatic_replot = 0;

  gx = 30;
  gy = 40;
  n = 600;
  alg = ["Cubic Spline Approximation",
	 "Delaunay Linear Interpolation",
	 "Natural Neighbors Interpolation",
	 "KNN Inv. Distance Weighted",
	 "3NN Linear Interpolation",
	 "4NN Around Inv. Dist. Weighted"];

  opt = [0, 0, -1, 50, 1.001, 0];

  x = rand(n,1)*6-3;
  y = rand(n,1)*6-3;
  z = 3 * (1-x).^2 .* exp(-(x.^2) - (y+1).^2) - ...
      10 * (x/5 - x.^3 - y.^5) .* exp(-x.^2 - y.^2)- ...
      1/3 * exp(-(x+1).^2 - y.^2);
  
  xg = linspace (-2.5, 3, gx)';
  yg = linspace (-3, 2.5, gy)';
  multiplot(3,2);

  j = 1;
  for i = [GRID_CSA GRID_DTLI GRID_NNI GRID_NNIDW GRID_NNLI GRID_NNAIDW]
    pladv(j++);
    zg = griddata(x, y, z, xg, yg, i, opt(i));
    ## zg(isnan(zg)) = 0;
    ofi = do_fortran_indexing;  do_fortran_indexing = 1;
    zg(isnan(zg)) = 0;
    do_fortran_indexing = ofi;
    title(alg(i,:));
    meshc(xg, yg, zg');
  endfor

  automatic_replot = t;
  ##colormap(ocmap);

endfunction