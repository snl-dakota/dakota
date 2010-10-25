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

function __pl_draw_legend(xm, xM, ym, yM)

  global __pl

  strm = plgstrm+1;

  [a, b] = plgchr;

  scale = 100; # why 100? look at plplot plpage.c plP_subpInit() and tell me :)
  ch_height = 1.2 * b/scale * (__pl.multi_col(strm));
  ch_width = b/scale * (__pl.multi_row(strm));

  ## if (!ishold)
  ##	__pl.lab_str(1,:) = "";
  ## endif

  plwind(-0.01, 1.01, -0.01, 1.01);	# this makes it independent of the scale

  llm = ch_width * columns(__pl.lab_str)/2;
  xxl= __pl.legend_xpos(strm) - llm * __pl.legend_xpos(strm);
  xxr= __pl.legend_xpos(strm) + llm * (1-__pl.legend_xpos(strm));
  yyt = __pl.legend_ypos(strm);

  k = 1;
  for i=1:__pl.lab_pos(strm)-1
    if (length(tdeblank(__pl.lab_str(i,:))) != 0);
      k++;
    endif
  endfor
  k--;

  yyb = (__pl.legend_ypos(strm) - ch_height * k);

  if (__pl.legend(strm) == 2)	# opaque
    plcol(0);	# background color (erase lines behind legend)
    plfill([xxl;xxr;xxr;xxl], [yyb; yyb; yyt; yyt])
    plcol(15);pllsty(1);
    plline([xxl;xxr;xxr;xxl;xxl], [yyb; yyb; yyt; yyt;yyb])
  endif

  if( __pl.type(strm) == -2)
    minx = xxl; maxx = xxr;
    miny = yyb; maxy = yyt;

    n = __pl.lab_pos(strm)-1;
    clevel = linspace(0,1,n+1);
    z = fliplr([clevel; clevel]);

    min_color = 0; max_color = 0; min_width = 0; max_width = 0;
    tr = [(maxx-minx); 0; minx; 0; (maxy-miny)/(n); miny];

    if(n>2) ## FIXME : sometimes n=1, which cause a division by zero
    for i = 1:n
      plshade(z, 0, minx, maxx, miny, maxy, 
	      clevel(i), clevel(i+1),
	      1, (i-1) / (n-1), 1,
	      min_color, min_width, max_color, max_width, 1, tr);
    endfor
    endif
    plcol(15);pllsty(1);
    plline([xxl;xxr;xxr;xxl;xxl], [yyb; yyb; yyt; yyt;yyb]);
  endif

  k=0.5;	
  for i=1:__pl.lab_pos(strm)-1
    if (length(tdeblank(__pl.lab_str(i,:))) != 0)
      xx = __pl.legend_xpos(strm);
      yy = (__pl.legend_ypos(strm) - ch_height * k++ );
      
      plcol(15);
      plptex(xx, yy, 0, 0, __pl.legend_xpos(strm), __pl.lab_str(i,:));
      if(__pl.type(strm) != -2)
			plcol(__pl.lab_col(strm,i)); pllsty(__pl.lab_lsty(strm,i));
			plline([xxl; xxr],[yy-ch_height/2; yy-ch_height/2]);
			try
				if (__pl.lab_sym(strm,i) != 0)
	    			plpoin(xxl + ch_height/3 ,yy , __pl.lab_sym(strm,i));
	  		endif
			catch; end_try_catch
      endif
    endif
  endfor

  ## restore size
  plwind(__pl.axis(strm,1), __pl.axis(strm,2),
	 __pl.axis(strm,3), __pl.axis(strm,4))

endfunction
