## Copyright (C) 1998-2003 Joao Cardoso.
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

## usage: shg
##
## Show the graph window.  Currently, this is the same as executing
## replot without any arguments.

function shg ()

  if (nargin != 0)
    warning ("shg: ignoring extra arguments");
  endif

  __pl_init;

  plreplot;plflush;pleop;

endfunction
