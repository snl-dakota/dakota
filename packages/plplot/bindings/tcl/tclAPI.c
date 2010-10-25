/* $Id: tclAPI.c 3186 2006-02-15 18:17:33Z slbrow $

    Copyright 1994, 1995
    Maurice LeBrun			mjl@dino.ph.utexas.edu
    Institute for Fusion Studies	University of Texas at Austin

    Copyright (C) 2004  Joao Cardoso
    Copyright (C) 2004  Andrew Ross

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

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    This module implements a Tcl command set for interpretively calling
    PLplot functions.  Each Tcl command is responsible for calling the
    appropriate underlying function in the C API.  Can be used with any
    driver, in principle.
*/

#include "plplotP.h"
#include "pltcl.h"
#ifndef __WIN32__
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include "tclgen.h"

/* PLplot/Tcl API handlers.  Prototypes must come before Cmds struct */

static int loopbackCmd	(ClientData, Tcl_Interp *, int, char **);
static int plcontCmd	(ClientData, Tcl_Interp *, int, char **);
static int plmeshCmd	(ClientData, Tcl_Interp *, int, char **);
static int plmeshcCmd	(ClientData, Tcl_Interp *, int, char **);
static int plot3dCmd	(ClientData, Tcl_Interp *, int, char **);
static int plot3dcCmd	(ClientData, Tcl_Interp *, int, char **);
static int plsurf3dCmd	(ClientData, Tcl_Interp *, int, char **);
static int plsetoptCmd	(ClientData, Tcl_Interp *, int, char **);
static int plshadeCmd	(ClientData, Tcl_Interp *, int, char **);
static int plshadesCmd	(ClientData, Tcl_Interp *, int, char **);
static int plmapCmd	(ClientData, Tcl_Interp *, int, char **);
static int plmeridiansCmd (ClientData, Tcl_Interp *, int, char **);
static int plvectCmd   (ClientData, Tcl_Interp *, int, char **);

/*
 * The following structure defines all of the commands in the PLplot/Tcl
 * core, and the C procedures that execute them.
 */

typedef struct Command {
    int (*proc)();		/* Procedure to process command. */
    ClientData clientData;	/* Arbitrary value to pass to proc. */
    int *deleteProc;		/* Procedure to invoke when deleting
				 * command. */
    ClientData deleteData;	/* Arbitrary value to pass to deleteProc
				 * (usually the same as clientData). */
} Command;

typedef struct {
    char *name;
    int (*proc)();
} CmdInfo;

/* Built-in commands, and the procedures associated with them */

static CmdInfo Cmds[] = {
    {"loopback",	loopbackCmd},
#include "tclgen_s.h"
    {"plcol",		plcol0Cmd},
    {"plcont",		plcontCmd},
    {"plmap",		plmapCmd},
    {"plmeridians",	plmeridiansCmd},
    {"plmesh",		plmeshCmd},
    {"plmeshc",		plmeshcCmd},
    {"plot3d",		plot3dCmd},
    {"plot3dc",		plot3dcCmd},
    {"plsurf3d",	plsurf3dCmd},
    {"plsetopt",	plsetoptCmd},
    {"plshade",		plshadeCmd},
    {"plshades",	plshadesCmd},
    {"plvect",         plvectCmd},
    {NULL,		NULL}
};

/* Hash table and associated flag for directing control */

static int cmdTable_initted;
static Tcl_HashTable cmdTable;

/* Variables for holding error return info from PLplot */

static PLINT errcode;
static char errmsg[160];

/* Library initialization */

#ifndef PL_LIBRARY
#define PL_LIBRARY ""
#endif

extern char* plplotLibDir;

#if (!defined(MAC_TCL) && !defined(__WIN32__))
/*
 * Use an extended search for installations on Unix where we
 * have very likely installed plplot so that plplot.tcl is
 * in  /usr/local/plplot/lib/plplot5.1.0/tcl
 */
#define PLPLOT_EXTENDED_SEARCH
#endif

/* Static functions */

/* Evals the specified command, aborting on an error. */

static int
tcl_cmd(Tcl_Interp *interp, char *cmd);

/*--------------------------------------------------------------------------*\
 * Append_Cmdlist
 *
 * Generates command list from Cmds, storing into interp->result.
\*--------------------------------------------------------------------------*/

static void
Append_Cmdlist(Tcl_Interp *interp)
{
    static int inited = 0;
    static char** namelist;
    int i, j, ncmds = sizeof(Cmds)/sizeof(CmdInfo);

    if (!inited) {
	namelist = (char **) malloc( ncmds * sizeof(char *) );

	for( i=0; i < ncmds; i++ )
	    namelist[i] = Cmds[i].name;

    /* Sort the list, couldn't get qsort to do it for me for some reason, grrr. */

	for( i=0; i < ncmds-1; i++ )
	    for( j=i+1; j < ncmds-1; j++ ) {
		if (strcmp(namelist[i], namelist[j]) > 0) {
		    char *t = namelist[i];
		    namelist[i] = namelist[j];
		    namelist[j] = t;
		}
	    }

	inited = 1;
    }

    for( i=0; i < ncmds; i++ )
	Tcl_AppendResult(interp, " ", namelist[i], (char *) NULL);
}

/*--------------------------------------------------------------------------*\
 * plTclCmd_Init
 *
 * Sets up command hash table for use with plframe to PLplot Tcl API.
 *
 * Right now all API calls are allowed, although some of these may not
 * make much sense when used with a widget.
\*--------------------------------------------------------------------------*/

static void
plTclCmd_Init(Tcl_Interp *interp)
{
    register Command *cmdPtr;
    register CmdInfo *cmdInfoPtr;

/* Register our error variables with PLplot */

    plsError(&errcode, errmsg);

/* Initialize hash table */

    Tcl_InitHashTable(&cmdTable, TCL_STRING_KEYS);

/* Create the hash table entry for each command */

    for (cmdInfoPtr = Cmds; cmdInfoPtr->name != NULL; cmdInfoPtr++) {
	int new;
	Tcl_HashEntry *hPtr;

	hPtr = Tcl_CreateHashEntry(&cmdTable, cmdInfoPtr->name, &new);
	if (new) {
	    cmdPtr = (Command *) ckalloc(sizeof(Command));
	    cmdPtr->proc = cmdInfoPtr->proc;
	    cmdPtr->clientData = (ClientData) NULL;
	    cmdPtr->deleteProc = NULL;
	    cmdPtr->deleteData = (ClientData) NULL;
	    Tcl_SetHashValue(hPtr, cmdPtr);
	}
    }
}

/*--------------------------------------------------------------------------*\
 * plTclCmd
 *
 * Front-end to PLplot/Tcl API for use from Tcl commands (e.g. plframe).
 *
 * This command is called by the plframe widget to process subcommands
 * of the "cmd" plframe widget command.  This is the plframe's direct
 * plotting interface to the PLplot library.  This routine can be called
 * from other commands that want a similar capability.
 *
 * In a widget-based application, a PLplot "command" doesn't make much
 * sense by itself since it isn't connected to a specific widget.
 * Instead, you have widget commands.  This allows arbitrarily many
 * widgets and requires a slightly different syntax than if there were
 * only a single output device.  That is, the widget name (and in this
 * case, the "cmd" widget command, after that comes the subcommand)
 * must come first.  The plframe widget checks first for one of its
 * internal subcommands, those specifically designed for use with the
 * plframe widget.  If not found, control comes here.
\*--------------------------------------------------------------------------*/

int
plTclCmd(char *cmdlist, Tcl_Interp *interp, int argc, char **argv)
{
    register Tcl_HashEntry *hPtr;
    int result = TCL_OK;

    errcode = 0; errmsg[0] = '\0';

/* Create hash table on first call */

    if ( ! cmdTable_initted) {
	cmdTable_initted = 1;
	plTclCmd_Init(interp);
    }

/* no option -- return list of available PLplot commands */

    if (argc == 0) {
	Tcl_AppendResult(interp, cmdlist, (char *) NULL);
	Append_Cmdlist(interp);
	return TCL_OK;
    }

/* Pick out the desired command */

    hPtr = Tcl_FindHashEntry(&cmdTable, argv[0]);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "bad option \"", argv[0],
			 "\" to \"cmd\": must be one of ",
			 cmdlist, (char *) NULL);
	Append_Cmdlist(interp);
	result = TCL_ERROR;
    }
    else {
	register Command *cmdPtr = (Command *) Tcl_GetHashValue(hPtr);
	result = (*cmdPtr->proc)(cmdPtr->clientData, interp, argc, argv);
	if (result == TCL_OK) {
	    if (errcode != 0) {
		result = TCL_ERROR;
		Tcl_AppendResult(interp, errmsg, (char *) NULL);
	    }
	}
    }

    return result;
}

/*--------------------------------------------------------------------------*\
 * loopbackCmd
 *
 * Loop-back command for Tcl interpreter.  Main purpose is to enable a
 * compatible command syntax whether you are executing directly through a
 * Tcl interpreter or a plframe widget.  I.e. the syntax is:
 *
 *	<widget> cmd <PLplot command>		(widget command)
 *	loopback cmd <PLplot command>		(pltcl command)
 *
 * This routine is essentially the same as plTclCmd but without some of
 * the window dressing required by the plframe widget.
\*--------------------------------------------------------------------------*/

