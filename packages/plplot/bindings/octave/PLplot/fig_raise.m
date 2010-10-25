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

function fig_raise(fig)

  ## fig_raise([figure])
  ##
  ## raise current (or "figure") plot window  

  if (nargin == 0)
    [id driver intp] = figure;
  else
    [id driver intp] = figure(fig);
  endif
    
  if (strcmp(driver, "tk"))
    tk_cmd(intp,"raise .",0);
    figure(id)
  else
    error("fig_raise can only be used with the tk driver.\n");
  endif

endfunction
