## Copyright (C) 2002 Joao Cardoso.
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

function __pl_logplotit

  global __pl
  strm = __pl_init;

  ## plplot has no notion of log plots. Plot only positive values
  ## FIXME no matrix allowed!
  for i=1:__pl.items(strm)-1
    
    x = __pl.x{i, strm};
    y = __pl.y{i, strm};

    if (__pl.type(strm) == 10)
      if (any(x <= 0))
	expr = "ix = find(x > 0);";
      else
	break;
      endif
      
    elseif (__pl.type(strm) == 20)
      if (any(y <= 0))
	expr = "ix = find(y > 0);";
      else
	break;
      endif
    elseif (__pl.type(strm) == 30)
      if (any(y <= 0) || any(x <= 0))
	expr = "ix = find(y > 0 & x > 0);";
      else
	break;
      endif
    endif

    try 
      k = __pl.items(strm);
      while(!isempty(eval(expr)))
	ixx = ix(find(ix == (min(ix):rows(ix)+min(ix)-1)'));
	__pl.x{k, strm} = x(ixx,:);
	__pl.y{k, strm} = y(ixx,:);
	__pl.fmt{k, strm} = __pl.fmt{i, strm};

	## replace label string by special mark: check first that it is not empty!!
	nofmt = isempty(__pl.fmt{k, strm});
	if (nofmt == 1) ## no format string, supply an empty one
	  __pl.fmt{k, strm} = ';;';
	else
	  haslab = index(__pl.fmt{k, strm},';');
	  if (haslab == 0) ## format string has no label string, use original format plus empty label string
	    __pl.fmt{i, strm} = __pl.fmt{k, strm};
	    __pl.fmt{k, strm} = [__pl.fmt{k, strm} ';;'];
	  else ## format string has label, replace label string with an empty one
	    __pl.fmt{k, strm}(index(__pl.fmt{k, strm}, ';')+1:rindex(__pl.fmt{k, strm},';')-1) = '';
	    __pl.fmt{k, strm}(index(__pl.fmt{k, strm}, ';')+2:rindex(__pl.fmt{k, strm},';')-1) = '';
	  endif
	endif
	x(ixx,:) = y(ixx,:) = []; k++;
      endwhile
      k--;
      __pl.x{i, strm} = __pl.x{k, strm};
      __pl.y{i, strm} = __pl.y{k, strm};
      __pl.items(strm) = k;
    catch
      error("__pl_plotit: FIXME. Logplot of matrices with negative elements is in some cases not allowed.\n");
    end
    
  endfor

endfunction