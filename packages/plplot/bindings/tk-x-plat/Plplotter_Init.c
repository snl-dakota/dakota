/* $Id: Plplotter_Init.c 3186 2006-02-15 18:17:33Z slbrow $
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

#ifdef BUILD_Plplot
#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT
#endif /* BUILD_Vfs */

/*----------------------------------------------------------------------*\
 * Plplotter_Init
 *
 * Initialization routine for extended wish'es.
 * Creates the plframe, wait_until, and host_id (w/Tcl-DP only)
 * commands.  The more basic Plplot-Tcl initialization is handled by
 * the Plbasicinit function called from here.
\*----------------------------------------------------------------------*/

EXTERN int
Plplotter_Init( Tcl_Interp *interp )
{
#ifdef USE_TCL_STUBS
    /*
     * We hard-wire 8.1 here, rather than TCL_VERSION, TK_VERSION because
     * we really don't mind which version of Tcl, Tk we use as long as it
     * is 8.1 or newer.  Otherwise if we compiled against 8.2, we couldn't
     * be loaded into 8.1
     */
    Tcl_InitStubs(interp,"8.1",0);
#endif
#ifdef USE_TK_STUBS
    Tk_InitStubs(interp,"8.1",0);
#endif
    /* This must be before any other Tcl related calls */
    if (PlbasicInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }

    /*
     * Note, the old technique of:
     *    main = Tk_MainWindow(interp);
     * and then passing main to 'plframe' as the clientdata can
     * cause unusual problems, probably due to bugs in Tk on
     * some platforms, when the 'main window' doesn't yet exist
     * properly by the time we are called.  Since plframe only
     * uses the value in one place (each time a new frame is
     * created), we simply use 'Tk_MainWindow' in plframe, and
     * avoid the startup problems.
     */

/* plframe -- PLplot graphing widget */

    Tcl_CreateCommand( interp, "plframe", (Tcl_CmdProc*) plPlotterCmd,
		       (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);

    Tcl_PkgProvide(interp,"Plplotter",VERSION);
    return TCL_OK;
}

