## Copyright (C) 1998-2003  Joao Cardoso.
## Copyright (C) 2004  Rafael Laboissiere
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

function __pl_contour(x, y, z, n)

  global __pl

  strm = plgstrm+1;

  old_empty_list_elements_ok = warn_empty_list_elements;

  unwind_protect
    
  warn_empty_list_elements = 0;

  grid = 0;
  if (__pl.grid(strm))
    grid = 2;
  endif

  if (ishold == 0)
    xm = min(min(x)); xM = max(max(x));
    ym = min(min(y)); yM = max(max(y));
    zm = min(min(z)); zM = max(max(z));

    if (__pl.axis_st(strm))
      ## at least x always exist
      xm = __pl.axis(strm,1); xM = __pl.axis(strm,2);	
      z = z(find(x >= xm & x <= xM), :);

      if (length(__pl.axis) >= 4)	
	ym = __pl.axis(strm,3); yM = __pl.axis(strm,4);
	z = z(:, find(y >= ym & y <= yM));
      else
	__pl.axis(strm,3) = ym; __pl.axis(strm,4) = yM;
      endif
      if (length(__pl.axis) == 6)
	zm = __pl.axis(strm,5); zM = __pl.axis(strm,6);
      else
	__pl.axis(strm,5) = zm; __pl.axis(strm,6) = zM;		
      endif
    else	# make axis() return current axis
      __pl.axis(strm,1) = xm; __pl.axis(strm,2) = xM;
      __pl.axis(strm,3) = ym; __pl.axis(strm,4) = yM;
      __pl.axis(strm,5) = zm; __pl.axis(strm,6) = zM;		
    endif
    
    __pl.type(strm) = 0; ## was -3 ??
    __pl.plcol(strm) = 1;
    __pl.pllsty(strm) = 1;	
    __pl.lab_pos(strm) = 1;
    __pl.lab_str = "";
    plcol(15); pllsty(1);
    __pl_plenv(xm, xM, ym, yM, 0, grid);
  else
    if (columns(__pl.axis(strm,:)) != 6)
      error("You must contour/shade plot something before entering hold mode");
    endif
    xm = __pl.axis(strm,1); xM = __pl.axis(strm,2);
    ym = __pl.axis(strm,3); yM = __pl.axis(strm,4);
    zm = __pl.axis(strm,5); zM = __pl.axis(strm,6);
    z = z(find(x >= xm & x <= xM), find(y >= ym & y <= yM));
  endif

  [xlen, ylen] = size(z);
  tr = [(xM-xm)/(xlen-1); 0; xm; 0; (yM-ym)/(ylen-1); ym];

  if (!is_scalar(n))
    st = (zM-zm)/length(n)/2;
    clevel = n; n = length(n);
  else
    st = (zM-zm)/n/2;
    clevel = (linspace(zm+st, zM-st, n))';
  endif	

  for i=1:n
    plcol(__pl.plcol(strm)); pllsty(__pl.pllsty(strm));
    plcont(z, 1, xlen, 1, ylen, clevel(i), tr);
    __pl.lab_str = [__pl.lab_str; sprintf("%#+.2G", clevel(i))];
    __pl.lab_col(strm,__pl.lab_pos(strm)) = __pl.plcol(strm);
    __pl.lab_lsty(strm,__pl.lab_pos(strm)) = __pl.pllsty(strm);
    __pl.lab_pos(strm) = __pl.lab_pos(strm) + 1;				
    __pl.plcol(strm) = rem(__pl.plcol(strm), 15)+1;
    if  (__pl.line_style(strm))
      __pl.pllsty(strm) = rem(__pl.pllsty(strm), 8)+1;
    endif
  endfor

  ## to not mess up the legend, print it only after ploting all contour lines	
  if (__pl.legend(strm))
    __pl_draw_legend
  endif

  plcol(15);
  pllab(tdeblank(__pl.xlabel(strm,:)),
	tdeblank(__pl.ylabel(strm,:)),
	tdeblank(__pl.tlabel(strm,:)));
  plflush;

  __pl.items(strm) = 1; # for now!

  unwind_protect_cleanup  
  
  warn_empty_list_elements = old_empty_list_elements_ok;

  end_unwind_protect  

endfunction
