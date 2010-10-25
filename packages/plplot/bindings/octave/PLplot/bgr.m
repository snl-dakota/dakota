## Copyright (C) 2002-2003 Joao Cardoso.
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

## map = bgr(n)
##
## return a colormap of size 'n' going from blue to green to red.
## Good for coloring magnitude colored surfaces.
## Without arguments, map has lenght 64

function map = bgr(n)

  if (nargin != 1)
    n = 64;
  endif

  i = fix(linspace (1, n, 5));

  rg = i(1):i(2);
  r(1,rg) = 0;
  g(1,rg) = linspace(0,1,length(rg));
  b(1,rg) = 1;
  
  rg = i(2):i(3);
  r(1,rg) = 0;
  g(1,rg) = 1;
  b(1,rg) = linspace(1,0,length(rg));
  
  rg = i(3):i(4);
  r(1,rg) = linspace(0,1,length(rg));
  g(1,rg) = 1;
  b(1,rg) = 0;

  rg = i(4):i(5);
  r(1,rg) = 1;
  g(1,rg) = linspace(1,0,length(rg));
  b(1,rg) = 0;

  map = [r', g', b'];
endfunction