static int
loopbackCmd(ClientData clientData, Tcl_Interp *interp,
	    int argc, char **argv)
{
    register Tcl_HashEntry *hPtr;
    int result = TCL_OK;

    argc--; argv++;
    if (argc == 0 || (strcmp(argv[0], "cmd") != 0)) {
	Tcl_AppendResult(interp, "bad option \"", argv[0],
			 "\" to \"loopback\": must be ",
			 "\"cmd ?options?\" ", (char *) NULL);
	return TCL_ERROR;
    }

/* Create hash table on first call */

    if ( ! cmdTable_initted) {
	cmdTable_initted = 1;
	plTclCmd_Init(interp);
    }

/* no option -- return list of available PLplot commands */

    argc--; argv++;
    if (argc == 0) {
	Append_Cmdlist(interp);
	return TCL_OK;
    }

/* Pick out the desired command */

    hPtr = Tcl_FindHashEntry(&cmdTable, argv[0]);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "bad option \"", argv[0],
			 "\" to \"loopback cmd\": must be one of ",
			 (char *) NULL);
	Append_Cmdlist(interp);
	result = TCL_ERROR;
    }
    else {
	register Command *cmdPtr = (Command *) Tcl_GetHashValue(hPtr);
	result = (*cmdPtr->proc)(cmdPtr->clientData, interp, argc, argv);
    }

    return result;
}

/*--------------------------------------------------------------------------*\
 * PlbasicInit
 *
 * Used by both Pltcl and Pltk.  Ensures we have been correctly loaded
 * into a Tcl/Tk interpreter, that the plplot.tcl startup file can be
 * found and sourced, and that the Matrix library can be found and used,
 * and that it correctly exports a stub table.
\*--------------------------------------------------------------------------*/

