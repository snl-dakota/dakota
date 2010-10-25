## Copyright (C) 1999 Paul Kienzle
##
## This program is free software and may be used for any purpose.  This
## copyright notice must be maintained.  Paul Kienzle is not responsible
## for the consequences of using this software.

## Colormap.
##
## map = green (number)

## Author: Paul Kienzle <pkienzle@cs.indiana.edu>
## Created: Sept 99
## Modified: jc

function map = green (number)

  if (nargin == 0)
    number = 64;
  elseif (nargin > 1)
    usage ("green (number)");
  endif

  gr = [0:(number-1)]';

  map = [ zeros(number,1), gr, zeros(number,1) ] / (number - 1);

endfunction
