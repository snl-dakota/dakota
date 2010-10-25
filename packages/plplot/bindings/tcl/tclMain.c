/* $Id: tclMain.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 * Modified version of tclMain.c, from Tcl 8.3.2.
 * Maurice LeBrun
 * Jan 2 2001
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
 * Based on previous version of tclMain.c, from Tcl 7.3.
 * Modifications include:
 * 1. Tcl_Main() changed to pltclMain().
 * 2. Changes to work with ANSI C
 * 3. Changes to support user-installable error or output handlers.
 * 4. PLplot argument parsing routine called to handle arguments.
 * 5. Added define of _POSIX_SOURCE and eliminated include of tclInt.h.
 *
 * Original comments follow.
 */

/*
 * tclMain.c --
 *
 *	Main program for Tcl shells and other Tcl-based applications.
 *
 * Copyright (c) 1988-1994 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: tclMain.c 3186 2006-02-15 18:17:33Z slbrow $
 */

#include <tcl.h>
#include "plplot.h"

/* From tclIntDecls.h */

EXTERN int		TclFormatInt _ANSI_ARGS_((char * buffer, long n));
EXTERN int		TclObjCommandComplete _ANSI_ARGS_((Tcl_Obj * cmdPtr));

# undef TCL_STORAGE_CLASS
# define TCL_STORAGE_CLASS DLLEXPORT

/*
 * The following code ensures that tclLink.c is linked whenever
 * Tcl is linked.  Without this code there's no reference to the
 * code in that file from anywhere in Tcl, so it may not be
 * linked into the application.
 */

EXTERN int Tcl_LinkVar();
int (*tclDummyLinkVarPtr)() = Tcl_LinkVar;

/*
 * Declarations for various library procedures and variables (don't want
 * to include tclPort.h here, because people might copy this file out of
 * the Tcl source directory to make their own modified versions).
 * Note:  "exit" should really be declared here, but there's no way to
 * declare it without causing conflicts with other definitions elsewher
 * on some systems, so it's better just to leave it out.
 */

extern int		isatty _ANSI_ARGS_((int fd));
extern char *		strcpy _ANSI_ARGS_((char *dst, CONST char *src));

static char *tclStartupScriptFileName = NULL;

/* pltcl enhancements */

static void
plPrepOutputHandler(Tcl_Interp *interp, int code, int tty);

/* These are globally visible and can be replaced */

void (*tclErrorHandler)(Tcl_Interp *interp, int code, int tty) = NULL;

void (*tclPrepOutputHandler)(Tcl_Interp *interp, int code, int tty)
     = plPrepOutputHandler;

/* Options data structure definition. */

static char *tclStartupScript = NULL;
static char *pltcl_notes[] = {
    "Specifying the filename on the command line is compatible with modern",
    "tclsh syntax.  Old tclsh's used the -f syntax, which is still supported.",
    "You may use either syntax but not both.",
    NULL};

static PLOptionTable options[] = {
{
    "f",			/* File to read & process */
    NULL,
    NULL,
    &tclStartupScriptFileName,
    PL_OPT_STRING,
    "-f",
    "File from which to read commands" },
{
    "file",			/* File to read & process (alias) */
    NULL,
    NULL,
    &tclStartupScriptFileName,
    PL_OPT_STRING | PL_OPT_INVISIBLE,
    "-file",
    "File from which to read commands" },
{
    "e",			/* Script to run on startup */
    NULL,
    NULL,
    &tclStartupScript,
    PL_OPT_STRING,
    "-e",
    "Script to execute on startup" },
{
    NULL,			/* option */
    NULL,			/* handler */
    NULL,			/* client data */
    NULL,			/* address of variable to set */
    0,				/* mode flag */
    NULL,			/* short syntax */
    NULL }			/* long syntax */
};


/*
 *----------------------------------------------------------------------
 *
 * TclSetStartupScriptFileName --
 *
 *	Primes the startup script file name, used to override the
 *      command line processing.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	This procedure initializes the file name of the Tcl script to
 *      run at startup.
 *
 *----------------------------------------------------------------------
 */
void TclSetStartupScriptFileName(char *fileName)
{
    tclStartupScriptFileName = fileName;
}


/*
 *----------------------------------------------------------------------
 *
 * TclGetStartupScriptFileName --
 *
 *	Gets the startup script file name, used to override the
 *      command line processing.
 *
 * Results:
 *	The startup script file name, NULL if none has been set.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
char *TclGetStartupScriptFileName(void)
{
    return tclStartupScriptFileName;
}



/*
 *----------------------------------------------------------------------
 *
 * Tcl_Main --
 *
 *	Main program for tclsh and most other Tcl-based applications.
 *
 * Results:
 *	None. This procedure never returns (it exits the process when
 *	it's done.
 *
 * Side effects:
 *	This procedure initializes the Tcl world and then starts
 *	interpreting commands;  almost anything could happen, depending
 *	on the script being interpreted.
 *
 *----------------------------------------------------------------------
 */

