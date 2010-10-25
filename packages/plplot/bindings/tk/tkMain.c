/* $Id: tkMain.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 * Modified version of tkMain.c, from Tk 3.6.
 * Maurice LeBrun
 * 23-Jun-1994
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
 * Modifications include:
 * 1. main() changed to pltkMain().
 * 2. tcl_RcFileName -> RcFileName, now passed in through the argument list.
 * 3. Tcl_AppInit -> AppInit, now passed in through the argument list.
 * 4. Support for -e <script> startup option
 *
 * The original notes follow.
 */

/*
 * main.c --
 *
 *	This file contains the main program for "wish", a windowing
 *	shell based on Tk and Tcl.  It also provides a template that
 *	can be used as the basis for main programs for other Tk
 *	applications.
 *
 * Copyright (c) 1990-1993 The Regents of the University of California.
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
 */

#include "plplotP.h"

#include <stdio.h>
#include <stdlib.h>
#include <tcl.h>
#include <tk.h>
#ifdef HAVE_ITCL
# ifndef HAVE_ITCLDECLS_H
#  define RESOURCE_INCLUDED
# endif
# include <itcl.h>
#endif

/* itk.h includes itclInt.h which includes tclInt.h ...disaster -mjl */
/* #ifdef HAVE_ITK */
/* #include <itk.h> */
/* #endif */

/* From itkDecls.h */

EXTERN int		Itk_Init _ANSI_ARGS_((Tcl_Interp * interp));

/* From tclIntDecls.h */

EXTERN int		Tcl_Import _ANSI_ARGS_((Tcl_Interp * interp,
				Tcl_Namespace * nsPtr, char * pattern,
				int allowOverwrite));

EXTERN Tcl_Namespace *	Tcl_GetGlobalNamespace _ANSI_ARGS_((
				Tcl_Interp * interp));

/*
 * Declarations for various library procedures and variables (don't want
 * to include tkInt.h or tkConfig.h here, because people might copy this
 * file out of the Tk source directory to make their own modified versions).
 */

/* these are defined in unistd.h, included by plplotP.h
extern void		exit _ANSI_ARGS_((int status));
extern int		isatty _ANSI_ARGS_((int fd));
extern int		read _ANSI_ARGS_((int fd, char *buf, size_t size));
*/
extern char *		strrchr _ANSI_ARGS_((CONST char *string, int c));

/*
 * Global variables used by the main program:
 */

static Tcl_Interp *interp;	/* Interpreter for this application. */
static Tcl_DString command;	/* Used to assemble lines of terminal input
				 * into Tcl commands. */
static int tty;			/* Non-zero means standard input is a
				 * terminal-like device.  Zero means it's
				 * a file. */
static char errorExitCmd[] = "exit 1";

/*
 * Command-line options:
 */

static int synchronize = 0;
static char *script = NULL;
static char *fileName = NULL;
static char *name = NULL;
static char *display = NULL;
static char *geometry = NULL;

static Tk_ArgvInfo argTable[] = {
    {"-file", TK_ARGV_STRING, (char *) NULL, (char *) &fileName,
	"File from which to read commands"},
    {"-e", TK_ARGV_STRING, (char *) NULL, (char *) &script,
	"Script to execute on startup"},
    {"-geometry", TK_ARGV_STRING, (char *) NULL, (char *) &geometry,
	"Initial geometry for window"},
    {"-display", TK_ARGV_STRING, (char *) NULL, (char *) &display,
	"Display to use"},
    {"-name", TK_ARGV_STRING, (char *) NULL, (char *) &name,
	"Name to use for application"},
    {"-sync", TK_ARGV_CONSTANT, (char *) 1, (char *) &synchronize,
	"Use synchronous mode for display server"},
    {(char *) NULL, TK_ARGV_END, (char *) NULL, (char *) NULL,
	(char *) NULL}
};

/*
 * Forward declarations for procedures defined later in this file:
 */

