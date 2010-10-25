/* $Id: tkshell.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 * Maurice LeBrun
 * 6-May-93
 *
 * A miscellaneous assortment of Tcl support functions.
 *
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
 */

#include "plserver.h"

/*----------------------------------------------------------------------*\
 * Pltk_Init
 *
 * Initialization routine for extended wish'es.
 * Creates the plframe, matrix, wait_until, and host_id (w/Tcl-DP only)
 * commands.  Also sets the auto_path variable.
\*----------------------------------------------------------------------*/

int
Pltk_Init( Tcl_Interp *interp )
{
    Tk_Window main;

    main = Tk_MainWindow(interp);

/* plframe -- PLplot graphing widget */

    Tcl_CreateCommand(interp, "plframe", (Tcl_CmdProc*) plFrameCmd,
                      (ClientData) main, (Tcl_CmdDeleteProc*) NULL);

/* matrix -- matrix support command */

    Tcl_CreateCommand(interp, "matrix", (Tcl_CmdProc*) Tcl_MatrixCmd,
                      (ClientData) main, (Tcl_CmdDeleteProc*) NULL);

/* wait_until -- waits for a specific condition to arise */
/* Can be used with either Tcl-DP or TK */

    Tcl_CreateCommand(interp, "wait_until", (Tcl_CmdProc*) plWait_Until,
		      (ClientData) NULL,  (Tcl_CmdDeleteProc*) NULL);

/* host_id -- returns host IP number.  Only for use with Tcl-DP */

#ifdef PLD_dp
    Tcl_CreateCommand(interp, "host_id", (Tcl_CmdProc*) plHost_ID,
		      (ClientData) NULL,  (Tcl_CmdDeleteProc*) NULL);
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

    display = Tk_Display(main);
    map = DefaultColormap(display, DefaultScreen(display));

/* Convert this to use esc function if it's going to be used */
/* SaveColormap(display, map); */
    }
#endif
    return TCL_OK;
}

/*----------------------------------------------------------------------*\
 * plWait_Until
 *
 * Tcl command -- wait until the specified condition is satisfied.
 * Processes all events while waiting.
 *
 * This command is more capable than tkwait, and has the added benefit
 * of working with Tcl-DP as well.  Example usage:
 *
 *	wait_until {[info exists foobar]}
 *
 * Note the [info ...] command must be protected by braces so that it
 * isn't actually evaluated until passed into this routine.
\*----------------------------------------------------------------------*/

int
plWait_Until(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{
    int result = 0;

    dbug_enter("plWait_Until");

    for (;;) {
	if (Tcl_ExprBoolean(interp, argv[1], &result)) {
	    fprintf(stderr, "wait_until command \"%s\" failed:\n\t %s\n",
		    argv[1], interp->result);
	    break;
	}
	if (result)
	    break;

	Tk_DoOneEvent(0);
    }
    return TCL_OK;
}
