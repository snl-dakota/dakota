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

## function label_plot( data [, label [, class [, names]]])
##
## plot data with point style, marking each point with a number
## indicating the point index in the array data
##
## if label exist, plot each class with a diferent color and symbol
##
## if class exist and is scalar, label only that class
## if class is vector label data(class,:) as points
## if class is matrix label only points such that data == class
##
## if names exists, they will be used at the plot legend
##
## too old, needs to be updated. 

function label_plot( data, label, class, names)

  global __pl
  strm = __pl_init;
  static j;

  set_axis = 0;	# signal that I set the axis
  if (!__pl.axis_st(strm))
    axis([min(data(:,1)), max(data(:,1)), min(data(:,2)), max(data(:,2))]);
    set_axis = 1;
  endif

  margin_st =  plot_margin(1);
  was_hold = 0;
  if (ishold)
    was_hold = 1;
  else
    j=1;
  endif

  [nr, nc] = size (data);

  if (nargin < 2)
    plot(data(:,1), data(:,2),'o');
  else
    if (columns(label) != 1)
      label = label';
      if (columns(label) != 1)
	error("label_plot: `label' must be a vector.\n");
      endif
    endif

    cla = create_set(label);

    if (nargin == 3 && isstr(class))
      names = setstr(ones(max(cla),columns(class))*32);
      t=1;for i=cla; names(i,:) = class(t++,:); endfor
    elseif (nargin <= 3)
      names = setstr(ones(max(cla),4)*32);
      for i=cla; names(i,:) = sprintf(" %2d ", i);end
    endif
    
    for i=cla
      j = rem(j,9)+1; fmt = sprintf("@%d%d;%s;", j-1, j, names(i,:));
      plot(data(find(label==i),1), data(find(label==i),2), fmt);hold on
    endfor
    hold off
  endif

  if (nargin == 3 && is_scalar(class))
    item = find(label == class)';
  elseif (nargin == 3 && is_vector(class) && columns(class) == 1)
    ## if (columns(class) != 1)
    ##	item = class;
    ## else
    item = class';
    ## endif
  elseif (nargin == 3 && is_matrix(class) && !isstr(class))
    item = []; ct = rows(class);
    if (nc != columns(class))
      error("label_plot: `data' and `class' must have the same number of columns\n");
    endif
    
    for i=1:ct
      [ix, iy] = find(data == ones(nr,1)*class(i,:));
      item = [item, ix(1)]; 
    endfor
  else
    item = 0:-1;
  endif

  hold on
  plot(data(item,1), data(item,2), '95;Erros;'); 
  hold off

  for i = item
    plptex(data(i,1), data(i,2), 0, 0, 0, num2str(i));
  endfor

  if (set_axis) # if I set the axis, I can unset them.
    axis;
  endif

  if (was_hold)
    hold on
  endif

  plflush;pleop;
  plot_margin (margin_st);

endfunction
