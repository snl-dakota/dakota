## Copyright (C) 1998-2003 Joao Cardoso.
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

## closefig([n])
##
## Close plot window n, or current plot windows if nargin = 0.
## Makes the lowest figure the current figure, if it exists.

function closefig(n)

  global __pl

  if ( !(exist "__pl") || !struct_contains (__pl,"inited"))
    return;
  endif

  ## old_fig = plgstrm;
  old_fig = figure;

  if (nargin == 0)
    n = old_fig;
  endif

  plsstrm(n);
  plend1;
  __pl.open(n+1) = 0;

  if ( n != old_fig)
    if (__pl.open(old_fig+1))
      plsstrm(old_fig);
      figure(old_fig);
    endif
  else
    n = min(find(__pl.open == 1));
    if (isempty(n))
      return;
    endif
    plsstrm(n-1);
    figure(n-1);
  endif

endfunction
