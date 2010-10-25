/* $Id: xtk04.c 3186 2006-02-15 18:17:33Z slbrow $

   Copyright (C) 2004  Joao Cardoso

   This file is part of PLplot.

   PLplot is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Library Public License as published
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   PLplot is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with PLplot; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "pltk.h"

#include <math.h>

static int
AppInit(Tcl_Interp *interp);

int mat_max( tclMatrix *pm, Tcl_Interp *interp,
	     int argc, char *argv[] )
{
    PLFLT max = pm->fdata[0];
    int i;
    for( i=1; i < pm->len; i++ )
	if (pm->fdata[i] > max)
	    max = pm->fdata[i];

    sprintf( interp->result, "%f", max );
    return TCL_OK;
}

int mat_min( tclMatrix *pm, Tcl_Interp *interp,
	     int argc, char *argv[] )
{
    PLFLT min = pm->fdata[0];
    int i;
    for( i=1; i < pm->len; i++ )
	if (pm->fdata[i] < min)
	    min = pm->fdata[i];

    sprintf( interp->result, "%f", min );
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * main --
 *
 * Just a stub routine to call pltkMain.  The latter is nice to have
 * when building extended wishes, since then you don't have to rely on
 * sucking the Tk main out of libtk (which doesn't work correctly on all
 * systems/compilers/linkers/etc).  Hopefully in the future Tk will
 * supply a sufficiently capable tkMain() type function that can be used
 * instead.
\*--------------------------------------------------------------------------*/

int
main(int argc, char **argv)
{
    exit(pltkMain(argc, argv, NULL, AppInit));
}

/*
 *--------------------------------------------------------------------------
 *
 * AppInit --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 * Taken from tkAppInit.c --
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *--------------------------------------------------------------------------
 */

int   get_dataCmd        (ClientData, Tcl_Interp *, int, char **);

static int
AppInit(Tcl_Interp *interp)
{
    Tk_Window mainWindow = Tk_MainWindow(interp);

/*
 * Call the init procedures for included packages.  Each call should
 * look like this:
 *
 * if (Mod_Init(interp) == TCL_ERROR) {
 *     return TCL_ERROR;
 * }
 *
 * where "Mod" is the name of the module.
 */

    if (Pltk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

/*
 * Call Tcl_CreateCommand for application-specific commands, if
 * they weren't already created by the init procedures called above.
 */

    Tcl_CreateCommand(interp, "get_data",(Tcl_CmdProc*) get_dataCmd,
                      (ClientData) mainWindow, (Tcl_CmdDeleteProc*) NULL);

    Tcl_MatrixInstallXtnsn( "max", mat_max );
    Tcl_MatrixInstallXtnsn( "min", mat_min );

    return TCL_OK;
}

int   get_dataCmd( ClientData cd, Tcl_Interp *interp, int argc, char **argv )
{
    tclMatrix *pm, *matPtr;
    int nx, ny, i, j;
    PLFLT pi = 3.1415927;
    int kx = 3, ky = 2;

    pm = Tcl_GetMatrixPtr( interp, argv[1] );
    matPtr = pm;

    if ( pm->dim != 2 ) {
	interp->result = "must use 2-d matrix.";
	return TCL_ERROR;
    }

    nx = pm->n[0], ny = pm->n[1];

    for( i=0; i < nx; i++ ) {
	for( j=0; j < ny; j++ ) {
	    pm->fdata[I2D(i,j)] = sin(pi*kx*i/64) * sin(pi*ky*j/64);
	}
    }

    return TCL_OK;
}