int
PlbasicInit( Tcl_Interp *interp )
{
    int debug = plsc->debug;
    char *libDir = NULL;
    static char initScript[] =
    "tcl_findLibrary plplot " VERSION " \"\" plplot.tcl PL_LIBRARY pllibrary";
#ifdef PLPLOT_EXTENDED_SEARCH
    static char initScriptExtended[] =
    "tcl_findLibrary plplot " VERSION "/tcl \"\" plplot.tcl PL_LIBRARY pllibrary";
#endif

#ifdef USE_TCL_STUBS
/*
 * We hard-wire 8.1 here, rather than TCL_VERSION, TK_VERSION because
 * we really don't mind which version of Tcl, Tk we use as long as it
 * is 8.1 or newer.  Otherwise if we compiled against 8.2, we couldn't
 * be loaded into 8.1
 */
    Tcl_InitStubs(interp,"8.1",0);
#endif

#if 1
    if (Matrix_Init(interp) != TCL_OK) {
	if (debug) fprintf(stderr, "error in matrix init\n");
        return TCL_ERROR;
    }
#else

/*
 * This code is really designed to be used with a stubified Matrix
 * extension.  It is not well tested under a non-stubs situation
 * (which is in any case inferior).  The USE_MATRIX_STUBS define
 * is made in pltcl.h, and should be removed only with extreme caution.
 */
#ifdef USE_MATRIX_STUBS
    if (Matrix_InitStubs(interp,"0.1",0) == NULL) {
	if (debug) fprintf(stderr, "error in matrix stubs init\n");
        return TCL_ERROR;
    }
#else
    Tcl_PkgRequire(interp,"Matrix","0.1",0);
#endif
#endif

    Tcl_SetVar(interp, "plversion", VERSION, TCL_GLOBAL_ONLY);

/* Begin search for init script */
/* Each search begins with a test of libDir, so rearrangement is easy. */
/* If search is successful, both libDir (C) and pllibrary (tcl) are set */

/* if we are in the build tree, search there */
    if (plInBuildTree()) {
	if (debug) fprintf(stderr, "trying BUILD_DIR\n");
	libDir = BUILD_DIR "/bindings/tcl";
	Tcl_SetVar(interp, "pllibrary", libDir, TCL_GLOBAL_ONLY);
	if (Tcl_Eval(interp, initScript) != TCL_OK) {
	    libDir = NULL;
	    Tcl_UnsetVar(interp, "pllibrary", TCL_GLOBAL_ONLY);
	    Tcl_ResetResult(interp);
	}
    }

/* Tcl extension dir and/or PL_LIBRARY */
    if (libDir == NULL) {
	if (debug) fprintf(stderr, "trying init script\n");
	if (Tcl_Eval(interp, initScript) != TCL_OK) {
	/* This unset is needed for Tcl < 8.4 support. */
	    Tcl_UnsetVar(interp, "pllibrary", TCL_GLOBAL_ONLY);
	/* Clear the result to get rid of the error message */
	    Tcl_ResetResult(interp);
	}
	else
	    libDir = (char*) Tcl_GetVar(interp, "pllibrary", TCL_GLOBAL_ONLY);
    }

#ifdef TCL_DIR
/* Install directory */
    if (libDir == NULL) {
	if (debug) fprintf(stderr, "trying TCL_DIR\n");
	libDir = TCL_DIR;
	Tcl_SetVar(interp, "pllibrary", libDir, TCL_GLOBAL_ONLY);
	if (Tcl_Eval(interp, initScript) != TCL_OK) {
	    libDir = NULL;
	    Tcl_UnsetVar(interp, "pllibrary", TCL_GLOBAL_ONLY);
	    Tcl_ResetResult(interp);
	}
    }
#endif

#ifdef PLPLOT_EXTENDED_SEARCH
/* Unix extension directory */
    if (libDir == NULL) {
	if (debug) fprintf(stderr, "trying extended init script\n");
	if (Tcl_Eval(interp, initScriptExtended) != TCL_OK) {
	/* This unset is needed for Tcl < 8.4 support. */
	    Tcl_UnsetVar(interp, "pllibrary", TCL_GLOBAL_ONLY);
	/* Clear the result to get rid of the error message */
	    Tcl_ResetResult(interp);
	}
	else
	    libDir = (char *) Tcl_GetVar(interp, "pllibrary", TCL_GLOBAL_ONLY);
    }

/* Last chance, current directory */
    if (libDir == NULL) {
	Tcl_DString ds;
	if (debug) fprintf(stderr, "trying curdir\n");
	if (Tcl_Access("plplot.tcl", 0) != 0) {
	    if (debug) fprintf(stderr, "couldn't find plplot.tcl in curdir\n");
	    return TCL_ERROR;
	}

    /* It seems to be here.  Set pllibrary & eval plplot.tcl "by hand" */
	libDir = Tcl_GetCwd(interp, &ds);
	if (libDir == NULL) {
	    if (debug) fprintf(stderr, "couldn't get curdir\n");
	    return TCL_ERROR;
	}
	libDir = plstrdup(libDir);
	Tcl_DStringFree(&ds);
	Tcl_SetVar(interp, "pllibrary", libDir, TCL_GLOBAL_ONLY);

	if (Tcl_EvalFile(interp, "plplot.tcl") != TCL_OK) {
	    if (debug) fprintf(stderr, "error evalling plplot.tcl\n");
	    return TCL_ERROR;
	}
    }
#endif

    if (libDir == NULL) {
	if (debug) fprintf(stderr, "libdir NULL at end of search\n");
	return TCL_ERROR;
    }

/* Used by init code in plctrl.c */
    plplotLibDir = plstrdup(libDir);

/* wait_until -- waits for a specific condition to arise */
/* Can be used with either Tcl-DP or TK */

    Tcl_CreateCommand(interp, "wait_until", (Tcl_CmdProc *) plWait_Until,
		      (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * Pltcl_Init
 *
 * Initialization routine for extended tclsh's.
 * Sets up auto_path, creates the matrix command and numerous commands for
 * interfacing to PLplot.  Should not be used in a widget-based system.
\*--------------------------------------------------------------------------*/

int
Pltcl_Init( Tcl_Interp *interp )
{
    register CmdInfo *cmdInfoPtr;
/* This must be before any other Tcl related calls */
    if (PlbasicInit(interp) != TCL_OK) {
	return TCL_ERROR;
    }

/* Register our error variables with PLplot */

    plsError(&errcode, errmsg);

/* PLplot API commands */

    for (cmdInfoPtr = Cmds; cmdInfoPtr->name != NULL; cmdInfoPtr++) {

	Tcl_CreateCommand(interp, cmdInfoPtr->name, cmdInfoPtr->proc,
			  (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
    }

/* We really need this so the TEA based 'make install' can
 * properly determine the package we have installed */

    Tcl_PkgProvide(interp, "Pltcl", VERSION);
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
 *  wait_until {[info exists foobar]}
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

	Tcl_DoOneEvent(0);
    }
    return TCL_OK;
}

/*----------------------------------------------------------------------*\
 * pls_auto_path
 *
 * Sets up auto_path variable.
 * Directories are added to the FRONT of autopath.  Therefore, they are
 * searched in reverse order of how they are listed below.
 *
 * Note: there is no harm in adding extra directories, even if they don't
 * actually exist (aside from a slight increase in processing time when
 * the autoloaded proc is first found).
\*----------------------------------------------------------------------*/

int
pls_auto_path(Tcl_Interp *interp)
{
    char *buf, *ptr=NULL, *dn;
#ifdef DEBUG
    char *path;
#endif

    buf = (char *) malloc(256 * sizeof(char));

/* Add TCL_DIR */

#ifdef TCL_DIR
    Tcl_SetVar(interp, "dir", TCL_DIR, TCL_GLOBAL_ONLY);
    if (tcl_cmd(interp, "set auto_path \"$dir $auto_path\"") == TCL_ERROR)
	return TCL_ERROR;
#ifdef DEBUG
    fprintf(stderr, "adding %s to auto_path\n", TCL_DIR);
    path = Tcl_GetVar(interp, "auto_path", 0);
    fprintf(stderr, "auto_path is %s\n", path);
#endif
#endif

/* Add $HOME/tcl */

    if ((dn = getenv("HOME")) != NULL) {
	plGetName(dn, "tcl", "", &ptr);
	Tcl_SetVar(interp, "dir", ptr, 0);
	if (tcl_cmd(interp, "set auto_path \"$dir $auto_path\"") == TCL_ERROR)
	    return TCL_ERROR;
#ifdef DEBUG
	fprintf(stderr, "adding %s to auto_path\n", ptr);
	path = Tcl_GetVar(interp, "auto_path", 0);
	fprintf(stderr, "auto_path is %s\n", path);
#endif
    }

/* Add PL_TCL_ENV = $(PL_TCL) */

#if defined (PL_TCL_ENV)
    if ((dn = getenv(PL_TCL_ENV)) != NULL) {
	plGetName(dn, "", "", &ptr);
	Tcl_SetVar(interp, "dir", ptr, 0);
	if (tcl_cmd(interp, "set auto_path \"$dir $auto_path\"") == TCL_ERROR)
	    return TCL_ERROR;
#ifdef DEBUG
	fprintf(stderr, "adding %s to auto_path\n", ptr);
	path = Tcl_GetVar(interp, "auto_path", 0);
	fprintf(stderr, "auto_path is %s\n", path);
#endif
    }
#endif  /* PL_TCL_ENV */

/* Add PL_HOME_ENV/tcl = $(PL_HOME_ENV)/tcl */

#if defined (PL_HOME_ENV)
    if ((dn = getenv(PL_HOME_ENV)) != NULL) {
	plGetName(dn, "tcl", "", &ptr);
	Tcl_SetVar(interp, "dir", ptr, 0);
	if (tcl_cmd(interp, "set auto_path \"$dir $auto_path\"") == TCL_ERROR)
	    return TCL_ERROR;
#ifdef DEBUG
	fprintf(stderr, "adding %s to auto_path\n", ptr);
	path = Tcl_GetVar(interp, "auto_path", 0);
	fprintf(stderr, "auto_path is %s\n", path);
#endif
    }
#endif  /* PL_HOME_ENV */

/* Add cwd */

    if (getcwd(buf, 256) == 0) {
	Tcl_SetResult(interp, "Problems with getcwd in pls_auto_path", TCL_STATIC);
	return TCL_ERROR;
    }

    Tcl_SetVar(interp, "dir", buf, 0);
    if (tcl_cmd(interp, "set auto_path \"$dir $auto_path\"") == TCL_ERROR)
	return TCL_ERROR;

    /*** see if plserver was invoked in the build tree ***/
    if (plInBuildTree()) {
      Tcl_SetVar(interp, "dir", BUILD_DIR "/bindings/tk", TCL_GLOBAL_ONLY);
      if (tcl_cmd(interp, "set auto_path \"$dir $auto_path\"") == TCL_ERROR)
     	return TCL_ERROR;
    }

#ifdef DEBUG
    fprintf(stderr, "adding %s to auto_path\n", buf);
    path = Tcl_GetVar(interp, "auto_path", 0);
    fprintf(stderr, "auto_path is %s\n", path);
#endif

    free_mem(buf);
    free_mem(ptr);

    return TCL_OK;
}

/*----------------------------------------------------------------------*\
 * tcl_cmd
 *
 * Evals the specified command, aborting on an error.
\*----------------------------------------------------------------------*/

static int
tcl_cmd(Tcl_Interp *interp, char *cmd)
{
    int result;

    result = Tcl_VarEval(interp, cmd, (char **) NULL);
    if (result != TCL_OK) {
	fprintf(stderr, "TCL command \"%s\" failed:\n\t %s\n",
		cmd, interp->result);
    }
    return result;
}

/*--------------------------------------------------------------------------*\
 * PLplot API Calls
 *
 * Any call that results in something actually being plotted must be
 * followed by by a call to plflush(), to make sure all output from
 * that command is finished.  Devices that have text/graphics screens
 * (e.g. Tek4xxx and emulators) implicitly switch to the graphics screen
 * before graphics commands, so a plgra() is not necessary in this case.
 * Although if you switch to the text screen via user control (instead of
 * using pltext()), the device will get confused.
\*--------------------------------------------------------------------------*/

static char buf[200];

#include "tclgen.c"

/*--------------------------------------------------------------------------*\
 * plcontCmd
 *
 * Processes plcont Tcl command.
 *
 * The C function is:
 * void
 * c_plcont(PLFLT **f, PLINT nx, PLINT ny, PLINT kx, PLINT lx,
 * 	 PLINT ky, PLINT ly, PLFLT *clevel, PLINT nlevel,
 * 	 void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
 * 	 PLPointer pltr_data);
 *
 * Since f will be specified by a Tcl Matrix, nx and ny are redundant, and
 * are automatically eliminated.  Same for nlevel, since clevel will be a 1-d
 * Tcl Matrix.  Since most people plot the whole data set, we will allow kx,
 * lx and ky, ly to be defaulted--either you specify all four, or none of
 * them.  We allow three ways of specifying the coordinate transforms: 1)
 * Nothing, in which case we will use the identity mapper pltr0 2) pltr1, in
 * which case the next two args must be 1-d Tcl Matricies 3) pltr2, in which
 * case the next two args must be 2-d Tcl Matricies.  Finally, a new
 * paramater is allowed at the end to specify which, if either, of the
 * coordinates wrap on themselves.  Can be 1 or x, or 2 or y.  Nothing or 0
 * specifies that neither coordinate wraps.
 *
 * So, the new call from Tcl is:
 * 	plcont f [kx lx ky ly] clev [pltr x y] [wrap]
 *
\*--------------------------------------------------------------------------*/

static int tclmateval_modx, tclmateval_mody;

PLFLT tclMatrix_feval (PLINT i, PLINT j, PLPointer p)
{
    tclMatrix *matPtr = (tclMatrix *) p;

    i = i % tclmateval_modx;
    j = j % tclmateval_mody;

/*    printf( "tclMatrix_feval: i=%d j=%d f=%f\n", i, j,
      matPtr->fdata[I2D(i,j)] );
	    */
    return matPtr->fdata[I2D(i,j)];
}

static int
plcontCmd( ClientData clientData, Tcl_Interp *interp,
	   int argc, char *argv[] )
{
    tclMatrix *matPtr, *matf, *matclev;
    PLINT nx, ny, kx=0, lx=0, ky=0, ly=0, nclev;
    char *pltrname = "pltr0";
    tclMatrix *mattrx = NULL, *mattry = NULL;
    PLFLT **z, **zused, **zwrapped;

    int arg3_is_kx = 1, i, j;
    void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer);
    PLPointer pltr_data = NULL;
    PLcGrid  cgrid1;
    PLcGrid2 cgrid2;

    int wrap = 0;

    if (argc  < 3 ) {
	Tcl_AppendResult( interp, "wrong # args: see documentation for ",
			 argv[0], (char *) NULL);
	return TCL_ERROR;
    }

    matf = Tcl_GetMatrixPtr( interp, argv[1] );
    if (matf == NULL) return TCL_ERROR;

    if (matf->dim != 2) {
	interp->result = "Must use 2-d data.";
	return TCL_ERROR;
    } else {
	nx = matf->n[0];
	ny = matf->n[1];
	tclmateval_modx = nx;
	tclmateval_mody = ny;

        /* convert matf to 2d-array so can use standard wrap approach
	 * from now on in this code. */
        plAlloc2dGrid(&z, nx, ny );
	for (i=0; i < nx; i++) {
	   for (j=0; j < ny; j++) {
	      z[i][j] = tclMatrix_feval(i, j, matf);
	   }
	}
    }

/* Now check the next argument.  If it is all digits, then it must be kx,
   otherwise it is the name of clev. */

    for( i=0; i < (int) strlen( argv[2] ) && arg3_is_kx; i++ )
	if (!isdigit(argv[2][i]))
	    arg3_is_kx = 0;

    if (arg3_is_kx) {
    /* Check that there are enough args */
	if (argc < 7) {
	    interp->result = "plcont, bogus syntax";
	    return TCL_ERROR;
	}

    /* Peel off the ones we need */
	kx = atoi( argv[3] );
	lx = atoi( argv[4] );
	ky = atoi( argv[5] );
	ly = atoi( argv[6] );

    /* adjust argc, argv to reflect our consumption */
	argc -= 6, argv += 6;
    } else {
	argc -= 2, argv += 2;
    }

/* The next argument has to be clev */

    if (argc < 1) {
	interp->result = "plcont, bogus syntax";
	return TCL_ERROR;
    }

    matclev = Tcl_GetMatrixPtr( interp, argv[0] );
    if (matclev == NULL) return TCL_ERROR;
    nclev = matclev->n[0];

    if (matclev->dim != 1) {
	interp->result = "clev must be 1-d matrix.";
	return TCL_ERROR;
    }

    argc--, argv++;

/* Now handle trailing optional parameters, if any */

    if (argc >= 3) {
    /* There is a pltr spec, parse it. */
	pltrname = argv[0];
	mattrx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (mattrx == NULL) return TCL_ERROR;
	mattry = Tcl_GetMatrixPtr( interp, argv[2] );
        if (mattry == NULL) return TCL_ERROR;

	argc -= 3, argv += 3;
    }

    if (argc) {
    /* There is a wrap spec, get it. */
	wrap = atoi( argv[0] );

    /* Hmm, I said the the doc they could also say x or y, have to come back
       to this... */

	argc--, argv++;
    }

/* There had better not be anything else on the command line by this point. */

    if (argc) {
	interp->result = "plcont, bogus syntax, too many args.";
	return TCL_ERROR;
    }

/* Now we need to set up the data for contouring. */

    if ( !strcmp( pltrname, "pltr0" ) ) {
	pltr = pltr0;
        zused = z;

    /* wrapping is only supported for pltr2. */
	if (wrap) {
	    interp->result = "Must use pltr2 if want wrapping.";
	    return TCL_ERROR;
	}
    }
    else if ( !strcmp( pltrname, "pltr1" ) ) {
	pltr = pltr1;
	cgrid1.xg = mattrx->fdata;
	cgrid1.nx = nx;
	cgrid1.yg = mattry->fdata;
	cgrid1.ny = ny;
        zused = z;

    /* wrapping is only supported for pltr2. */
	if (wrap) {
	    interp->result = "Must use pltr2 if want wrapping.";
	    return TCL_ERROR;
	}

	if (mattrx->dim != 1 || mattry->dim != 1) {
	    interp->result = "Must use 1-d coord arrays with pltr1.";
	    return TCL_ERROR;
	}

	pltr_data = &cgrid1;
    }
    else if ( !strcmp( pltrname, "pltr2" ) ) {
    /* printf( "plcont, setting up for pltr2\n" ); */
	if (!wrap) {
	/* printf( "plcont, no wrapping is needed.\n" ); */
	    plAlloc2dGrid( &cgrid2.xg, nx, ny );
	    plAlloc2dGrid( &cgrid2.yg, nx, ny );
	    cgrid2.nx = nx;
	    cgrid2.ny = ny;
	    zused = z;

	    matPtr = mattrx;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

	    matPtr = mattry;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
	}
	else if (wrap == 1) {
	    plAlloc2dGrid( &cgrid2.xg, nx+1, ny );
	    plAlloc2dGrid( &cgrid2.yg, nx+1, ny );
	    plAlloc2dGrid( &zwrapped, nx+1, ny );
	    cgrid2.nx = nx+1;
	    cgrid2.ny = ny;
	    zused = zwrapped;

	    matPtr = mattrx;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

	    matPtr = mattry;
	    for( i=0; i < nx; i++ ) {
		for( j=0; j < ny; j++ ) {
		    cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
	            zwrapped[i][j] = z[i][j];
		}
	    }

	    for( j=0; j < ny; j++ ) {
		cgrid2.xg[nx][j] = cgrid2.xg[0][j];
		cgrid2.yg[nx][j] = cgrid2.yg[0][j];
		zwrapped[nx][j] = zwrapped[0][j];
	    }

            /* z not used in executable path after this so free it before
	     * nx value is changed. */
	    plFree2dGrid( z, nx, ny );

	    nx++;
	}
	else if (wrap == 2) {
	    plAlloc2dGrid( &cgrid2.xg, nx, ny+1 );
	    plAlloc2dGrid( &cgrid2.yg, nx, ny+1 );
	    plAlloc2dGrid( &zwrapped, nx, ny+1 );
	    cgrid2.nx = nx;
	    cgrid2.ny = ny+1;
	    zused = zwrapped;

	    matPtr = mattrx;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

	    matPtr = mattry;
	    for( i=0; i < nx; i++ ) {
		for( j=0; j < ny; j++ ) {
		    cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
		    zwrapped[i][j] = z[i][j];
		}
	    }

	    for( i=0; i < nx; i++ ) {
		cgrid2.xg[i][ny] = cgrid2.xg[i][0];
		cgrid2.yg[i][ny] = cgrid2.yg[i][0];
		zwrapped[i][ny] = zwrapped[i][0];
	    }

            /* z not used in executable path after this so free it before
	     * ny value is changed. */
	    plFree2dGrid( z, nx, ny );

	    ny++;
	}
	else {
	    interp->result =
		"Invalid wrap specifier, must be <empty>, 0, 1, or 2.";
	    return TCL_ERROR;
	}

	pltr = pltr2;
	pltr_data = &cgrid2;
    }
    else {
	Tcl_AppendResult( interp,
			  "Unrecognized coordinate transformation spec:",
			  pltrname, ", must be pltr0 pltr1 or pltr2.",
			  (char *) NULL );
	return TCL_ERROR;
    }
    if (!arg3_is_kx) {
       /* default values must be set here since nx, ny can change with wrap. */
       kx = 1; lx = nx;
       ky = 1; ly = ny;
    }

/*    printf( "plcont: nx=%d ny=%d kx=%d lx=%d ky=%d ly=%d\n",
	    nx, ny, kx, lx, ky, ly );
    printf( "plcont: nclev=%d\n", nclev );
 */

/* contour the data.*/

    plcont( zused, nx, ny,
	     kx, lx, ky, ly,
	     matclev->fdata, nclev,
	     pltr, pltr_data );

/* Now free up any space which got allocated for our coordinate trickery. */

/* zused points to either z or zwrapped.  In both cases the allocated size
 * was nx by ny.  Now free the allocated space, and note in the case
 * where zused points to zwrapped, the separate z space has been freed by
 * previous wrap logic. */
    plFree2dGrid( zused, nx, ny );

    if (pltr == pltr1) {
    /* Hmm, actually, nothing to do here currently, since we just used the
       Tcl Matrix data directly, rather than allocating private space. */
    }
    else if (pltr == pltr2) {
    /* printf( "plcont, freeing space for grids used in pltr2\n" ); */
	plFree2dGrid( cgrid2.xg, nx, ny );
	plFree2dGrid( cgrid2.yg, nx, ny );
    }

    plflush();
    return TCL_OK;
}

/*---------------------------------------------------------------------------*\
 * plvect implementation (based on plcont above)
\*---------------------------------------------------------------------------*/
static int
plvectCmd( ClientData clientData, Tcl_Interp *interp,
	   int argc, char *argv[] )
{
    tclMatrix *matPtr, *matu, *matv;
    PLINT nx, ny;
    char *pltrname = "pltr0";
    tclMatrix *mattrx = NULL, *mattry = NULL;
    PLFLT **u, **v, **uused, **vused, **uwrapped, **vwrapped;
    PLFLT scaling;

    int i, j;
    void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer);
    PLPointer pltr_data = NULL;
    PLcGrid  cgrid1;
    PLcGrid2 cgrid2;

    int wrap = 0;

    if (argc  < 3 ) {
       Tcl_AppendResult( interp, "wrong # args: see documentation for ",
                        argv[0], (char *) NULL);
       return TCL_ERROR;
    }

    matu = Tcl_GetMatrixPtr( interp, argv[1] );
    if (matu == NULL) return TCL_ERROR;

    if (matu->dim != 2) {
       interp->result = "Must use 2-d data.";
       return TCL_ERROR;
    } else {
       nx = matu->n[0];
       ny = matu->n[1];
       tclmateval_modx = nx;
       tclmateval_mody = ny;

        /* convert matu to 2d-array so can use standard wrap approach
	 * from now on in this code. */
         plAlloc2dGrid(&u, nx, ny );
        for (i=0; i < nx; i++) {
          for (j=0; j < ny; j++) {
             u[i][j] = tclMatrix_feval(i, j, matu);
          }
       }
    }

    matv = Tcl_GetMatrixPtr( interp, argv[2] );
    if (matv == NULL) return TCL_ERROR;

    if (matv->dim != 2) {
       interp->result = "Must use 2-d data.";
       return TCL_ERROR;
    } else {
       nx = matv->n[0];
       ny = matv->n[1];
       tclmateval_modx = nx;
       tclmateval_mody = ny;

        /* convert matv to 2d-array so can use standard wrap approach
        * from now on in this code. */
        plAlloc2dGrid(&v, nx, ny );
       for (i=0; i < nx; i++) {
          for (j=0; j < ny; j++) {
             v[i][j] = tclMatrix_feval(i, j, matv);
          }
       }
    }

    argc -= 3, argv += 3;

/* The next argument has to be scaling */

    if (argc < 1) {
       interp->result = "plvect, bogus syntax";
       return TCL_ERROR;
    }

    scaling = atof(argv[0]);
    argc--, argv++;

/* Now handle trailing optional parameters, if any */

    if (argc >= 3) {
    /* There is a pltr spec, parse it. */
       pltrname = argv[0];
       mattrx = Tcl_GetMatrixPtr( interp, argv[1] );
       if (mattrx == NULL) return TCL_ERROR;
       mattry = Tcl_GetMatrixPtr( interp, argv[2] );
       if (mattry == NULL) return TCL_ERROR;

       argc -= 3, argv += 3;
    }

    if (argc) {
    /* There is a wrap spec, get it. */
       wrap = atoi( argv[0] );

    /* Hmm, I said the the doc they could also say x or y, have to come back
       to this... */

       argc--, argv++;
    }

/* There had better not be anything else on the command line by this point. */

    if (argc) {
       interp->result = "plvect, bogus syntax, too many args.";
       return TCL_ERROR;
    }

/* Now we need to set up the data for contouring. */

    if ( !strcmp( pltrname, "pltr0" ) ) {
       pltr = pltr0;
       uused = u;
       vused = v;

    /* wrapping is only supported for pltr2. */
       if (wrap) {
           interp->result = "Must use pltr2 if want wrapping.";
           return TCL_ERROR;
       }
    }
    else if ( !strcmp( pltrname, "pltr1" ) ) {
       pltr = pltr1;
       cgrid1.xg = mattrx->fdata;
       cgrid1.nx = nx;
       cgrid1.yg = mattry->fdata;
       cgrid1.ny = ny;
       uused = u;
        vused = v;

    /* wrapping is only supported for pltr2. */
       if (wrap) {
           interp->result = "Must use pltr2 if want wrapping.";
           return TCL_ERROR;
       }

       if (mattrx->dim != 1 || mattry->dim != 1) {
           interp->result = "Must use 1-d coord arrays with pltr1.";
           return TCL_ERROR;
       }

       pltr_data = &cgrid1;
    }
    else if ( !strcmp( pltrname, "pltr2" ) ) {
    /* printf( "plvect, setting up for pltr2\n" ); */
       if (!wrap) {
       /* printf( "plvect, no wrapping is needed.\n" ); */
           plAlloc2dGrid( &cgrid2.xg, nx, ny );
           plAlloc2dGrid( &cgrid2.yg, nx, ny );
           cgrid2.nx = nx;
           cgrid2.ny = ny;
           uused = u;
           vused = v;

           matPtr = mattrx;
           for( i=0; i < nx; i++ )
               for( j=0; j < ny; j++ )
                   cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];
           matPtr = mattry;
           for( i=0; i < nx; i++ ) {
               for( j=0; j < ny; j++ ) {
                   cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
	       }
	   }
       }
       else if (wrap == 1) {
	    plAlloc2dGrid( &cgrid2.xg, nx+1, ny );
	    plAlloc2dGrid( &cgrid2.yg, nx+1, ny );
	    plAlloc2dGrid( &uwrapped, nx+1, ny );
	    plAlloc2dGrid( &vwrapped, nx+1, ny );
	    cgrid2.nx = nx+1;
	    cgrid2.ny = ny;
	    uused = uwrapped;
	    vused = vwrapped;


	    matPtr = mattrx;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

	    matPtr = mattry;
	    for( i=0; i < nx; i++ ) {
		for( j=0; j < ny; j++ ) {
		   cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
                   uwrapped[i][j] = u[i][j];
                   vwrapped[i][j] = v[i][j];
               }
           }

           for( j=0; j < ny; j++ ) {
               cgrid2.xg[nx][j] = cgrid2.xg[0][j];
               cgrid2.yg[nx][j] = cgrid2.yg[0][j];
               uwrapped[nx][j] = uwrapped[0][j];
               vwrapped[nx][j] = vwrapped[0][j];
           }

            /* u and v not used in executable path after this so free it
            * before nx value is changed. */
           plFree2dGrid( u, nx, ny );
           plFree2dGrid( v, nx, ny );
           nx++;
       }
       else if (wrap == 2) {
           plAlloc2dGrid( &cgrid2.xg, nx, ny+1 );
           plAlloc2dGrid( &cgrid2.yg, nx, ny+1 );
           plAlloc2dGrid( &uwrapped, nx, ny+1 );
           plAlloc2dGrid( &vwrapped, nx, ny+1 );
           cgrid2.nx = nx;
           cgrid2.ny = ny+1;
           uused = uwrapped;
           vused = vwrapped;

           matPtr = mattrx;
           for( i=0; i < nx; i++ )
               for( j=0; j < ny; j++ )
                   cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

           matPtr = mattry;
           for( i=0; i < nx; i++ ) {
               for( j=0; j < ny; j++ ) {
                   cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
                   uwrapped[i][j] = u[i][j];
                   vwrapped[i][j] = v[i][j];
               }
           }

           for( i=0; i < nx; i++ ) {
               cgrid2.xg[i][ny] = cgrid2.xg[i][0];
               cgrid2.yg[i][ny] = cgrid2.yg[i][0];
               uwrapped[i][ny] = uwrapped[i][0];
               vwrapped[i][ny] = vwrapped[i][0];
           }

            /* u and v not used in executable path after this so free it
            * before ny value is changed. */
           plFree2dGrid( u, nx, ny );
           plFree2dGrid( v, nx, ny );

           ny++;
       }
       else {
           interp->result =
               "Invalid wrap specifier, must be <empty>, 0, 1, or 2.";
           return TCL_ERROR;
       }

       pltr = pltr2;
       pltr_data = &cgrid2;
    }
    else {
       Tcl_AppendResult( interp,
                         "Unrecognized coordinate transformation spec:",
                         pltrname, ", must be pltr0 pltr1 or pltr2.",
                         (char *) NULL );
       return TCL_ERROR;
    }


/* plot the vector data.*/

    plvect( uused, vused, nx, ny,
           scaling, pltr, pltr_data );
/* Now free up any space which got allocated for our coordinate trickery. */

/* uused points to either u or uwrapped.  In both cases the allocated size
 * was nx by ny.  Now free the allocated space, and note in the case
 * where uused points to uwrapped, the separate u space has been freed by
 * previous wrap logic. */
    plFree2dGrid( uused, nx, ny );
    plFree2dGrid( vused, nx, ny );

    if (pltr == pltr1) {
    /* Hmm, actually, nothing to do here currently, since we just used the
       Tcl Matrix data directly, rather than allocating private space. */
    }
    else if (pltr == pltr2) {
    /* printf( "plvect, freeing space for grids used in pltr2\n" ); */
       plFree2dGrid( cgrid2.xg, nx, ny );
       plFree2dGrid( cgrid2.yg, nx, ny );
    }

    plflush();
    return TCL_OK;
}

/* ------------------------------------------------------------------------*\
 *
* plmeshCmd
 *
 * Processes plmesh Tcl command.
 *
 * We support 3 different invocation forms:
 * 1)	plmesh x y z nx ny opt
 * 2)	plmesh x y z opt
 * 3)	plmesh z opt
 *
 * Form 1) is an exact mirror of the usual C API.  In form 2) we infer nx and
 * ny from the input data, and in form 3 we inver nx and ny, and also take
 * the x and y arrays to just be integral spacing.
\*--------------------------------------------------------------------------*/

static int
plmeshCmd( ClientData clientData, Tcl_Interp *interp,
	   int argc, char *argv[] )
{
    PLINT nx, ny, opt;
    PLFLT *x, *y, **z;
    tclMatrix *matx, *maty, *matz, *matPtr;
    int i;

    if (argc == 7) {
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
        if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
        if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ) {
	    interp->result = "x y and z must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }
    else if (argc == 5) {
	opt  = atoi( argv[4] );

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ) {
	    interp->result = "x y and z must all be float";
	    return TCL_ERROR;
	}

	nx = matx->n[0]; ny = maty->n[0];

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }
    else if (argc == 3) {
	interp->result = "unimplemented";
	return TCL_ERROR;
    }
    else {
	Tcl_AppendResult(interp, "wrong # args: should be \"plmesh ",
			 "x y z nx ny opt\", or a valid contraction ",
			 "thereof.", (char *) NULL);
	return TCL_ERROR;
    }

    plmesh( x, y, z, nx, ny, opt );

    if (argc == 7) {
	free(z);
    }
    else if (argc == 5) {
	free(z);
    }
    else {			/* argc == 3 */
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plmeshcCmd
 *
 * Processes plmeshc Tcl command.
 *
 * We support 5 different invocation forms:
 * 1)	plmeshc x y z nx ny opt clevel nlevel
 * 2)	plmeshc x y z nx ny opt clevel
 * 3)	plmeshc x y z nx ny opt
 * 4)	plmeshc x y z opt
 * 5)	plmeshc z opt
 *
 * Form 1) is an exact mirror of the usual C API.  In form 2) we infer nlevel.
 * In form 3,4 and 5 clevel is set to NULL. In form 4 we infer nx and
 * ny from the input data, and in form 5 we infer nx and ny, and also take
 * the x and y arrays to just be integral spacing.
