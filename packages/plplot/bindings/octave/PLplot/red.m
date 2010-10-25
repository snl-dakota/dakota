## Copyright (C) 1999 Paul Kienzle
##
## This program is free software and may be used for any purpose.  This
## copyright notice must be maintained.  Paul Kienzle is not responsible
## for the consequences of using this software.

## Colormap:
##
## map = red (number)

## Author: Paul Kienzle <pkienzle@cs.indiana.edu>
## Created: Sept 99
## modified: jc

function map = red (number)

  if (nargin == 0)
    number = 64;
  elseif (nargin > 1)
    usage ("red (number)");
  endif

  gr = [0:(number-1)]';

  map = [ gr, zeros(number,1), zeros(number,1) ] / (number - 1);

endfunction
