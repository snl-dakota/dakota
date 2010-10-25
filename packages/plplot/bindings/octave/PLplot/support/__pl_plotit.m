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

function __pl_plotit

  ## __pl.type is the type of plot (polar, logx, etc.),
  ## __pl.items the number of triples (x, y, fmt) in the plot argument
  ## __pl.[x|y|fmt]{i,j} stores the triples where i is the item and j the figure

  global __pl

  strm = __pl_init;
  if (__pl.stopdraw(strm))
    return
  endif

  old_empty_list_elements_ok = warn_empty_list_elements;

  unwind_protect
    
  warn_empty_list_elements = 0;

  if (__pl.type(strm) >= 100 && __pl.type(strm) < 200)
    __pl_meshplotit;
    return;
  endif

  ## for log plots one has to calculate the log of the data,
  ## but first remove the negative data.
  if (__pl.type(strm) == 10 || __pl.type(strm) == 20 || __pl.type(strm) == 30)
    __pl_logplotit;
  endif
  
  ## It seems that a viewport of (x, x, y, y) doesn't work
  __pl_lxm = __pl.lxm(strm); __pl_lxM = __pl.lxM(strm);
  __pl_lym = __pl.lym(strm); __pl_lyM = __pl.lyM(strm);

  if (__pl_lxm == __pl_lxM)		
    __pl_lxm = __pl_lxm - 10*eps; __pl_lxM = __pl_lxM + 10*eps;
  endif
  if (__pl_lym == __pl_lyM)
    __pl_lym = __pl_lym - 10*eps; __pl_lyM = __pl_lyM + 10*eps;
  endif

  ## now check for user supplied axis ranges
  ## FIXME: possible polar plot inconsistency. User must specify axis in (x,y)
  ## coordinates, not (r,t). Advise user to use zoom() or ginput() to get (x,y)
    
  if (__pl.axis_st(strm))
    __pl_lxm = __pl.axis(strm,1); __pl_lxM = __pl.axis(strm,2);
    if (length(__pl.axis(strm,:)) >= 4)
      __pl_lym = __pl.axis(strm,3); __pl_lyM = __pl.axis(strm,4);
    else
      __pl.axis(strm,3) = __pl_lym; __pl.axis(strm,4) = __pl_lyM;
    endif
  else	## make axis() return current axis
    __pl.axis(strm,1) = __pl_lxm; __pl.axis(strm,2) = __pl_lxM;
    __pl.axis(strm,3) = __pl_lym; __pl.axis(strm,4) = __pl_lyM;
  endif

  ## for log plots,

  if (__pl.type(strm) == 10 || __pl.type(strm) == 30)
    __pl_lxm = log10(__pl_lxm); __pl_lxM = log10(__pl_lxM);
  endif
  if (__pl.type(strm) == 20 || __pl.type(strm) == 30)		
    __pl_lym = log10(__pl_lym); __pl_lyM = log10(__pl_lyM);
  endif
    
  ## start from the beginning
    
  if (__pl.items(strm) != 1)
    __pl.plcol(strm) = 1;
    __pl.pllsty(strm) = 1;
    __pl.lab_pos(strm) = 1;
    __pl.lab_sym(strm,:) = 0;
    __pl.line_count(strm) = 1;
    __pl.lab_str = "";
  endif
  
  ## set plot limits, plot box, grid and axis type  
  plcol(15); pllsty(1);
  if (__pl.type(strm) == -1)  # polar plot is special
    __pl_plenv(__pl_lxm, __pl_lxM, __pl_lym, __pl_lyM, 1, __pl.type(strm));
    if (__pl.grid(strm) == 1)
      __pl_polargrid(__pl_lxM, __pl_lyM, __pl_lxm, __pl_lym)
    endif
  else
    t = 0;
    if (__pl.axisxy(strm))
      t = __pl.grid(strm) + __pl.axisxy(strm);
    elseif (__pl.grid(strm))
      t = 1 + __pl.grid(strm);
    endif
    __pl_plenv(__pl_lxm, __pl_lxM, __pl_lym, __pl_lyM, 0, __pl.type(strm) + t);
  endif			    

  ## get the plots to be done, previously stored in the __pl_struct structure
  for items=1:__pl.items(strm)-1

    x = __pl.x{items, strm};
    y = __pl.y{items, strm};
    fmt = __pl.fmt{items, strm};

    if (__pl.type(strm) == 10 || __pl.type(strm) == 30)
      x = log10(x); 
    endif
    if (__pl.type(strm) == 20 || __pl.type(strm) == 30)
      y = log10(y);
    endif
    
    ## this is here just because of style variable
    if (isempty(fmt))
      [style, color, symbol, key_title] = __pl_opt(fmt);
    else
      [style, color, symbol, key_title] = __pl_opt(fmt(1,:));
    endif

    if (color != 20)
      __pl.plcol(strm) = color;
    endif
    
    xc = columns(x); yc = columns(y);
    
    if (is_matrix(x) && !is_vector(x) && is_matrix(y) && !is_vector(y)) 
      if (style != 9 && (xc != yc || rows(x) != rows(y)))
	error ("__pl_plotit: matrix dimensions must match.");
      endif
      range = "i;";	
    elseif (is_vector(x) || is_vector(y))
      range ="1:yc;";
    endif
    
    for i=1:xc
      for j=eval(range)
	
	if (is_strmatrix(fmt) && !is_strvector(fmt))
	  [style, color, symbol, key_title] = __pl_opt(fmt(rem(j-1,rows(fmt))+1,:));
	  if (color != 20)
	    __pl.plcol(strm) = color;
	  endif
	endif
	
	if (isempty(key_title))
	  if (!is_vector(x) || !is_vector(y))
	    lab = sprintf("line %d.%d", __pl.line_count(strm), (i-1)*yc+j);
	    if (i==xc && j==max(eval(range)))
	      __pl.line_count(strm)=__pl.line_count(strm)+1;
	    endif	# nothing more to do, hein?
	  else
	    lab = sprintf("line %d", __pl.line_count(strm));
	    __pl.line_count(strm) = __pl.line_count(strm)+1;
	  endif
	elseif (length(key_title) != 1 || key_title != " ")
	  if ((!is_vector(x) || !is_vector(y)) && is_strvector(fmt))
	    lab = sprintf("%s %d", key_title, (i-1)*yc+j);
	  else
	    lab = sprintf("%s", key_title);
	  endif
	else
	  lab = " ";
	endif

	if (__pl.legend(strm))
	  if (lab == "") ## log plots may have a CTRL-A as the label plot! hugh!
	    __pl.lab_str = [__pl.lab_str; " "];
	  else
	    __pl.lab_str = [__pl.lab_str; lab];
	  endif
	  __pl.lab_col(strm, __pl.lab_pos(strm)) = __pl.plcol(strm);
	  __pl.lab_lsty(strm, __pl.lab_pos(strm)) = __pl.pllsty(strm);
	  __pl.lab_pos(strm) = __pl.lab_pos(strm)+1;

	  if (style == 1 || style == 5)
	    __pl.lab_sym(strm, __pl.lab_pos(strm)-1) = symbol;
	  else
	    __pl.lab_sym(strm, __pl.lab_pos(strm)-1) = 0;
	  endif
	endif
	
	if (lab != "") ## log plots may have a CTRL-A as the label plot
	  plcol(__pl.plcol(strm));
	  __pl.plcol(strm) = rem(__pl.plcol(strm), 15)+1;
	  pllsty(__pl.pllsty(strm));
	  if  (__pl.line_style(strm))
	    __pl.pllsty(strm) = rem(__pl.pllsty(strm), 8)+1;
	  endif
	endif

	switch (style)
	  case (1)	## linespoints
	    plline(x(:,i),y(:,j));
	    plpoin(x(:,i),y(:,j), symbol);

	  case (2)	## lines
	    plline(x(:,i),y(:,j));

	    ## case (3) ## boxerrorbars
	    ## case (4) ## boxes

	  case (5) ## points
	    plpoin(x(:,i),y(:,j), symbol);

	  case (6) ## dots
	    plpoin(x(:,i),y(:,j), -1);								

	  case (7) ## impulses
	    for j=1:rows(x)
	      xs = [x(j,i); x(j,i)];
	      ys = [0; y(j,i)];
	      plline(xs,ys);
	    endfor	

	  case (8) ## steps
            xs = ys = [];
	    for j=1:rows(x)-1
	      xs = [xs; x(j,i); x(j+1,i); x(j+1,i)];
	      ys = [ys; y(j,i); y(j,i); y(j+1,i)];			
	    endfor	
	    plline(xs,ys);	

	  case (9) ## errorbars	    
	    if ( (xc == 1 && yc == 1) || xc > 3 || yc > 3)
	      error("plot with errorbars: either x or y or both must be 2 or 3 columns.\n\
		  If x (or y) has two columns, then it is interpreted as (x, dx),\n\
		  and a bar is plotted from x-dx to x+dx;\n\
		  if it has three columns, it is interpreted as (x, xmin, xmax)");			
	    endif

	    pllsty(1); # solid bars
	    if (xc == 2)
	      xm = x(:,1) .- x(:,2); xM = x(:,1) .+ x(:,2);
	    elseif (xc == 3)
	      xm = x(:,2); xM = x(:,3);
	    endif
	    
	    if (yc == 2)
	      ym = y(:,1) .- y(:,2); yM = y(:,1) .+ y(:,2);
	    elseif (yc == 3)
	      ym = y(:,2); yM = y(:,3);
	    endif
	    
	    if (xc != 1)
	      plerrx( xm, xM, y(:,1));
	    endif
	    if (yc != 1)
	      plerry( x(:,1), ym, yM);
	    endif
	    pllsty(__pl.pllsty(strm)); # recover linestyle
	    
	  otherwise
	    plline(x(:,i),y(:,j));
	    warning("__pl_plotit: FIXME: format '%s' not implemented",fmt);
	  endswitch

	  if (style == 9) 	## errorbars
	    break;
	  endif
	endfor ## j=eval(range)
    endfor ## i=1:xc
  endfor ## i=1:__pl.items

  if (__pl.legend(strm))
    __pl_draw_legend
  endif

  plcol(15);
  pllab(tdeblank(__pl.xlabel(strm,:)), tdeblank(__pl.ylabel(strm,:)), tdeblank(__pl.tlabel(strm,:)));
  plflush;

  unwind_protect_cleanup  
  
  warn_empty_list_elements = old_empty_list_elements_ok;

  end_unwind_protect  

endfunction
