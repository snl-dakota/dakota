## Copyright (C) 1996, 1997 John W. Eaton
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

## usage: fmt = __pl_opt__ (caller, opt)
##
## Decode plot option strings.
##
## OPT can currently be some combination of the following:
##
##   "-"   for lines plot style (default).
##   "."   for dots plot style.
##   "@"   for points plot style.
##   "-@"  for linespoints plot style.
##   "^"   for impulses plot style.
##   "L"   for steps plot style.
##   "#"   for boxes plot style.
##   "~"   for errorbars plot style.
##   "#~"  for boxerrorbars plot style.
##   "n"   with n in 1-6 (wraps at 8), plot color
##   "nm"  with m in 1-6 (wraps at 6), point style (only valid for "@" or "-@")
##   "c"   where c is one of ["r", "g", "b", "m", "c", "w"] colors.
##   ";title;" where "title" is the label for the key.
##		if title is blank, no label will be drawn
##
##   Special points formats:
##
##     ".", "+", "*", "o", "x" will display points in that style.
##
##   The legend label text can be specified in between `;' characters, eg: ';x^2;'
##	 The special character `#' followed by another special character may be
##   used for special text manipulation, eg ';x#u2#d;' print in `x^2' math,
##	 and `title("#gS#u5#d#dn=1#u") prints \sum_{n=1}^{5}.
##
##		#u superscript, end with #d
##		#d subscript, end with #u
##		#b backspace
##		## # itself
##		#+ toogle overline mode
##		#- toogle underline mode
##		#gx greek letter x
##		#fn switch to normal font
##		#fr switch to roman font
##		#fi switch to italic font
##		#fs switch to script font
##		#(nnn) hershey character nnn
##
##		For special fonts, the extended charset must be loaded first,
##	 	use `plfontld(1)' to set, and `plfontld(0)' to return to normal.
##
##   The colors, line styles, and point styles have the following
##   meanings:
#
#          y     yellow        .     point
#          m     magenta       o     circle
#          c     cyan          x     x-mark
#          r     red           +     plus
#          g     green         -     solid
#          b     blue          *     star
#          w     white         :     dotted
#          k     black         -.    dashdot
#          v     violet        --    dashed
#	       s     salmon
#
# Symbols, from 0-9, are:
#
#		0  'square'
#		1  '.'
#		2  '+'
#		3  '*'
#		4  'o'
#		5  'x'
#		6  'square'
#		7  'triangle'
#		8  'cross-circle'
#		9  'dot-circle'.
#
# The PLplot colors are:
#
#   0. black (default background)
#   1. red
#   2. yellow
#   3. green
#   4. aquamarine
#   5. pink
#   6. wheat
#   7. grey
#   8. brown
#   9. blue
#  10. BlueViolet
#  11. cyan
#  12. turquoise
#  13. magenta
#  14. salmon
#  15. white (default foreground)
                                         
## Author: Rick Niles <niles@axp745.gsfc.nasa.gov>
## Adapted-By: jwe
## Modified: jc

function [style, color, symbol, key_title] = __pl_opt (opt)

  style = 2;
  color = 20;
  symbol = 21;
  key_title = "";
  
  set_color = 0;
  set_symbol = 0;
  set_lines = 0;
  set_dots = 0;
  set_points = 0;
  set_impulses = 0;
  set_steps = 0;
  set_boxes = 0;
  set_errbars = 0;
  set_key = 0;
  more_opts = 1;

  if (nargin != 1)
    usage ("__pl_opt (opt)");
  endif

  if (! isstr (opt))
    error ("__pl_opt: argument must be a string");
  endif

  if isempty(opt)
    return;
  endif
  
  while (more_opts)

    ## first get next char.

    if (max (size (opt)) > 1)
      ## [char, opt] = sscanf (opt, "%c %s", "c");
      char = opt(1);
      opt = opt(2:length(opt));
    else
      char = opt;
      more_opts = 0;
    endif

    ## now set flags based on char.

    if (char == "-")
      set_lines = 1;
    elseif (char == ".")
      set_dots	= 1;
      set_symbol = 1;
      symbol = 1;
    elseif (char == "@")
      set_points = 1;
    elseif (char == "^")
      set_impulses = 1;
    elseif (char == "L")
      set_steps = 1;
    elseif (char == "~")
      set_errbars = 1;
    elseif (char == "#")
      set_boxes = 1;
    elseif (isdigit(char))
      if (set_color)
	set_points = 1;
	symbol = str2num(char);
	set_symbol = 1;
      else
	color = str2num(char);
	set_color = 1;
      endif
    elseif (char == "r")
      set_color = 1;
      color = 1;
    elseif (char == "g")
      set_color = 1;
      color = 3;
    elseif (char == "b")
      set_color = 1;
      color = 9;
    elseif (char == "m")
      set_color = 1;
      color = 13;
    elseif (char == "c")
      set_color = 1;
      color = 11;
    elseif (char == "w")
      set_color = 1;
      color = 15;
    elseif (char == "k")
      set_color = 1;
      color = 0;
    elseif (char == "s")
      set_color = 1;
      color = 14;
    elseif (char == "v")
      set_color = 1;
      color = 10;
    elseif (char == "y")
      set_color = 1;
      color = 2;
    elseif (char == "*")
      set_points = 1;
      set_symbol = 1;
      symbol = 3;
    elseif (char == "+")
      set_points = 1;
      set_symbol = 1;
      symbol = 2;
    elseif (char == "o")
      set_points = 1;
      set_symbol = 1;
      symbol = 4;
    elseif (char == "x")
      set_points = 1;
      set_symbol = 1;
      symbol = 5;
    elseif (char == ";")  # title mode
      set_key = 1;
      working = 1;
      key_title = ""; 
      while (working)
        if (max (size (opt)) > 1)
	  char = opt(1);
	  opt = opt(2:length(opt));
        else
	  char = opt;
	  if (char != ";")
            error ("unfinished key label");
          endif
          more_opts = 0;
          working = 0;
        endif
        if (char == ";")
          working = 0;
        else
	  if (isempty (key_title))  # needs this to avoid empty matrix warning.
            key_title = char;
	  else
            key_title = strcat (key_title, char);
	  endif
	endif
      endwhile
    elseif (char == " ")
      ## whitespace -- do nothing.
    else
      error ("unrecognized format character: '%s'", char);
    endif
  endwhile

  ## now create format string.

  if (set_lines)
    if (set_points)
      style = 1;	# linespoints
    else
      style = 2;	# lines
    endif
  elseif (set_boxes)
    if (set_errbars)
      style = 3;	# boxerrorbars
    else
      style = 4;	# boxes
    endif
  elseif (set_points)
    style = 5;	# points
  elseif (set_dots)
    style = 6;	# dots
  elseif (set_impulses)
    style = 7;	# impulses
  elseif (set_steps)
    style = 8;	# steps
  elseif (set_errbars)
    style = 9; 	# errorbars
  endif

endfunction
