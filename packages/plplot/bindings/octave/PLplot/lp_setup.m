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

## lp_setup(command)
##
## set your printer command and options for use with `save_fig'
## default is "lpr".
## You must specify the lp command also, e.g. `lp_setup("lpr -n 2 -P lj3")'

function out = lp_setup(command)

  global __pl

  if (!exist("__pl.lp_options"))
    __pl.lp_options = "lpr";
  endif

  if (nargin == 1 && isstr(command))
    __pl.lp_options = command;
  endif

  out = __pl.lp_options;

endfunction
