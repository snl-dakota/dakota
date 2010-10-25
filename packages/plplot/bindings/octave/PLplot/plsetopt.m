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

## function plsetopt(option, value)
##
## set "option" to "value". Must be called before openning a plot window.
## No check in "value" is done, and some options might be senseless,
## depending on the driver.
##
## If "reset" is specified as an option, all stored options are cleared.
## If "show" is specified as an option, all stored options are printed.
## If "help" is specified as an option, all options and help are printed.
## If "apply" is specified as an option, all set options are applied.
## If "get" is specified as an option, the current option value is returned

function ret = plsetopt(option, value)

  options = ["reset"; "Reset options";
	     "show"; "Show set options";
	     "help"; "Show all possible options";
	     "apply"; "Apply the set options";
	     "get"; "Get current set value";
	     "display"; "X server to contact";
	     "server_name"; "Main window name of PLplot server (tk driver)";
	     "server_host";"Host to run PLplot server on (dp driver)";
	     "server_port"; "Port to talk to PLplot server on (dp driver)";
	     "user"; "User name on remote node (dp driver)";
	     "plserver"; "Invoked name of PLplot server (tk or dp driver)";
	     "plwindow"; "Name of PLplot container window (tk or dp driver)";
	     "tcl_cmd"; "TCL command string run at startup (note: disabled)";
	     "auto_path"; "Additional directory(s) to autoload (tk or dp driver)";
	     "tk_file"; "file for plserver (tk or dp driver)";
	     "width"; "Sets pen width (1 <= width <= 10)"; 
	     "geometry"; "Window size, in pixels (e.g. -geometry 400x300)";
	     "bufmax";"bytes sent before flushing output";
	     "db";"Double buffer X window output";
	     "np"; "No pause between pages";
	     "debug"; "Print debugging info (implies -verbose)";
	     "verbose"; "Be more verbose than usual";
	     "nopixmap";"Don't use pixmaps in X-based drivers";
	     "bg"; "Background color (0=black, FFFFFF=white)";
	     "v"; "Print out the PLplot library version number";
	     "hack"; "Enable driver-specific hack(s)";
	     "dev"; "Output device name";
	     "o"; "Output filename";
	     "px"; "Plots per page in x";
	     "py"; "Plots per page in y";
	     "wplt"; "Relative coordinates [0-1] of window into plot";
	     "mar"; "Margin space in relative coordinates (0 to 0.5, def 0)";
	     "a"; "Page aspect ratio (def: same as output device)";
	     "jx"; "Page justification in x (-0.5 to 0.5, def 0)";
	     "jy"; "Page justification in y (-0.5 to 0.5, def 0)";
	     "ori"; "Plot orientation (0,2=landscape, 1,3=portrait)";
	     "freeaspect"; "Allow aspect ratio to adjust to orientation swaps";
	     "portrait"; "Sets portrait mode (both orientation and aspect ratio)";
	     "width";  "Sets pen width (1 <= width <= 10)";
	     "ncol0"; "Number of colors to allocate in cmap 0 (upper bound)";
	     "ncol1"; "Number of colors to allocate in cmap 1 (upper bound)";
	     "fam"; "Create a family of output files";
	     "fsiz"; "Output family file size (e.g. -fsiz 0.5G, def MB)";
	     "fbeg"; "First family member number on output";
	     "finc";  "Increment between family members";
	     "fflen"; "Family member number minimum field width";
	     "dpi"; "Resolution, in dots per inch (e.g. -dpi 360x360)";
	     "drvopt"; "Driver dependent option";
	     "compression"; "Sets compression level in supporting devices"];
  
  global __pl set_options = "";

  ret = "";

  if (nargin == 0 || (nargin >= 1 && !isstr(option)) || (nargin == 2 && !isstr(value)))
    help "plsetopt"
    return
  endif

  if (!exist("__pl") || !struct_contains (__pl,"inited"))
    ## __pl_init; this doesn't make sense,
    ## as some options must be set before plinit()
    plplot_stub;
  endif

  nr = rows(options);
  if (option(1) == "-")
    option(1) = "";
  endif

  if (!exist("value") || isempty(value))
    value = "1";
  endif

  if (strcmp(option, "help"))
    for i=1:2:nr
      printf("%12s: %s\n", deblank(options(i,:)), options(i+1,:));
    endfor
    return
  endif

  if (strcmp(option, "reset"))
    set_options="";
    return
  endif

  if (strcmp(option, "show") || strcmp(option, "apply") || strcmp(option,"get"))
    for i=1:2:rows(set_options)
      sopt =  deblank(set_options(i,:)); opt =  deblank(options(i,:));
      if (!isempty(sopt))
	if (strcmp(option, "show"))
	  printf("%s: %s\n", opt, sopt);
	elseif (strcmp(option, "apply"))
	  plSetOpt(opt, sopt);
	  elseif (strcmp(opt, value))
	    ret = sopt;
	endif
      endif
    endfor
    return
  endif

  found = 0;
  for i=1:2:nr
    if (strcmp(deblank(option), deblank(options(i,:))));
      set_options = __pl_matstr(set_options, value, i);
      found = 1;
      break;
    endif
  endfor
 
  if (! found)
    error("Option doesn't exist\n")
  endif

  
endfunction
