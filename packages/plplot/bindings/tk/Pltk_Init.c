/* $Id: Pltk_Init.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 * Copyright (C) 2004  Joao Cardoso
 *
 * This file is part of PLplot.
 *
 * PLplot is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Library Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * PLplot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with PLplot; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 * This file is derived from the old tkshell.c, and implements the Pltk init
 * function.  It can be loaded into any Tcl7.5 interpreter, and requires some
 * .tcl library files to be in known places.  Set your environment variables
 * to make sure of this.
 *
 * Old changes see 'tkshell.c'.  This file should be used _instead_ of
 * tkshell.c, if building the 'libPltk.so' shared library for dynamic loading
 * into Tcl.
 */

/*
 * tkshell.c
 * Maurice LeBrun
 * 6-May-93
 *
 * A miscellaneous assortment of Tcl support functions.
 */

#include "plserver.h"

extern int Matrix_Init(Tcl_Interp* interp);

/*----------------------------------------------------------------------*\
 * Pltk_Init
 *
 * Initialization routine for extended wish'es.
 * Creates the plframe, matrix, and host_id (w/Tcl-DP only)
 * commands.  Also sets the auto_path variable.
\*----------------------------------------------------------------------*/

int
Pltk_Init( Tcl_Interp *interp )
{
    /* This must be before any other Tcl related calls */
    if (PlbasicInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }

#ifdef USE_TK_STUBS
    /*
     * We hard-wire 8.1 here, rather than TCL_VERSION, TK_VERSION because
     * we really don't mind which version of Tcl, Tk we use as long as it
     * is 8.1 or newer.  Otherwise if we compiled against 8.2, we couldn't
     * be loaded into 8.1
     */
    Tk_InitStubs(interp,"8.1",0);
#endif

/* plframe -- PLplot graphing widget */

    Tcl_CreateCommand( interp, "plframe", (Tcl_CmdProc*) plFrameCmd,
		       (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);

/* host_id -- returns host IP number.  Only for use with Tcl-DP */

#ifdef PLD_dp
    Tcl_CreateCommand(interp, "host_id", (Tcl_CmdProc*) plHost_ID,
	      (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
#endif

/* Set up auto_path */

    if (pls_auto_path(interp) == TCL_ERROR)
	return TCL_ERROR;

/* Save initial RGB colormap components */
/* Disabled for now */

#if 0
    {
    Display *display;
    Colormap map;

    display = Tk_Display(mainWindow);
    map = DefaultColormap(display, DefaultScreen(display));

/* Convert this to use esc function if it's going to be used */
/* SaveColormap(display, map); */
    }
#endif

    Tcl_PkgProvide(interp,"Pltk",VERSION);

    return TCL_OK;
}