static void		Prompt _ANSI_ARGS_((Tcl_Interp *interp, int partial));
static void		StdinProc _ANSI_ARGS_((ClientData clientData,
			    int mask));

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	Main program for Wish.
 *
 * Results:
 *	None. This procedure never returns (it exits the process when
 *	it's done
 *
 * Side effects:
 *	This procedure initializes the wish world and then starts
 *	interpreting commands;  almost anything could happen, depending
 *	on the script being interpreted.
 *
 *----------------------------------------------------------------------
 */

int
pltkMain(int argc, char **argv, char *RcFileName,
	 int (*AppInit)(Tcl_Interp *interp))
{
    char *args, *p, *msg;
    char buf[20];
    int code;

    Tcl_FindExecutable(argv[0]);
    interp = Tcl_CreateInterp();
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
#endif

    /*
     * Parse command-line arguments.
     */

    if (Tk_ParseArgv(interp, (Tk_Window) NULL, &argc, (CONST char**) argv, argTable, 0)
	    != TCL_OK) {
	fprintf(stderr, "%s\n", interp->result);
	exit(1);
    }
    if (name == NULL) {
	if (fileName != NULL) {
	    p = fileName;
	} else {
	    p = argv[0];
	}
	name = strrchr(p, '/');
	if (name != NULL) {
	    name++;
	} else {
	    name = p;
	}
    }

    /*
     * If a display was specified, put it into the DISPLAY
     * environment variable so that it will be available for
     * any sub-processes created by us.
     */

    if (display != NULL) {
	Tcl_SetVar2(interp, "env", "DISPLAY", display, TCL_GLOBAL_ONLY);
    }

    /*
     * Initialize the Tk application.
     */

    /*
     * This must be setup *before* calling Tk_Init,
     * and `name' has already been setup above
     */

    Tcl_SetVar(interp, "argv0", name, TCL_GLOBAL_ONLY);

    if ( Tcl_Init( interp ) == TCL_ERROR ) {
	return TCL_ERROR;
    }
    if ( Tk_Init( interp ) == TCL_ERROR ) {
	return TCL_ERROR;
    }
#ifdef HAVE_ITCL
    if ( Itcl_Init( interp ) == TCL_ERROR ) {
	return TCL_ERROR;
    }
#endif
#ifdef HAVE_ITK
    if ( Itk_Init( interp ) == TCL_ERROR ) {
	return TCL_ERROR;
    }

/*
  Pulled in this next section from itkwish in itcl3.0.1.
*/

    /*
     *  This is itkwish, so import all [incr Tcl] commands by
     *  default into the global namespace.  Fix up the autoloader
     *  to do the same.
     */
    if (Tcl_Import(interp, Tcl_GetGlobalNamespace(interp),
            "::itk::*", /* allowOverwrite */ 1) != TCL_OK) {
        return TCL_ERROR;
    }

    if (Tcl_Import(interp, Tcl_GetGlobalNamespace(interp),
            "::itcl::*", /* allowOverwrite */ 1) != TCL_OK) {
        return TCL_ERROR;
    }

    if (Tcl_Eval(interp, "auto_mkindex_parser::slavehook { _%@namespace import -force ::itcl::* ::itk::* }") != TCL_OK) {
        return TCL_ERROR;
    }
#endif

    /*
     * Make command-line arguments available in the Tcl variables "argc"
     * and "argv".  Also set the "geometry" variable from the geometry
     * specified on the command line.
     */

    args = Tcl_Merge(argc-1, (CONST char * CONST *) argv+1);
    Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
    ckfree(args);
    sprintf(buf, "%d", argc-1);
    Tcl_SetVar(interp, "argc", buf, TCL_GLOBAL_ONLY);

    if (geometry != NULL) {
	Tcl_SetVar(interp, "geometry", geometry, TCL_GLOBAL_ONLY);
    }

    /*
     * Set the "tcl_interactive" variable.
     */

    tty = isatty(0);
    Tcl_SetVar(interp, "tcl_interactive",
	    ((fileName == NULL) && tty) ? "1" : "0", TCL_GLOBAL_ONLY);

    /*
     * Add a few application-specific commands to the application's
     * interpreter.
     */

    /*
     * Invoke application-specific initialization.
     */

    if ((*AppInit)(interp) != TCL_OK) {
	fprintf(stderr, "(*AppInit) failed: %s\n", interp->result);
    }

    /*
     * Set the geometry of the main window, if requested.
     */

    if (geometry != NULL) {
	code = Tcl_VarEval(interp, "wm geometry . ", geometry, (char *) NULL);
	if (code != TCL_OK) {
	    fprintf(stderr, "%s\n", interp->result);
	}
    }

    /*
     * Process the startup script, if any.
     */

    if (script != NULL) {
	code = Tcl_VarEval(interp, script, (char *) NULL);
	if (code != TCL_OK) {
	    goto error;
	}
	tty = 0;
    }

    /*
     * Invoke the script specified on the command line, if any.
     */

    if (fileName != NULL) {
	code = Tcl_VarEval(interp, "source ", fileName, (char *) NULL);
	if (code != TCL_OK) {
	    goto error;
	}
	tty = 0;
    } else {
	/*
	 * Commands will come from standard input, so set up an event
	 * handler for standard input.  If the input device is aEvaluate the
	 * .rc file, if one has been specified, set up an event handler
	 * for standard input, and print a prompt if the input
	 * device is a terminal.
	 */

	if (RcFileName != NULL) {
	    Tcl_DString buffer;
	    char *fullName;
	    FILE *f;

	    fullName = Tcl_TildeSubst(interp, RcFileName, &buffer);
	    if (fullName == NULL) {
		fprintf(stderr, "%s\n", interp->result);
	    } else {
		f = fopen(fullName, "r");
		if (f != NULL) {
		    code = Tcl_EvalFile(interp, fullName);
		    if (code != TCL_OK) {
			fprintf(stderr, "%s\n", interp->result);
		    }
		    fclose(f);
		}
	    }
	    Tcl_DStringFree(&buffer);
	}
	Tk_CreateFileHandler(0, TK_READABLE, StdinProc, (ClientData) 0);
	if (tty) {
	    Prompt(interp, 0);
	}
    }
    fflush(stdout);
    Tcl_DStringInit(&command);

    /*
     * Loop infinitely, waiting for commands to execute.  When there
     * are no windows left, Tk_MainLoop returns and we exit.
     */

    Tk_MainLoop();

    /*
     * Don't exit directly, but rather invoke the Tcl "exit" command.
     * This gives the application the opportunity to redefine "exit"
     * to do additional cleanup.
     */

    Tcl_Eval(interp, "exit");
    exit(1);

error:
    msg = (char *) Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
    if (msg == NULL) {
	msg = interp->result;
    }
    fprintf(stderr, "%s\n", msg);
    Tcl_Eval(interp, errorExitCmd);
    return 1;			/* Needed only to prevent compiler warnings. */
}

/*
 *----------------------------------------------------------------------
 *
 * StdinProc --
 *
 *	This procedure is invoked by the event dispatcher whenever
 *	standard input becomes readable.  It grabs the next line of
 *	input characters, adds them to a command being assembled, and
 *	executes the command if it's complete.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Could be almost arbitrary, depending on the command that's
 *	typed.
 *
 *----------------------------------------------------------------------
 */

    /* ARGSUSED */
static void
StdinProc(clientData, mask)
    ClientData clientData;		/* Not used. */
    int mask;				/* Not used. */
{
#define BUFFER_SIZE 4000
    char input[BUFFER_SIZE+1];
    static int gotPartial = 0;
    char *cmd;
    int code, count;

    count = read(fileno(stdin), input, BUFFER_SIZE);
    if (count <= 0) {
	if (!gotPartial) {
	    if (tty) {
		Tcl_Eval(interp, "exit");
		exit(1);
	    } else {
		Tk_DeleteFileHandler(0);
	    }
	    return;
	} else {
	    count = 0;
	}
    }
    cmd = Tcl_DStringAppend(&command, input, count);
    if (count != 0) {
	if ((input[count-1] != '\n') && (input[count-1] != ';')) {
	    gotPartial = 1;
	    goto prompt;
	}
	if (!Tcl_CommandComplete(cmd)) {
	    gotPartial = 1;
	    goto prompt;
	}
    }
    gotPartial = 0;

    /*
     * Disable the stdin file handler while evaluating the command;
     * otherwise if the command re-enters the event loop we might
     * process commands from stdin before the current command is
     * finished.  Among other things, this will trash the text of the
     * command being evaluated.
     */

    Tk_CreateFileHandler(0, 0, StdinProc, (ClientData) 0);
    code = Tcl_RecordAndEval(interp, cmd, 0);
    Tk_CreateFileHandler(0, TK_READABLE, StdinProc, (ClientData) 0);
    Tcl_DStringFree(&command);
    if (*interp->result != 0) {
	if ((code != TCL_OK) || (tty)) {
	    printf("%s\n", interp->result);
	}
    }

    /*
     * Output a prompt.
     */

    prompt:
    if (tty) {
	Prompt(interp, gotPartial);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Prompt --
 *
 *	Issue a prompt on standard output, or invoke a script
 *	to issue the prompt.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A prompt gets output, and a Tcl script may be evaluated
 *	in interp.
 *
 *----------------------------------------------------------------------
 */

static void
Prompt(interp, partial)
    Tcl_Interp *interp;			/* Interpreter to use for prompting. */
    int partial;			/* Non-zero means there already
					 * exists a partial command, so use
					 * the secondary prompt. */
{
    char *promptCmd;
    int code;

    promptCmd = (char *) Tcl_GetVar(interp,
	partial ? "tcl_prompt2" : "tcl_prompt1", TCL_GLOBAL_ONLY);
    if (promptCmd == NULL) {
	defaultPrompt:
	if (!partial) {
	    fputs("% ", stdout);
	}
    } else {
	code = Tcl_Eval(interp, promptCmd);
	if (code != TCL_OK) {
	    Tcl_AddErrorInfo(interp,
		    "\n    (script that generates prompt)");
	    fprintf(stderr, "%s\n", interp->result);
	    goto defaultPrompt;
	}
    }
    fflush(stdout);
}
