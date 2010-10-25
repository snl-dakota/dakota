/* $Id: pltcl.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 *	Main program for Tcl-interface to PLplot.  Allows interpretive
 *	execution of plotting primitives without regard to output driver.
 *
 * Maurice LeBrun
 * IFS, University of Texas at Austin
 * 19-Jun-1994
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
 */

#include "plplotP.h"
#include "pltcl.h"
#ifdef HAVE_ITCL
# ifndef HAVE_ITCLDECLS_H
#  define RESOURCE_INCLUDED
# endif
# include <itcl.h>
#endif

static int
AppInit(Tcl_Interp *interp);

/*--------------------------------------------------------------------------*\
 * main --
 *
 * Just a stub routine to call pltclMain.  The latter is nice to have when
 * building extended tclsh's, since then you don't have to rely on sucking
 * the Tcl main out of libtcl (which doesn't work correctly on all
 * systems/compilers/linkers/etc).  Hopefully in the future Tcl will
 * supply a sufficiently capable tclMain() type function that can be used
 * instead.
\*--------------------------------------------------------------------------*/

int
main(int argc, char **argv)
{
    exit(pltclMain(argc, argv, NULL, AppInit));
}

/*--------------------------------------------------------------------------*\
 * plExitCmd
 *
 * PLplot/Tcl extension command -- handle exit.
 * The reason for overriding the normal exit command is so we can tell
 * the PLplot library to clean up.
\*--------------------------------------------------------------------------*/

static int
plExitCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{

/* Print error message if one given */

    if (interp->result != NULL && interp->result[0] != '\0')
	fprintf(stderr, "%s\n", interp->result);

    plspause(0);
    plend();

    Tcl_UnsetVar(interp, "tcl_prompt1", 0);
    Tcl_Eval(interp, "tclexit");

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * prPromptCmd
 *
 * PLplot/Tcl extension command -- print the prompt.
 * Allows much more flexible setting of the prompt.
\*--------------------------------------------------------------------------*/

static int
prPromptCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{
    PLStream *pls;
    char prompt[80];

    plgpls(&pls);

    if (pls->ipls == 0)
	sprintf(prompt, "pltext; puts -nonewline \"pltcl> \"; flush stdout");
    else
	sprintf(prompt, "pltext; puts -nonewline \"pltcl_%d> \"; flush stdout", pls->ipls);

    Tcl_VarEval(interp, prompt, 0);

    return TCL_OK;
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
 *--------------------------------------------------------------------------
 */

static int
AppInit(Tcl_Interp *interp)
{
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

    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
#ifdef HAVE_ITCL
    if (Itcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
#endif
    if (Pltcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

/* Application-specific startup.  That means: for use in pltcl ONLY. */

/* Rename "exit" to "tclexit", and insert custom exit handler */

    Tcl_VarEval(interp, "rename exit tclexit", (char *) NULL);

    Tcl_CreateCommand(interp, "exit", (Tcl_CmdProc *) plExitCmd,
                      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(interp, "pr_prompt", (Tcl_CmdProc *) prPromptCmd,
                      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* Custom prompt, to make sure we are in text mode when entering commands */

    Tcl_SetVar(interp, "tcl_prompt1", "pr_prompt", 0);

    return TCL_OK;
}
