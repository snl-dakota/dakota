/* $Id: plserver.c 3186 2006-02-15 18:17:33Z slbrow $

    Copyright 1993, 1994, 1995
    Maurice LeBrun			mjl@dino.ph.utexas.edu
    Institute for Fusion Studies	University of Texas at Austin

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

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    PLplot graphics server.

    Just a front-end to the pltkMain() function.  Structured along the
    preferred lines for extended wish'es.  Is typically run as a child
    process from the PLplot TK driver to render output.  Can use either TK
    send or Tcl-DP RPC for communication, depending on how it is invoked.

    Note that plserver can be used the same way as wish or dpwish, as it
    contains the functionality of each of these (except the -notk Tcl-DP
    command-line option is not supported).
*/

#define NEED_PLDEBUG
#include "plserver.h"

/* Application-specific command-line options */
/* Variable declarations */

static char *client_name;	/* Name of client main window */
static char *auto_path;		/* addition to auto_path */
static int child;		/* set if child of TK driver */
#ifdef PLD_dp
static int dp;			/* set if using Tcl-DP to communicate */
#endif
static char *client_host;	/* Host id for client */
static char *client_port;	/* Communications port id for client */

static Tk_ArgvInfo argTable[] = {
    {"-client_name", TK_ARGV_STRING, (char *) NULL, (char *) &client_name,
	 "Client main window name to connect to"},
    {"-client_host", TK_ARGV_STRING, (char *) NULL, (char *) &client_host,
	 "Client host to connect to"},
    {"-client_port", TK_ARGV_STRING, (char *) NULL, (char *) &client_port,
	 "Client port (Tcl-DP) to connect to"},
    {"-auto_path", TK_ARGV_STRING, (char *) NULL, (char *) &auto_path,
	 "Additional directory(s) to autoload"},
    {"-child", TK_ARGV_CONSTANT, (char *) 1, (char *) &child,
	 "Set ONLY when child of PLplot TK driver"},
    {(char *) NULL, TK_ARGV_END, (char *) NULL, (char *) NULL,
	 (char *) NULL}
};

/* PLplot/Tk extension command -- handle exit. */

static int
plExitCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);

/* Evals the specified command, aborting on an error. */

static void
tcl_cmd(Tcl_Interp *interp, char *cmd);

/* Application-specific startup */

static int
AppInit(Tcl_Interp *interp);

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
    int i, myargc = argc;
    char *myargv[20];
    Tcl_Interp *interp;
    char *helpmsg = "Command-specific options:";

#ifdef DEBUG
    fprintf(stderr, "Program %s called with arguments :\n", argv[0]);
    for (i = 1; i < argc; i++) {
	fprintf(stderr, "%s ", argv[i]);
    }
    fprintf(stderr, "\n");
#endif

/* Create interpreter just for argument parsing */

    interp = Tcl_CreateInterp();

/* Save arglist to get around tk_ParseArgv limitations */

    for (i = 0; i < argc; i++) {
	myargv[i] = argv[i];
    }

/* Parse args */
/* Examine the result string to see if an error return is really an error */

    if (Tk_ParseArgv(interp, (Tk_Window) NULL, &argc, (CONST char**) argv,
		     argTable, TK_ARGV_NO_DEFAULTS) != TCL_OK) {
	fprintf(stderr, "\n(plserver) %s\n\n", interp->result);
	fprintf(stderr, "\
The client_<xxx> and -child options should not be used except via the\n\
PLplot/Tk driver.\n\n(wish) ");
	if (strncmp(interp->result, helpmsg, strlen(helpmsg)))
	    exit(1);
    }

/* No longer need interpreter */

#if TCL_MAJOR_VERSION < 7 || ( TCL_MAJOR_VERSION == 7 && TCL_MINOR_VERSION < 5 )
    Tcl_DeleteInterp(interp);
#endif

/* Call pltkMain() with original argc/argv list, to make sure -h is seen */
/* Does not return until program exit */

    exit(pltkMain(myargc, myargv, NULL, AppInit));
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

/* Application-specific startup.  That means: for use in plserver ONLY. */

/* Pass child variable to interpreter if set. */

    if (child != 0)
	Tcl_SetVar(interp, "child", "1", 0);

/* If client_name is set, TK send is being used to communicate. */
/* If client_port is set, Tcl-DP RPC is being used to communicate. */
/* The "dp" variable determines which style communication is used */

    if (client_name != NULL) {
	Tcl_SetVar(interp, "client_name", client_name, 0);
	tcl_cmd(interp, "set dp 0");
#ifdef PLD_dp
	dp = 0;
#endif
    }
    else if (client_port != NULL) {
#ifdef PLD_dp
	Tcl_SetVar(interp, "client_port", client_port, 0);
	if (client_host != NULL)
	    Tcl_SetVar(interp, "client_host", client_host, 0);
	dp = 1; tcl_cmd(interp, "set dp 1");
#else
	Tcl_AppendResult(interp,
			 "no Tcl-DP support in this version of plserver",
			 (char *) NULL);
	return TCL_ERROR;
#endif
    }

/* Add user-specified directory(s) to auto_path */

    if (auto_path != NULL) {
	Tcl_SetVar(interp, "dir", auto_path, 0);
	tcl_cmd(interp, "set auto_path \"$dir $auto_path\"");
    }

/* Rename "exit" to "tkexit", and insert custom exit handler */

    tcl_cmd(interp, "rename exit tkexit");

    Tcl_CreateCommand(interp, "exit", (Tcl_CmdProc *) plExitCmd,
                      (ClientData) mainWindow, (Tcl_CmdDeleteProc*) NULL);

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * plExitCmd
 *
 * PLplot/Tk extension command -- handle exit.
 * The reason for overriding the normal exit command is so we can tell the
 * client to abort.
\*--------------------------------------------------------------------------*/

static int
plExitCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{
    int value = 0;

/* Print error message if one given */

    if (interp->result != NULL && interp->result[0] != '\0')
	fprintf(stderr, "%s\n", interp->result);

/* Best to check the syntax before proceeding */

    if ((argc != 1) && (argc != 2)) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
			 " ?returnCode?\"", (char *) NULL);
	return TCL_ERROR;
    }
    if ((argc != 1) && (Tcl_GetInt(interp, argv[1], &value) != TCL_OK)) {
	Tcl_AppendResult(interp, "non-integer return code: \"", argv[1],
			 "\"", (char *) NULL);
	return TCL_ERROR;
    }

/* If client exists, tell it to self destruct */

    Tcl_VarEval(interp, "plserver_link_end", (char **) NULL);

/* Now really exit */

    return Tcl_VarEval(interp, "tkexit", argv[1], (char **) NULL);
}

/*--------------------------------------------------------------------------*\
* tcl_cmd
*
* Evals the specified command, aborting on an error.
\*--------------------------------------------------------------------------*/

static void
tcl_cmd(Tcl_Interp *interp, char *cmd)
{
    int result;

    dbug_enter("tcl_cmd");
    pldebug("tcl_cmd", "evaluating command %s\n", cmd);

    result = Tcl_VarEval(interp, cmd, (char **) NULL);
    if (result != TCL_OK) {
	Tcl_Eval(interp, "exit");
	exit(1);
    }
}
