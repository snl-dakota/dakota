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

## closeallfig()
##
## Close all plplot windows

function closeallfig

  global __pl

  if (!exist("__pl") || !struct_contains (__pl,"inited"))
    return
  endif

  ## another atexit bug!
  ## if rows() has not been used when closeallfig is called, an error occurs here
  [nr, nc] = size(__pl.open);

  for i=1:nr
    if (__pl.open(i))
      closefig(i-1);
    endif
  endfor

endfunction