\*--------------------------------------------------------------------------*/

static int
plmeshcCmd( ClientData clientData, Tcl_Interp *interp,
	   int argc, char *argv[] )
{
    PLINT nx, ny, opt, nlev=10;
    PLFLT *x, *y, **z;
    PLFLT *clev;

    tclMatrix *matx, *maty, *matz, *matPtr, *matlev;
    int i;

    if (argc == 9) {
	nlev = atoi( argv[8] );
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	matlev = Tcl_GetMatrixPtr( interp, argv[7] );
	if (matlev == NULL) return TCL_ERROR;

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ||
	     matlev->type != TYPE_FLOAT) {
	    interp->result = "x y z and clevel must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
	     matlev->dim != 1 || matlev->n[0] != nlev) {
	    interp->result = "popo Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;
	clev = matlev->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }

    else if (argc == 8) {
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */
	matlev = Tcl_GetMatrixPtr( interp, argv[7] );
	if (matlev == NULL) return TCL_ERROR;

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ||
	     matlev->type != TYPE_FLOAT) {
	    interp->result = "x y z and clevel must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
	     matlev->dim != 1 || matlev->n[0] != nlev) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;
	clev = matlev->fdata;
	nlev = matlev->n[0];

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }

    else if (argc == 7) {
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );
        clev = NULL;

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ) {
	    interp->result = "x y and z must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }
    else if (argc == 5) {
	opt = atoi( argv[4] );
        clev = NULL;

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ) {
	    interp->result = "x y and z must all be float";
	    return TCL_ERROR;
	}

	nx = matx->n[0]; ny = maty->n[0];

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }
    else if (argc == 3) {
	interp->result = "unimplemented";
	return TCL_ERROR;
    }
    else {
	Tcl_AppendResult(interp, "wrong # args: should be \"plmeshc ",
			 "x y z nx ny opt clevel nlevel\", or a valid contraction ",
			 "thereof.", (char *) NULL);
	return TCL_ERROR;
    }

    plmeshc( x, y, z, nx, ny, opt, clev, nlev);

    if (argc == 7) {
	free(z);
    }
    else if (argc == 5) {
	free(z);
    }
    else {			/* argc == 3 */
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plot3dCmd
 *
 * Processes plot3d Tcl command.
 *
 * We support 3 different invocation forms:
 * 1)	plot3d x y z nx ny opt side
 * 2)	plot3d x y z opt side
 * 3)	plot3d z opt side
 *
 * Form 1) is an exact mirror of the usual C API.  In form 2) we infer nx and
 * ny from the input data, and in form 3 we inver nx and ny, and also take
 * the x and y arrays to just be integral spacing.
