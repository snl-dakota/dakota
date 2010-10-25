/* $Id: matrixInit.c 3186 2006-02-15 18:17:33Z slbrow $

    Tcl Matrix initializer.
    Vince Darley

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

#include "tclMatrix.h"

int Matrix_Init( Tcl_Interp *interp ) {

    /* matrix -- matrix	support	command	*/
    Tcl_CreateCommand(interp, "matrix",	(Tcl_CmdProc *) Tcl_MatrixCmd,
		      (ClientData) NULL, (Tcl_CmdDeleteProc *)	NULL);

    Tcl_PkgProvide(interp, "Matrix", "0.1");
    return TCL_OK;
}
