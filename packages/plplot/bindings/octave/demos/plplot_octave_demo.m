## Copyright (C) 1998, 1999, 2000 Joao Cardoso.
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

printf("\n\n\
This demo runs script files x01c.m to x19c.m, which are the octave\n\
versions of the equivalent C files.\n\
The Octave version is an almost direct copy of the C sources,\n\
 and only in a few cases the code is vectorized.\n\n\
To advance from one plot to the next, hit the Enter key or\n\
press the right mouse button. If you endup with an undesired\n\
plot window, type at the Octave prompt \"plend1\".\n\n\
Some demos print instructions, so keep this window visible.\n\n\
Press any key to continue...\n");

fflush(stdout);
kbhit;

plplot_stub;

for i=[1:19];
  ## To set defaults, use plSetOpt, as used in the command line, e.g.
  ## plSetOpt("dev", "png");
  ## plSetOpt("o", "foo.ps");
  ## plSetOpt("fam", "1");
  
  plSetOpt("dev", "xwin");

  cmd = sprintf("x%.2dc",i);
  printf("Demo %s\n", cmd);
  fflush(stdout);
  eval(cmd);
endfor

