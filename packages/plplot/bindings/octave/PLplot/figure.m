## Copyright (C) 1998-2003  Joao Cardoso
## Copyright (C) 2004  Alan W. Irwin
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

## usage: [id driver intp] = figure ()
##
## usage: [id driver intp] = figure (n)
##
## usage: [id driver intp] = figure (n, device)
##
## usage: [id driver intp] = figure (n, device, file)
##
## usage: [id driver intp] = figure (n, device, file , win_id)
##
## usage: [id driver intp] = figure (n, device, file , win_id, tk_file, plot_frame)
##
## Set the current plot window to plot window n or creates a new figure,
##    returning the window id and driver;
## If the driver is tk, also returns its interpreter name 
## If device is specified, it is opened. Default is 'xwin'.
## If file is specified, it is passed to PLplot for use. Can be a real file for
##    plot output (e.g. postscript), or a  Xserver DISPLAY name. 
## If win_id is specified, the plot will use the X window identified by win_id
##   if the driver is "xwin". Use program `xwininfo' to get its hexadecimal id
##   and octave `hex2dec' to get its decimal id.
##
## For use with an external .tcl script:
##
## If tk_file is specified, the tk driver executes this file. In this case,
##    where tk_file tipically draws a tk GUI, plot_frame should be the name of
##    an empty frame where the actual plot will take place.
##
## if file == "" or win_id == 0 no action is taken for this arguments.

