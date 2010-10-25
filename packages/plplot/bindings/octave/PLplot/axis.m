## Copyright (C) 1996 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.

## usage: ax = axis ()
##        axis (ax | opt)
##        axis (ax, opt)
##
## Sets the axis limits.
##
## With no arguments, turns "autoscaling" on, and returns the current axis
##
## "ax" can be a 2, 4 or 6 element row vector that sets the minimum and
##   maximum lower and upper limits of the data that will be plotted: 
##     [xmin, xmax], [xmin, xmax, ymin, ymax], [xmin, xmax, ymin, ymax, zmin, zmax]
##
## "opt" is an option string, that:
##
##   control the aspect ratio of the axes:
##     "square"   Force a square aspect ratio. 
##     "equal"    Force x distance to equal y-distance. 
##     "normal"   Restore the balance. 
##     "auto"     Set the specified axes to have nice limits around the data.
##     "manual"   Fix the current axes limits. 
##     "tight"    Fix axes to the limits of the data. 
##     "image"    Equivalent to "tight" and "equal". 
## 
##   affect the appearance of tic marks (note, if there are no tic marks
##   for an axis, there can be no labels):
##     "on"       Turn tic marks and labels on for all axes. 
##     "off"      Turn tic marks off for all axes. 
##     "tic[xy]"  Turn tic marks on for all axes, or turn them on for
##                  the specified axes and off for the remainder. 
##     "label[xy] Turn tic labels on for all axes, or turn them on
##                  for the specified axes and off for the remainder. 
##     "nolabel"  Turn tic labels off for all axes. 
## 
##   affect the appearance of the x and y coordinate axis
##      "axison"  Turn on the x and y coordinate axis
##      "axisoff" Turn off the x and y coordinate axis
##
##   returns current axis state:
##     [s1,s2,s3] = axis('state') returns current settings

## Author: jwe
## Modified: jc

function curr_axis = axis (ax, opt)

  ## the idea is that _pl.axis always has the current max/min axis, either
  ## as setup by the user, or automaticaly. It interacts with hold mode.

  global __pl
  strm = __pl_init;

  if (nargin == 0)
    __pl.axis_st(strm) = 0;
    __pl.aspect(strm) = 0;
    curr_axis = __pl.axis(strm,:);
  elseif (nargin == 1 && isstr (ax))
    opt = ax; ax = [];
  elseif (nargin == 1 && is_vector (ax))
    opt = "";
  else
    usage "axis"
  endif

  if (nargin != 0 && !isempty (ax))
    len = length (ax);

    if (len != 2 && len != 4 && len != 6)
      error ("axis: expecting vector with 2, 4, or 6 elements.\n");
    endif
    
    __pl.axis_st(strm) = 1;

    if (len > 1)
      __pl.axis(strm, [1, 2]) = [ax(1), ax(2)];
    endif

    if (len > 3)
      __pl.axis(strm, [3, 4]) = [ax(3), ax(4)];
    endif

    if (len > 5)
      __pl.axis(strm, [5, 6]) = [ax(5), ax(6)];
    endif

  elseif (nargin != 0 && !isempty(opt))

    switch(opt)
      case "state" ## [s1, s2, s3] = axis('state') returns current settings
  	## S1 = 'auto' or 'manual'.
	if (__pl.axis_st(strm) == 0 && __pl.margin(strm) == 1)
	  S1 = "auto";
	elseif (__pl.axis_st(strm) == 1)
	  S1 = "manual";
	elseif (__pl.axis_st(strm) == 0 && __pl.margin(strm) == 0)
	  S1 = "tight";
	elseif (__pl.aspect(strm) == 1 &&__pl.axis_st(strm) == 0)
	  S1 = "image";
	endif

  	## S2 = 'on' or 'off'.
	S2 = "off";
	if (__pl.xticks(strm,3) == 1 && __pl.yticks(strm,3) == 1)
	  S2 = "on";
	endif

  	## S3 = 'xy' or 'ij'.
	S3 = "xy";
	curr_axis = [S1; S2; S3];

	## aspect ratio
      case "square"
	__pl.aspect(strm) = 2;
      case "equal"
	__pl.aspect(strm) = 1;
      case "normal"
	__pl.axis_st(strm) = 0;
	__pl.aspect(strm) = 0;

	## the way axis limits are interpreted.
      case "auto" 
	__pl.axis_st(strm) = 0;
	__pl.margin(strm) = 1;
      case "manual"  ## fix the current axis
	__pl.axis_st(strm) = 1;
      case "tight" 
	__pl.axis_st(strm) = 0;
	__pl.margin(strm) = 0;
      case "image"
	__pl.aspect(strm) = 1;
	__pl.margin(strm) = 0;

	## the appearance of tic marks
      case "on"  ## tic marks and tick labels on for all axes 
	__pl.xticks(strm,3) = 1;
	__pl.xticks(strm,4) = 1;
	__pl.yticks(strm,3) = 1;
	__pl.yticks(strm,4) = 1;
      case "off"  ## tic marks off for all axes 
	__pl.xticks(strm,3) = 0;
	__pl.yticks(strm,3) = 0;
      case "ticxy"  ## on for the specified axes and off for the remainder.
	__pl.xticks(strm,3) = 1;
	__pl.yticks(strm,3) = 1;
      case "ticx" 
	__pl.xticks(strm,3) = 1;
	__pl.yticks(strm,3) = 0;
      case "ticy" 
	__pl.xticks(strm,3) = 0;
	__pl.yticks(strm,3) = 1;

	## the appearance of tic labels
      case "labelxy"  ## on for the specified axes and off for the remainder. 
	__pl.xticks(strm,4) = 1;
	__pl.yticks(strm,4) = 1;
      case "labelx"
	__pl.xticks(strm,4) = 1;
	__pl.yticks(strm,4) = 0;
      case "labely"
	__pl.xticks(strm,4) = 0;
	__pl.yticks(strm,4) = 1;
      case "nolabel"  ## Turn tic labels off for all axes.
	__pl.xticks(strm,4) = 0;
	__pl.yticks(strm,4) = 0;

	##  the direction of increasing values on the axes
      case "ij" 
	warning("axis: option 'ij' not supported.");
      case "xy" 
	1;  ## default

	## the appearance of the xy axis
      case "axison"
	  __pl.axisxy(strm) = 1;
	case "axisoff"
	  __pl.axisxy(strm) = 0;

      otherwise
	error("axis: option '%s' not recognized.\n", tdeblank(opt));
    endswitch
  endif

  if (automatic_replot)
    __pl_plotit;
  endif

endfunction