\*--------------------------------------------------------------------------*/

static int
plot3dCmd( ClientData clientData, Tcl_Interp *interp,
	   int argc, char *argv[] )
{
    PLINT nx, ny, opt, side;
    PLFLT *x, *y, **z;
    tclMatrix *matx, *maty, *matz, *matPtr;
    int i;

    if (argc == 8) {
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );
	side = atoi( argv[7] );

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ) {
	    interp->result = "x y and z must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }
    else if (argc == 6) {
	opt  = atoi( argv[4] );
	side = atoi( argv[5] );

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ) {
	    interp->result = "x y and z must all be float";
	    return TCL_ERROR;
	}

	nx = matx->n[0]; ny = maty->n[0];

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }
    else if (argc == 4) {
	interp->result = "unimplemented";
	return TCL_ERROR;
    }
    else {
	Tcl_AppendResult(interp, "wrong # args: should be \"plot3d ",
			 "x y z nx ny opt side\", or a valid contraction ",
			 "thereof.", (char *) NULL);
	return TCL_ERROR;
    }

    plot3d( x, y, z, nx, ny, opt, side );

    if (argc == 8) {
	free(z);
    }
    else if (argc == 6) {
	free(z);
    }
    else {			/* argc == 4 */
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plot3dcCmd
 *
 * Processes plot3dc Tcl command.
 *
 * We support 5 different invocation forms:
 * 1)	plot3dc x y z nx ny opt clevel nlevel
 * 2)	plot3dc x y z nx ny opt clevel
 * 3)	plot3dc x y z nx ny opt
 * 4)	plot3dc x y z opt
 * 5)	plot3dc z opt
 *
 * Form 1) is an exact mirror of the usual C API.  In form 2) we infer nlevel.
 * In form 3,4 and 5 clevel is set to NULL. In form 4 we infer nx and
 * ny from the input data, and in form 5 we infer nx and ny, and also take
 * the x and y arrays to just be integral spacing.