int
pltclMain(int argc, char **argv, char *RcFileName /* OBSOLETE */,
	  int (*appInitProc)(Tcl_Interp *interp))
{
    Tcl_Obj *resultPtr;
    Tcl_Obj *commandPtr = NULL;
    char buffer[1000], *args;
    int code, gotPartial, tty, length;
    int exitCode = 0;
    Tcl_Channel inChannel, outChannel, errChannel;
    Tcl_Interp *interp;
    Tcl_DString argString;

    char usage[500];

    Tcl_FindExecutable(argv[0]);
    interp = Tcl_CreateInterp();
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
#endif

    /* First process plplot-specific args using the PLplot parser. */

    sprintf(usage, "\nUsage:\n        %s [filename] [options]\n", argv[0]);
    plSetUsage(NULL, usage);
    plMergeOpts(options, "pltcl options", pltcl_notes);
    (void) plparseopts(&argc, argv, PL_PARSE_FULL | PL_PARSE_SKIP );

    /*
     * Make (remaining) command-line arguments available in the Tcl variables
     * "argc" and "argv".  If the first argument doesn't start with a "-" then
     * strip it off and use it as the name of a script file to process.
     */

    if (tclStartupScriptFileName == NULL) {
	if ((argc > 1) && (argv[1][0] != '-')) {
	    tclStartupScriptFileName = argv[1];
	    argc--;
	    argv++;
	}
    }
    args = Tcl_Merge(argc-1, (CONST char * CONST *) argv+1);
    Tcl_ExternalToUtfDString(NULL, args, -1, &argString);
    Tcl_SetVar(interp, "argv", Tcl_DStringValue(&argString), TCL_GLOBAL_ONLY);
    Tcl_DStringFree(&argString);
    ckfree(args);

    if (tclStartupScriptFileName == NULL) {
	Tcl_ExternalToUtfDString(NULL, argv[0], -1, &argString);
    } else {
	tclStartupScriptFileName = Tcl_ExternalToUtfDString(NULL,
		tclStartupScriptFileName, -1, &argString);
    }

    TclFormatInt(buffer, argc-1);
    Tcl_SetVar(interp, "argc", buffer, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", Tcl_DStringValue(&argString), TCL_GLOBAL_ONLY);

    /*
     * Set the "tcl_interactive" variable.
     */

    tty = isatty(0);
    Tcl_SetVar(interp, "tcl_interactive",
	    ((tclStartupScriptFileName == NULL) && tty) ? "1" : "0",
	    TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     */

    if ((*appInitProc)(interp) != TCL_OK) {
	errChannel = Tcl_GetStdChannel(TCL_STDERR);
	if (errChannel) {
	    Tcl_WriteChars(errChannel,
		    "application-specific initialization failed: ", -1);
	    Tcl_WriteObj(errChannel, Tcl_GetObjResult(interp));
	    Tcl_WriteChars(errChannel, "\n", 1);
	}
    }

    /*
     * Process the startup script, if any.
     */

    if (tclStartupScript != NULL) {
	code = Tcl_VarEval(interp, tclStartupScript, (char *) NULL);
	if (code != TCL_OK) {
	    fprintf(stderr, "%s\n", interp->result);
	    exitCode = 1;
	}
    }

    /*
     * If a script file was specified then just source that file
     * and quit.
     */

    if (tclStartupScriptFileName != NULL) {
	code = Tcl_EvalFile(interp, tclStartupScriptFileName);
	if (code != TCL_OK) {
	    errChannel = Tcl_GetStdChannel(TCL_STDERR);
	    if (errChannel) {
		/*
		 * The following statement guarantees that the errorInfo
		 * variable is set properly.
		 */

		Tcl_AddErrorInfo(interp, "");
		Tcl_WriteObj(errChannel, Tcl_GetVar2Ex(interp, "errorInfo",
			NULL, TCL_GLOBAL_ONLY));
		Tcl_WriteChars(errChannel, "\n", 1);
	    }
	    exitCode = 1;
	}
	goto done;
    }
    Tcl_DStringFree(&argString);

    /*
     * We're running interactively.  Source a user-specific startup
     * file if the application specified one and if the file exists.
     */

    Tcl_SourceRCFile(interp);

    /*
     * Process commands from stdin until there's an end-of-file.  Note
     * that we need to fetch the standard channels again after every
     * eval, since they may have been changed.
     */

    commandPtr = Tcl_NewObj();
    Tcl_IncrRefCount(commandPtr);

    inChannel = Tcl_GetStdChannel(TCL_STDIN);
    outChannel = Tcl_GetStdChannel(TCL_STDOUT);
    gotPartial = 0;
    while (1) {
	if (tty) {
	    Tcl_Obj *promptCmdPtr;

	    promptCmdPtr = Tcl_GetVar2Ex(interp,
		    (gotPartial ? "tcl_prompt2" : "tcl_prompt1"),
		    NULL, TCL_GLOBAL_ONLY);
	    if (promptCmdPtr == NULL) {
                defaultPrompt:
		if (!gotPartial && outChannel) {
		    Tcl_WriteChars(outChannel, "% ", 2);
		}
	    } else {
		code = Tcl_EvalObjEx(interp, promptCmdPtr, 0);
		inChannel = Tcl_GetStdChannel(TCL_STDIN);
		outChannel = Tcl_GetStdChannel(TCL_STDOUT);
		errChannel = Tcl_GetStdChannel(TCL_STDERR);
		if (code != TCL_OK) {
		    if (errChannel) {
			Tcl_WriteObj(errChannel, Tcl_GetObjResult(interp));
			Tcl_WriteChars(errChannel, "\n", 1);
		    }
		    Tcl_AddErrorInfo(interp,
			    "\n    (script that generates prompt)");
		    goto defaultPrompt;
		}
	    }
	    if (outChannel) {
		Tcl_Flush(outChannel);
	    }
	}
	if (!inChannel) {
	    goto done;
	}
        length = Tcl_GetsObj(inChannel, commandPtr);
	if (length < 0) {
	    goto done;
	}
	if ((length == 0) && Tcl_Eof(inChannel) && (!gotPartial)) {
	    goto done;
	}

        /*
         * Add the newline removed by Tcl_GetsObj back to the string.
         */

	Tcl_AppendToObj(commandPtr, "\n", 1);
	if (!TclObjCommandComplete(commandPtr)) {
	    gotPartial = 1;
	    continue;
	}

	gotPartial = 0;
	code = Tcl_RecordAndEvalObj(interp, commandPtr, 0);
	inChannel = Tcl_GetStdChannel(TCL_STDIN);
	outChannel = Tcl_GetStdChannel(TCL_STDOUT);
	errChannel = Tcl_GetStdChannel(TCL_STDERR);
	Tcl_DecrRefCount(commandPtr);
	commandPtr = Tcl_NewObj();
	Tcl_IncrRefCount(commandPtr);

	/* User defined function to deal with tcl command output */
	/* Deprecated; for backward compatibility only */
	if ( ((code != TCL_OK) || tty) && tclErrorHandler )
	    (*tclErrorHandler)(interp, code, tty);
	else {
	    /* User defined function to prepare for tcl output */
	    /* This is the new way */
	    if ( ((code != TCL_OK) || tty) && tclPrepOutputHandler )
		(*tclPrepOutputHandler)(interp, code, tty);
	    /* Back to the stock tcl code */
	    if (code != TCL_OK) {
		if (errChannel) {
		    Tcl_WriteObj(errChannel, Tcl_GetObjResult(interp));
		    Tcl_WriteChars(errChannel, "\n", 1);
		}
	    } else if (tty) {
		resultPtr = Tcl_GetObjResult(interp);
		Tcl_GetStringFromObj(resultPtr, &length);
		if ((length > 0) && outChannel) {
		    Tcl_WriteObj(outChannel, resultPtr);
		    Tcl_WriteChars(outChannel, "\n", 1);
		}
	    }
	}
#ifdef TCL_MEM_DEBUG
	if (tclMemDumpFileName != NULL) {
	    Tcl_DecrRefCount(commandPtr);
	    Tcl_DeleteInterp(interp);
	    Tcl_Exit(0);
	}
#endif
    }

    /*
     * Rather than calling exit, invoke the "exit" command so that
     * users can replace "exit" with some other command to do additional
     * cleanup on exit.  The Tcl_Eval call should never return.
     */

    done:
    if (commandPtr != NULL) {
	Tcl_DecrRefCount(commandPtr);
    }
    sprintf(buffer, "exit %d", exitCode);
    Tcl_Eval(interp, buffer);
    return 0;		/* to silence warnings */
}

/*
 *--------------------------------------------------------------------------
 *
 * plPrepOutputHandler --
 *
 *	Prepares for output during command parsing.  We use it here to
 *	ensure we are on the text screen before issuing the error message,
 *	otherwise it may disappear.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	For some graphics devices, a switch between graphics and text modes
 *	is done.
 *
 *--------------------------------------------------------------------------
 */

static void
plPrepOutputHandler(Tcl_Interp *interp, int code, int tty)
{
    pltext();
}