function [n, driver, intp]= figure (n, device, file, win_id, tk_file, plot_frame)

  global __pl __tk_name

  if (!exist("__pl") || !struct_contains (__pl,"inited"))
    v = split(version ,'.');
    if (! (str2num(v(1,:)) >= 2 &&
           str2num(v(2,:)) >= 1 &&
           str2num(v(3,:)) >= 57))
        error("The PLplot-Octave scripts need an Octave version \n\
        greater then or equal to 2.1.57.\n");
    endif

    plplot_stub;

    ## closeallfig must be called once before atexit is called!
    closeallfig;
    atexit("closeallfig");

    __pl.inited = 1;

    if (automatic_replot == 0)
      warning("It is recommended that you set 'automatic_replot=1' \n\
	  in your ~/.octaverc file.");
      ##automatic_replot = 1;
    endif
  endif

  if (nargin == 0)
    if (plglevel >= 1)
      n = plgstrm;
      return;
    else
      n = 0;
    endif
  endif

  if (nargin != 0 || plglevel == 0)
    __pl.hold(plgstrm+1) = ishold; # save current hold state
    plsstrm(n);
    n = plgstrm;
    strm = n+1;

    if ( plglevel == 0)	# no device open yet

      if (nargin >= 4 && win_id != 0 ) # use the specified X window
	plsxwin(win_id);
      endif

      if (nargin >= 3 && !isempty(file)) # use the specified file
	plsfnam(file);
      endif

      if (nargin >= 2 )
	if (isstr(device))
	  plsdev(device);
	else
	  error("figure: `device' must be a string");
	endif
      else
	device = plsetopt ("get", "dev"); # possible user defaults
	if (isempty(device))
	  device="xwin";
	endif
	plsdev(device);
      endif

      __pl.type(strm) = 0;
	__pl.items(strm) = 1;
      __pl.legend_xpos(strm) = 1;	# legend x position
      __pl.legend_ypos(strm) = 1;	# legend y position
      __pl.legend(strm) = 2;	        # legend type
      __pl.lab_pos(strm) = 1;	        # label position
      __pl.lab_lsty(strm,1) = 0;	# label (line) style
      __pl.lab_col(strm,1) = 0;	# label color
      __pl.lab_sym(strm,:) = 0;	# label symbol
      __pl.multi_row(strm) = 1;	# multiplot state
      __pl.multi_col(strm) = 1;	# multiplot columns
      __pl.multi_cur(strm) = 1;	# current multiplot
      __pl.multi(strm) = 0;	# multiplot rows
      __pl.aspect(strm) = 0;    # plot aspect ratio (auto)
      __pl.axis_st(strm) = 0;	# axis state
      __pl.axis(strm,:) = zeros(1,6);	  # current axis
      ## xticks(1) xticks interval, xticks(2) number of sub-ticks
      ## xticks(3) ticks disabled, xticks(4) tick labels disabled
      __pl.xticks(strm,:) = [0, 0, 1, 1]; # xtick interval, number of minor xticks
      __pl.yticks(strm,:) = [0, 0, 1, 1];
      __pl.zticks(strm,:) = [0, 0, 1 ,1];

      ## x/y axis on/off
      __pl.axisxy(strm) = 0;

      ## min/max x/y values
      __pl.lxm(strm) = __pl.lym(strm) = __pl.lzm(strm) = realmax;
      __pl.lxM(strm) = __pl.lyM(strm) = __pl.lzm(strm) = -realmax;

      __pl.line_style(strm) = 0;  # don't increase line style after plot
      __pl.az(strm) = -60;	# azimuth
      __pl.alt(strm) = 30;	# altitude
      __pl.plcol(strm) = 1;	# current plot color
      __pl.pllsty(strm) = 1;	# current plot style
      __pl.line_count(strm) = 1;      # current label counter
      __pl.hold(strm) = 0; hold off;  # hold state
      __pl.open(strm) = 1;            # figure state
      __pl.margin(strm) = 1;	   # keep a small margin around box
      __pl.stopdraw(strm) = 0;	   # don't hold draw until drawnow().
      __pl.grid(strm) = 0;	   # grid state

      if (!struct_contains(__pl, "lab_str"))
	__pl.lab_str(strm) = "";	# label string
      endif

      if (struct_contains(__pl, "xlabel"))
	__pl.xlabel = __pl_matstr( __pl.xlabel, "X", strm); # x,y,z,title labels text
	__pl.ylabel = __pl_matstr( __pl.ylabel, "Y", strm);
	__pl.zlabel = __pl_matstr( __pl.zlabel, "Z", strm);
	__pl.tlabel = __pl_matstr( __pl.tlabel, "Title", strm);
      else
	__pl.xlabel(strm,:) = "X";
	__pl.ylabel(strm,:) = "Y";
	__pl.zlabel(strm,:) = "Z";
	__pl.tlabel(strm,:) = "Title";
      endif

      if (struct_contains(__pl, "shading"))
	__pl.shading = __pl_matstr(__pl.shading, "faceted", strm); # shading type
      else
	__pl.shading(strm,:) = "faceted";
      endif

      if (struct_contains(__pl, "intp"))
	__pl.intp = __pl_matstr(__pl.intp, " ", strm); # tk interpreter name		
      else
	__pl.intp(strm,:) = " ";
      endif

      ## the tk stuff
      if (strcmp("tk", sprintf("%s",plgdev')))
	if (! exist("tk_start") && nargin == 6)
	  error("Can't use this Tk feature of PLplot until tk_octave \
	      is installed!\n")
	elseif (exist("tk_start"))
	 if (!exist("__tk_name"))
	  tk_init;
	 endif

	  init_file = tmpnam();
	  fp = fopen (init_file,"w");

	  fprintf(fp, "set octave_interp {%s}\n", __tk_name);
	  fprintf(fp, "set octave_interp_pid %d\n", getpid);
	  fprintf(fp, "send -async $octave_interp to_octave intp=\"[tk appname]\"\n");

	  fprintf(fp, "proc to_octave {a args} {\n");
	  fprintf(fp, "global octave_interp octave_interp_pid;\n");
	  fprintf(fp, "send -async $octave_interp to_octave \"$a $args\";\n");
	  fprintf(fp, "#exec kill -16 $octave_interp_pid}\n");
	endif
	
	if (nargin == 6)
	  fprintf(fp, "source {%s}\n", tk_file);
	  fclose(fp);
	  plSetOpt ("plwindow", plot_frame);
	  plSetOpt ("tk_file", init_file);
	elseif (exist("__tk_name"))
	  fclose(fp);
	  plSetOpt ("tk_file", init_file);
	  plSetOpt("plwindow", sprintf(".figure_%d",n));
	else
  	  plSetOpt("plwindow", sprintf(".figure_%d",n));
	endif

	intp = sprintf("figure_%d",n);
	__pl.intp = __pl_matstr(__pl.intp, intp, strm);	# tk interpreter name
      else
	plSetOpt("plwindow", sprintf("Figure %d",n));
      endif

      plSetOpt("geometry", "400x400+750+0");
      plSetOpt("np", "");
      pldef	# user can override above defaults or add other options
      plsetopt("apply"); # override/add momentary options.

      ## init driver and make changes apply
      plinit;
      #next two commented. After last bop/eop CVS changes, first plot is empty.
      #pladv(0);
      #plflush;pleop;
      
      if ( exist("__tk_name") & (strcmp("tk", sprintf("%s",plgdev'))))
	eval(tk_receive(1));
	__pl.intp = __pl_matstr(__pl.intp, intp, strm);	# tk interpreter name					
	unlink(init_file);
      else
	intp = deblank(__pl.intp(strm,:));
      endif

    else
      if (__pl.hold(strm))
   	hold on;
      endif
      intp = deblank(__pl.intp(strm,:));
      ## warning("figure already opened");
    endif
  endif

  driver = sprintf("%s",plgdev');

  if (!exist("intp"))
    intp = deblank(__pl.intp(n+1,:));
  endif

endfunction