\*--------------------------------------------------------------------------*/

static int
plot3dcCmd( ClientData clientData, Tcl_Interp *interp,
	   int argc, char *argv[] )
{
    PLINT nx, ny, opt, nlev=10;
    PLFLT *x, *y, **z;
    PLFLT *clev;

    tclMatrix *matx, *maty, *matz, *matPtr, *matlev;
    int i;

    if (argc == 9) {
	nlev = atoi( argv[8] );
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	matlev = Tcl_GetMatrixPtr( interp, argv[7] );
	if (matlev == NULL) return TCL_ERROR;

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ||
	     matlev->type != TYPE_FLOAT) {
	    interp->result = "x y z and clevel must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
	     matlev->dim != 1 || matlev->n[0] != nlev) {
	    interp->result = "popo Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;
	clev = matlev->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }

    else if (argc == 8) {
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */
	matlev = Tcl_GetMatrixPtr( interp, argv[7] );
	if (matlev == NULL) return TCL_ERROR;

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ||
	     matlev->type != TYPE_FLOAT) {
	    interp->result = "x y z and clevel must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
	     matlev->dim != 1 || matlev->n[0] != nlev) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;
	clev = matlev->fdata;
	nlev = matlev->n[0];

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }

    else if (argc == 7) {
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );
        clev = NULL;

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ) {
	    interp->result = "x y and z must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }
    else if (argc == 5) {
	opt = atoi( argv[4] );
        clev = NULL;

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ) {
	    interp->result = "x y and z must all be float";
	    return TCL_ERROR;
	}

	nx = matx->n[0]; ny = maty->n[0];

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }
    else if (argc == 3) {
	interp->result = "unimplemented";
	return TCL_ERROR;
    }
    else {
	Tcl_AppendResult(interp, "wrong # args: should be \"plot3dc ",
			 "x y z nx ny opt clevel nlevel\", or a valid contraction ",
			 "thereof.", (char *) NULL);
	return TCL_ERROR;
    }

    plot3dc( x, y, z, nx, ny, opt, clev, nlev);

    if (argc == 7) {
	free(z);
    }
    else if (argc == 5) {
	free(z);
    }
    else {			/* argc == 3 */
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsurf3dCmd
 *
 * Processes plsurf3d Tcl command.
 *
 * We support 5 different invocation forms:
 * 1)	plsurf3d x y z nx ny opt clevel nlevel
 * 2)	plsurf3d x y z nx ny opt clevel
 * 3)	plsurf3d x y z nx ny opt
 * 4)	plsurf3d x y z opt
 * 5)	plsurf3d z opt
 *
 * Form 1) is an exact mirror of the usual C API.  In form 2) we infer nlevel.
 * In form 3,4 and 5 clevel is set to NULL. In form 4 we infer nx and
 * ny from the input data, and in form 5 we infer nx and ny, and also take
 * the x and y arrays to just be integral spacing.
\*--------------------------------------------------------------------------*/

static int
plsurf3dCmd( ClientData clientData, Tcl_Interp *interp,
	   int argc, char *argv[] )
{
    PLINT nx, ny, opt, nlev=10;
    PLFLT *x, *y, **z;
    PLFLT *clev;

    tclMatrix *matx, *maty, *matz, *matPtr, *matlev;
    int i;

    if (argc == 9) {
	nlev = atoi( argv[8] );
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	matlev = Tcl_GetMatrixPtr( interp, argv[7] );
	if (matlev == NULL) return TCL_ERROR;

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ||
	     matlev->type != TYPE_FLOAT) {
	    interp->result = "x y z and clevel must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
	     matlev->dim != 1 || matlev->n[0] != nlev) {
	    interp->result = "popo Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;
	clev = matlev->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }

    else if (argc == 8) {
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */
	matlev = Tcl_GetMatrixPtr( interp, argv[7] );
	if (matlev == NULL) return TCL_ERROR;

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ||
	     matlev->type != TYPE_FLOAT) {
	    interp->result = "x y z and clevel must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ||
	     matlev->dim != 1 || matlev->n[0] != nlev) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;
	clev = matlev->fdata;
	nlev = matlev->n[0];

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }

    else if (argc == 7) {
	nx   = atoi( argv[4] );
	ny   = atoi( argv[5] );
	opt  = atoi( argv[6] );
        clev = NULL;

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ) {
	    interp->result = "x y and z must all be float";
	    return TCL_ERROR;
	}

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }
    else if (argc == 5) {
	opt = atoi( argv[4] );
        clev = NULL;

	matx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (matx == NULL) return TCL_ERROR;
	maty = Tcl_GetMatrixPtr( interp, argv[2] );
	if (maty == NULL) return TCL_ERROR;
	matz = Tcl_GetMatrixPtr( interp, argv[3] );
	if (matz == NULL) return TCL_ERROR;
	matPtr = matz;		/* For dumb indexer macro, grrrr. */

	if ( matx->type != TYPE_FLOAT ||
	     maty->type != TYPE_FLOAT ||
	     matz->type != TYPE_FLOAT ) {
	    interp->result = "x y and z must all be float";
	    return TCL_ERROR;
	}

	nx = matx->n[0]; ny = maty->n[0];

	if ( matx->dim != 1 || matx->n[0] != nx ||
	     maty->dim != 1 || maty->n[0] != ny ||
	     matz->dim != 2 || matz->n[0] != nx || matz->n[1] != ny ) {
	    interp->result = "Inconsistent dimensions";
	    return TCL_ERROR;
	}

	x = matx->fdata;
	y = maty->fdata;

	z = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
	for( i=0; i < nx; i++ )
	    z[i] = &matz->fdata[ I2D(i,0) ];
    }
    else if (argc == 3) {
	interp->result = "unimplemented";
	return TCL_ERROR;
    }
    else {
	Tcl_AppendResult(interp, "wrong # args: should be \"plsurf3d ",
			 "x y z nx ny opt clevel nlevel\", or a valid contraction ",
			 "thereof.", (char *) NULL);
	return TCL_ERROR;
    }

    plsurf3d( x, y, z, nx, ny, opt, clev, nlev);

    if (argc == 7) {
	free(z);
    }
    else if (argc == 5) {
	free(z);
    }
    else {			/* argc == 3 */
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plsetoptCmd
 *
 * Processes plsetopt Tcl command.
\*--------------------------------------------------------------------------*/

static int
plsetoptCmd(ClientData clientData, Tcl_Interp *interp,
	     int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 argv[0], " option ?argument?\"", (char *) NULL);
	return TCL_ERROR;
    }

    plsetopt(argv[1], argv[2]);

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plshadeCmd
 *
 * Processes plshade Tcl command.
 * C version takes:
 *    data, nx, ny, defined,
 *    xmin, xmax, ymin, ymax,
 *    sh_min, sh_max, sh_cmap, sh_color, sh_width,
 *    min_col, min_wid, max_col, max_wid,
 *    plfill, rect, pltr, pltr_data
 *
 * We will be getting data through a 2-d Matrix, which carries along
 * nx and ny, so no need for those.  Toss defined since it's not supported
 * anyway.  Toss plfill since it is the only valid choice.  Take an optional
 * pltr spec just as for plcont, and add a wrapping specifier, also just as
 * in plcont.  So the new command looks like:
 *
 * 	plshade z xmin xmax ymin ymax \
 * 	    sh_min sh_max sh_cmap sh_color sh_width \
 * 	    min_col min_wid max_col max_wid \
 * 	    rect [pltr x y] [wrap]
\*--------------------------------------------------------------------------*/

static int
plshadeCmd( ClientData clientData, Tcl_Interp *interp,
	    int argc, char *argv[] )
{
    tclMatrix *matPtr, *matz, *mattrx = NULL, *mattry = NULL;
    PLFLT **z, **zused, **zwrapped;
    PLFLT xmin, xmax, ymin, ymax, sh_min, sh_max, sh_col;

    PLINT sh_cmap =1, sh_wid =2;
    PLINT min_col =1, min_wid =0, max_col =0, max_wid =0;
    PLINT rect =1;
    char *pltrname = "pltr0";
    void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer);
    PLPointer pltr_data = NULL;
    PLcGrid  cgrid1;
    PLcGrid2 cgrid2;
    PLINT wrap = 0;
    int nx, ny, i, j;

    if (argc < 16 ) {
	Tcl_AppendResult(interp, "bogus syntax for plshade, see doc.",
			 (char *) NULL );
	return TCL_ERROR;
    }

    matz = Tcl_GetMatrixPtr( interp, argv[1] );
    if (matz == NULL) return TCL_ERROR;
    if (matz->dim != 2) {
	interp->result = "Must plot a 2-d matrix.";
	return TCL_ERROR;
    }

    nx = matz->n[0];
    ny = matz->n[1];

    tclmateval_modx = nx;
    tclmateval_mody = ny;

    /* convert matz to 2d-array so can use standard wrap approach
     * from now on in this code. */
    plAlloc2dGrid(&z, nx, ny );
    for (i=0; i < nx; i++) {
      for (j=0; j < ny; j++) {
	 z[i][j] = tclMatrix_feval(i, j, matz);
      }
    }

    xmin = atof( argv[2] );
    xmax = atof( argv[3] );
    ymin = atof( argv[4] );
    ymax = atof( argv[5] );
    sh_min = atof( argv[6] );
    sh_max = atof( argv[7] );
    sh_cmap = atoi( argv[8] );
    sh_col = atof( argv[9] );
    sh_wid = atoi( argv[10] );
    min_col = atoi( argv[11] );
    min_wid = atoi( argv[12] );
    max_col = atoi( argv[13] );
    max_wid = atoi( argv[14] );
    rect = atoi( argv[15] );

    argc -= 16, argv += 16;

    if (argc >= 3) {
	pltrname = argv[0];
	mattrx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (mattrx == NULL) return TCL_ERROR;
	mattry = Tcl_GetMatrixPtr( interp, argv[2] );
	if (mattry == NULL) return TCL_ERROR;

	argc -= 3, argv += 3;
    }

    if (argc) {
	wrap = atoi( argv[0] );
	argc--, argv++;
    }

    if (argc) {
	interp->result = "plshade: bogus arg list";
	return TCL_ERROR;
    }

/* Figure out which coordinate transformation model is being used, and setup
   accordingly. */

    if ( !strcmp( pltrname, "pltr0" ) ) {
	pltr = pltr0;
        zused = z;

    /* wrapping is only supported for pltr2. */
	if (wrap) {
	    interp->result = "Must use pltr2 if want wrapping.";
	    return TCL_ERROR;
	}
    }
    else if ( !strcmp( pltrname, "pltr1" ) ) {
	pltr = pltr1;
	cgrid1.xg = mattrx->fdata;
	cgrid1.nx = nx;
	cgrid1.yg = mattry->fdata;
	cgrid1.ny = ny;
        zused = z;

    /* wrapping is only supported for pltr2. */
	if (wrap) {
	    interp->result = "Must use pltr2 if want wrapping.";
	    return TCL_ERROR;
	}

	if (mattrx->dim != 1 || mattry->dim != 1) {
	    interp->result = "Must use 1-d coord arrays with pltr1.";
	    return TCL_ERROR;
	}

	pltr_data = &cgrid1;
    }
    else if ( !strcmp( pltrname, "pltr2" ) ) {
    /* printf( "plshade, setting up for pltr2\n" ); */
	if (!wrap) {
	/* printf( "plshade, no wrapping is needed.\n" ); */
	    plAlloc2dGrid( &cgrid2.xg, nx, ny );
	    plAlloc2dGrid( &cgrid2.yg, nx, ny );
	    cgrid2.nx = nx;
	    cgrid2.ny = ny;
	    zused = z;

	    matPtr = mattrx;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

	    matPtr = mattry;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
	}
	else if (wrap == 1) {
	    plAlloc2dGrid( &cgrid2.xg, nx+1, ny );
	    plAlloc2dGrid( &cgrid2.yg, nx+1, ny );
            plAlloc2dGrid( &zwrapped, nx+1, ny );
	    cgrid2.nx = nx+1;
	    cgrid2.ny = ny;
	    zused = zwrapped;

	    matPtr = mattrx;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

	    matPtr = mattry;
	    for( i=0; i < nx; i++ ) {
		for( j=0; j < ny; j++ ) {
		    cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
		    zwrapped[i][j] = z[i][j];
		}
	    }

	    for( j=0; j < ny; j++ ) {
		cgrid2.xg[nx][j] = cgrid2.xg[0][j];
		cgrid2.yg[nx][j] = cgrid2.yg[0][j];
		zwrapped[nx][j] = zwrapped[0][j];
	    }

            /* z not used in executable path after this so free it before
	     * nx value is changed. */
	    plFree2dGrid( z, nx, ny );

	    nx++;
	}
	else if (wrap == 2) {
	    plAlloc2dGrid( &cgrid2.xg, nx, ny+1 );
	    plAlloc2dGrid( &cgrid2.yg, nx, ny+1 );
	    plAlloc2dGrid( &zwrapped, nx, ny+1 );
	    cgrid2.nx = nx;
	    cgrid2.ny = ny+1;
	    zused = zwrapped;

	    matPtr = mattrx;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

	    matPtr = mattry;
	    for( i=0; i < nx; i++ ) {
		for( j=0; j < ny; j++ ) {
		    cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
		    zwrapped[i][j] = z[i][j];
		}
	    }

	    for( i=0; i < nx; i++ ) {
		cgrid2.xg[i][ny] = cgrid2.xg[i][0];
		cgrid2.yg[i][ny] = cgrid2.yg[i][0];
		zwrapped[i][ny] = zwrapped[i][0];
	    }

            /* z not used in executable path after this so free it before
	     * ny value is changed. */
	    plFree2dGrid( z, nx, ny );

	    ny++;
	}
	else {
	    interp->result =
		"Invalid wrap specifier, must be <empty>, 0, 1, or 2.";
	    return TCL_ERROR;
	}

	pltr = pltr2;
	pltr_data = &cgrid2;
    }
    else {
	Tcl_AppendResult( interp,
			  "Unrecognized coordinate transformation spec:",
			  pltrname, ", must be pltr0 pltr1 or pltr2.",
			  (char *) NULL );
	return TCL_ERROR;
    }

/* Now go make the plot. */

    plshade( zused, nx, ny, NULL,
	      xmin, xmax, ymin, ymax,
	      sh_min, sh_max, sh_cmap, sh_col, sh_wid,
	      min_col, min_wid, max_col, max_wid,
	      plfill, rect, pltr, pltr_data );

/* Now free up any space which got allocated for our coordinate trickery. */

/* zused points to either z or zwrapped.  In both cases the allocated size
 * was nx by ny.  Now free the allocated space, and note in the case
 * where zused points to zwrapped, the separate z space has been freed by
 * previous wrap logic. */
    plFree2dGrid( zused, nx, ny );

    if (pltr == pltr1) {
    /* Hmm, actually, nothing to do here currently, since we just used the
       Tcl Matrix data directly, rather than allocating private space. */
    }
    else if (pltr == pltr2) {
    /* printf( "plshade, freeing space for grids used in pltr2\n" ); */
	plFree2dGrid( cgrid2.xg, nx, ny );
	plFree2dGrid( cgrid2.yg, nx, ny );
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plshadesCmd
 *
 * Processes plshades Tcl command.
 * C version takes:
 *    data, nx, ny, defined,
 *    xmin, xmax, ymin, ymax,
 *    clevel, nlevel, fill_width, cont_color, cont_width,
 *    plfill, rect, pltr, pltr_data
 *
 * We will be getting data through a 2-d Matrix, which carries along
 * nx and ny, so no need for those.  Toss defined since it's not supported
 * anyway.  clevel will be via a 1-d matrix, which carries along nlevel, so
 * no need for that.  Toss plfill since it is the only valid choice.
 * Take an optional
 * pltr spec just as for plcont, and add a wrapping specifier, also just as
 * in plcont.  So the new command looks like:
 *
 * 	plshades z xmin xmax ymin ymax \
 * 	    clevel, fill_width, cont_color, cont_width\
 * 	    rect [pltr x y] [wrap]
\*--------------------------------------------------------------------------*/

static int
plshadesCmd( ClientData clientData, Tcl_Interp *interp,
	    int argc, char *argv[] )
{
    tclMatrix *matPtr, *matz, *mattrx = NULL, *mattry = NULL;
    tclMatrix *matclevel = NULL;
    PLFLT **z, **zused, **zwrapped;
    PLFLT xmin, xmax, ymin, ymax;
    PLINT fill_width = 0, cont_color = 0, cont_width =0;
    PLINT rect =1;
    char *pltrname = "pltr0";
    void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer);
    PLPointer pltr_data = NULL;
    PLcGrid  cgrid1;
    PLcGrid2 cgrid2;
    PLINT wrap = 0;
    int nx, ny, nlevel, i, j;

    if (argc < 11 ) {
	Tcl_AppendResult(interp, "bogus syntax for plshades, see doc.",
			 (char *) NULL );
	return TCL_ERROR;
    }

    matz = Tcl_GetMatrixPtr( interp, argv[1] );
    if (matz == NULL) return TCL_ERROR;
    if (matz->dim != 2) {
	interp->result = "Must plot a 2-d matrix.";
	return TCL_ERROR;
    }

    nx = matz->n[0];
    ny = matz->n[1];

    tclmateval_modx = nx;
    tclmateval_mody = ny;

    /* convert matz to 2d-array so can use standard wrap approach
     * from now on in this code. */
    plAlloc2dGrid(&z, nx, ny );
    for (i=0; i < nx; i++) {
      for (j=0; j < ny; j++) {
	 z[i][j] = tclMatrix_feval(i, j, matz);
      }
    }

    xmin = atof( argv[2] );
    xmax = atof( argv[3] );
    ymin = atof( argv[4] );
    ymax = atof( argv[5] );

    matclevel = Tcl_GetMatrixPtr( interp, argv[6] );
    if (matclevel == NULL) return TCL_ERROR;
    nlevel = matclevel->n[0];
    if (matclevel->dim != 1) {
       interp->result = "clevel must be 1-d matrix.";
       return TCL_ERROR;
    }

    fill_width = atoi( argv[7] );
    cont_color = atoi( argv[8] );
    cont_width = atoi( argv[9] );
    rect = atoi( argv[10] );

    argc -= 11, argv += 11;

    if (argc >= 3) {
	pltrname = argv[0];
	mattrx = Tcl_GetMatrixPtr( interp, argv[1] );
	if (mattrx == NULL) return TCL_ERROR;
	mattry = Tcl_GetMatrixPtr( interp, argv[2] );
	if (mattry == NULL) return TCL_ERROR;

	argc -= 3, argv += 3;
    }

    if (argc) {
	wrap = atoi( argv[0] );
	argc--, argv++;
    }

    if (argc) {
	interp->result = "plshades: bogus arg list";
	return TCL_ERROR;
    }

/* Figure out which coordinate transformation model is being used, and setup
   accordingly. */

    if ( !strcmp( pltrname, "pltr0" ) ) {
	pltr = pltr0;
        zused = z;

    /* wrapping is only supported for pltr2. */
	if (wrap) {
	    interp->result = "Must use pltr2 if want wrapping.";
	    return TCL_ERROR;
	}
    }
    else if ( !strcmp( pltrname, "pltr1" ) ) {
	pltr = pltr1;
	cgrid1.xg = mattrx->fdata;
	cgrid1.nx = nx;
	cgrid1.yg = mattry->fdata;
	cgrid1.ny = ny;
        zused = z;

    /* wrapping is only supported for pltr2. */
	if (wrap) {
	    interp->result = "Must use pltr2 if want wrapping.";
	    return TCL_ERROR;
	}

	if (mattrx->dim != 1 || mattry->dim != 1) {
	    interp->result = "Must use 1-d coord arrays with pltr1.";
	    return TCL_ERROR;
	}

	pltr_data = &cgrid1;
    }
    else if ( !strcmp( pltrname, "pltr2" ) ) {
    /* printf( "plshades, setting up for pltr2\n" ); */
	if (!wrap) {
	/* printf( "plshades, no wrapping is needed.\n" ); */
	    plAlloc2dGrid( &cgrid2.xg, nx, ny );
	    plAlloc2dGrid( &cgrid2.yg, nx, ny );
	    cgrid2.nx = nx;
	    cgrid2.ny = ny;
	    zused = z;

	    matPtr = mattrx;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

	    matPtr = mattry;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
	}
	else if (wrap == 1) {
	    plAlloc2dGrid( &cgrid2.xg, nx+1, ny );
	    plAlloc2dGrid( &cgrid2.yg, nx+1, ny );
            plAlloc2dGrid( &zwrapped, nx+1, ny );
	    cgrid2.nx = nx+1;
	    cgrid2.ny = ny;
	    zused = zwrapped;

	    matPtr = mattrx;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

	    matPtr = mattry;
	    for( i=0; i < nx; i++ ) {
		for( j=0; j < ny; j++ ) {
		    cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
		    zwrapped[i][j] = z[i][j];
		}
	    }

	    for( j=0; j < ny; j++ ) {
		cgrid2.xg[nx][j] = cgrid2.xg[0][j];
		cgrid2.yg[nx][j] = cgrid2.yg[0][j];
		zwrapped[nx][j] = zwrapped[0][j];
	    }

            /* z not used in executable path after this so free it before
	     * nx value is changed. */
	    plFree2dGrid( z, nx, ny );

	    nx++;
	}
	else if (wrap == 2) {
	    plAlloc2dGrid( &cgrid2.xg, nx, ny+1 );
	    plAlloc2dGrid( &cgrid2.yg, nx, ny+1 );
	    plAlloc2dGrid( &zwrapped, nx, ny+1 );
	    cgrid2.nx = nx;
	    cgrid2.ny = ny+1;
	    zused = zwrapped;

	    matPtr = mattrx;
	    for( i=0; i < nx; i++ )
		for( j=0; j < ny; j++ )
		    cgrid2.xg[i][j] = mattrx->fdata[ I2D(i,j) ];

	    matPtr = mattry;
	    for( i=0; i < nx; i++ ) {
		for( j=0; j < ny; j++ ) {
		    cgrid2.yg[i][j] = mattry->fdata[ I2D(i,j) ];
		    zwrapped[i][j] = z[i][j];
		}
	    }

	    for( i=0; i < nx; i++ ) {
		cgrid2.xg[i][ny] = cgrid2.xg[i][0];
		cgrid2.yg[i][ny] = cgrid2.yg[i][0];
		zwrapped[i][ny] = zwrapped[i][0];
	    }

            /* z not used in executable path after this so free it before
	     * ny value is changed. */
	    plFree2dGrid( z, nx, ny );

	    ny++;
	}
	else {
	    interp->result =
		"Invalid wrap specifier, must be <empty>, 0, 1, or 2.";
	    return TCL_ERROR;
	}

	pltr = pltr2;
	pltr_data = &cgrid2;
    }
    else {
	Tcl_AppendResult( interp,
			  "Unrecognized coordinate transformation spec:",
			  pltrname, ", must be pltr0 pltr1 or pltr2.",
			  (char *) NULL );
	return TCL_ERROR;
    }

/* Now go make the plot. */

    plshades( zused, nx, ny, NULL,
	      xmin, xmax, ymin, ymax,
	      matclevel->fdata, nlevel, fill_width, cont_color, cont_width,
	      plfill, rect, pltr, pltr_data );

/* Now free up any space which got allocated for our coordinate trickery. */

/* zused points to either z or zwrapped.  In both cases the allocated size
 * was nx by ny.  Now free the allocated space, and note in the case
 * where zused points to zwrapped, the separate z space has been freed by
 * previous wrap logic. */
    plFree2dGrid( zused, nx, ny );

    if (pltr == pltr1) {
    /* Hmm, actually, nothing to do here currently, since we just used the
       Tcl Matrix data directly, rather than allocating private space. */
    }
    else if (pltr == pltr2) {
    /* printf( "plshades, freeing space for grids used in pltr2\n" ); */
	plFree2dGrid( cgrid2.xg, nx, ny );
	plFree2dGrid( cgrid2.yg, nx, ny );
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * mapform
 *
 * Defines our coordinate transformation.
 * x[], y[] are the coordinates to be plotted.
\*--------------------------------------------------------------------------*/

void
mapform(PLINT n, PLFLT *x, PLFLT *y)
{
    int i;
    double xp, yp, radius;
    for (i = 0; i < n; i++) {
	radius = 90.0 - y[i];
	xp = radius * cos(x[i] * PI / 180.0);
	yp = radius * sin(x[i] * PI / 180.0);
	x[i] = xp;
	y[i] = yp;
    }
}

/*--------------------------------------------------------------------------*\
 * plmapCmd
 *
 * Processes plmap Tcl command.
 * C version takes:
 *    string, minlong, maxlong, minlat, maxlat
 *
 *  e.g. .p cmd plmap globe 0 360 -90 90
\*--------------------------------------------------------------------------*/

static int
plmapCmd( ClientData clientData, Tcl_Interp *interp,
	    int argc, char *argv[] )
{
    PLFLT minlong, maxlong, minlat, maxlat;
    PLINT transform;

    if (argc < 7 ) {
	Tcl_AppendResult(interp, "bogus syntax for plmap, see doc.",
			 (char *) NULL );
	return TCL_ERROR;
    }

    transform = atoi(argv[2]);
    minlong = atof( argv[3] );
    maxlong = atof( argv[4] );
    minlat = atof( argv[5] );
    maxlat = atof( argv[6] );

    if (transform) {
	plmap(&mapform, argv[1], minlong, maxlong, minlat, maxlat);
    } else {
	plmap(NULL, argv[1], minlong, maxlong, minlat, maxlat);
    }

    plflush();
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plmeridiansCmd
 *
 * Processes plmeridians Tcl command.
 * C version takes:
 *    dlong, dlat, minlong, maxlong, minlat, maxlat
 *
 *  e.g. .p cmd plmeridians 1 ...
\*--------------------------------------------------------------------------*/

static int
plmeridiansCmd( ClientData clientData, Tcl_Interp *interp,
	    int argc, char *argv[] )
{
    PLFLT dlong, dlat, minlong, maxlong, minlat, maxlat;
    PLINT transform;

    if (argc < 8 ) {
	Tcl_AppendResult(interp, "bogus syntax for plmap, see doc.",
			 (char *) NULL );
	return TCL_ERROR;
    }

    transform = atoi(argv[1]);
    dlong = atof( argv[2] );
    dlat = atof( argv[3] );
    minlong = atof( argv[4] );
    maxlong = atof( argv[5] );
    minlat = atof( argv[6] );
    maxlat = atof( argv[7] );

    if (transform) {
	plmeridians(&mapform, dlong, dlat, minlong, maxlong, minlat, maxlat);
    } else {
	plmeridians(NULL, dlong, dlat, minlong, maxlong, minlat, maxlat);
    }

    plflush();
    return TCL_OK;
}
