/* $Id: plplotter.c 3186 2006-02-15 18:17:33Z slbrow $
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

/*
    plplotter.c

    Copyright 1993, 1994, 1995
    Maurice LeBrun                      mjl@dino.ph.utexas.edu
    Institute for Fusion Studies        University of Texas at Austin

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Based upon tkFrame.c from the TK 3.2 distribution:

    Copyright 1990 Regents of the University of California.
    Permission to use, copy, modify, and distribute this
    software and its documentation for any purpose and without
    fee is hereby granted, provided that the above copyright
    notice appear in all copies.  The University of California
    makes no representations about the suitability of this
    software for any purpose.  It is provided "as is" without
    express or implied warranty.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    This module implements "plframe" widgets for the Tk toolkit.  These are
    frames that have extra logic to allow them to be interfaced with the
    PLplot X driver.  These are then drawn into and respond to keyboard and
    mouse events.
*/
/*
#define DEBUG_ENTER
#define DEBUG
*/

#define NEED_PLDEBUG
#include "plserver.h"
#include "pltkwd.h"
#include "tcpip.h"

#ifdef __WIN32__
#define XSynchronize(display, bool) {display->request++;}
#define XSync(display, bool) {display->request++;}
#define XFlush(display)
#endif

#ifdef DEBUG_ENTER
/* This version of dbug_enter works cross-platform */
#undef dbug_enter
#define dbug_enter(a) if(plsc->debug) { \
    Tcl_Write(Tcl_GetStdChannel(TCL_STDERR),a,-1);}
#endif

#ifndef __WIN32__
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#include <fcntl.h>
#endif
#endif

/* plplot_tkwin_ccmap is statically defined in pltkwd.h.  Note that
 * tkwin.c also includes that header and uses that variable. */

#define NDEV    20              /* Max number of output device types */

/*
 * A data structure of the following type is kept for each
 * plframe that currently exists for this process:
 */

typedef struct PlPlotter {

/* This is stuff taken from tkFrame.c */

    Tk_Window tkwin;            /* Window that embodies the frame.  NULL
                                 * means that the window has been destroyed
                                 * but the data structures haven't yet been
                                 * cleaned up.*/
    Display *display;           /* Display containing widget.  Used, among
                                 * other things, so that resources can be
                                 * freed even after tkwin has gone away. */
    Tcl_Interp *interp;         /* Interpreter associated with
                                 * widget.  Used to delete widget
                                 * command.  */
    Tcl_Command widgetCmd;      /* Token for frame's widget command. */
    Tk_3DBorder border;         /* Structure used to draw 3-D border and
                                 * background. */
    int borderWidth;            /* Width of 3-D border (if any). */
    int relief;                 /* 3-d effect: TK_RELIEF_RAISED etc. */
    int width;                  /* Width to request for window.  <= 0 means
                                 * don't request any size. */
    int height;                 /* Height to request for window.  <= 0 means
                                 * don't request any size. */
    Tk_Cursor cursor;           /* Current cursor for window, or None. */
    int flags;                  /* Various flags;  see below for
                                 * definitions. */

/* These are new to plframe widgets */

/* control stuff */

    int tkwin_initted;          /* Set first time widget is mapped */
    PLStream *pls;              /* PLplot stream pointer */
    PLINT ipls;                 /* PLplot stream number */
    PLINT ipls_save;            /* PLplot stream number, save files */

    PLRDev *plr;                /* Renderer state information.  Malloc'ed */
    char *plpr_cmd;             /* Holds print command name.  Malloc'ed */

/* Used to allow active stuff on the plot */
    int active_plot;            /* The plot responds to mouse movement etc. */
    int isActive;               /* Used to turn event handling on and off. */

/* Used to handle resize and expose events */

    PLDisplay pldis;            /* Info about the display window */
    int prevWidth;              /* Previous window width */
    int prevHeight;             /* Previous window height */

/* Support for save operations */

    char *SaveFnam;             /* File name we are currently saving to.
                                   Malloc'ed. */
    char **devDesc;             /* Descriptive names for file-oriented
                                 * devices.  Malloc'ed. */
    char **devName;             /* Keyword names of file-oriented devices.
                                 * Malloc'ed. */

/* Used in selecting & modifying plot or device area */

    GC xorGC;                   /* GC used for rubber-band drawing */
    XPoint pts[5];              /* Points for rubber-band drawing */
    int continue_draw;          /* Set when doing rubber-band draws */
    Tk_Cursor xhair_cursor;     /* cursor used for drawing */
    PLFLT xl, xr, yl, yr;       /* Bounds on plot viewing area */
    char *xScrollCmd;           /* Command prefix for communicating with
                                 * horizontal scrollbar.  NULL means no
                                 * command to issue.  Malloc'ed. */
    char *yScrollCmd;           /* Command prefix for communicating with
                                 * vertical scrollbar.  NULL means no
                                 * command to issue.  Malloc'ed. */

/* Used for flashing bop or eop condition */

    char *bopCmd;               /* Proc to call at bop */
    char *eopCmd;               /* Proc to call at eop */

/* Used for drawing graphic crosshairs */

    int xhairs;                 /* Configuration option to turn on xhairs */
    int drawing_xhairs;         /* Set if we are currently drawing xhairs */
    XPoint xhair_x[2];          /* Points for horizontal xhair line */
    XPoint xhair_y[2];          /* Points for vertical xhair line */

/* Used for drawing a rubber band lilne segment */

    int rband;                  /* Configuration option to turn on rband */
    int drawing_rband;          /* See if we are currently drawing rband */
    XPoint rband_pt[2];         /* Ends of rubber band line */
    int double_buffer;		/* Double buffering configuration option */
} PlPlotter;

/*
 * Flag bits for plframes:
 *
 * REFRESH_PENDING:             Non-zero means a DoWhenIdle handler
 *                              has already been queued to refresh
 *                              this window.
 * RESIZE_PENDING;              Used to reschedule resize events
 * REDRAW_PENDING;              Used to redraw contents of plot buffer
 * UPDATE_V_SCROLLBAR:          Non-zero means vertical scrollbar needs
 *                              to be updated.
 * UPDATE_H_SCROLLBAR:          Non-zero means horizontal scrollbar needs
 *                              to be updated.
 */

#define REFRESH_PENDING         1
#define RESIZE_PENDING          2
#define REDRAW_PENDING          4
#define UPDATE_V_SCROLLBAR      8
#define UPDATE_H_SCROLLBAR      16

/* Defaults for plframes: */

#define DEF_PLFRAME_BG_COLOR            "Black"
#define DEF_PLFRAME_BG_MONO             "White"
#define DEF_PLFRAME_BORDER_WIDTH        "0"
#define DEF_PLFRAME_CURSOR              ((char *) NULL)
#define DEF_PLFRAME_HEIGHT              "250"
#define DEF_PLFRAME_RELIEF              "flat"
#define DEF_PLFRAME_WIDTH               "250"

/* Configuration info */

static Tk_ConfigSpec configSpecs[] = {
    {TK_CONFIG_BOOLEAN, "-activeplot", (char *) NULL, (char *) NULL,
        "1", Tk_Offset(PlPlotter, active_plot), TK_CONFIG_DONT_SET_DEFAULT},
    {TK_CONFIG_BORDER, "-background", "background", "Background",
	DEF_PLFRAME_BG_COLOR, Tk_Offset(PlPlotter, border),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-background", "background", "Background",
	DEF_PLFRAME_BG_MONO, Tk_Offset(PlPlotter, border),
	TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_SYNONYM, "-bd", "borderWidth", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-bg", "background", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_STRING, "-bopcmd", "bopcmd", "PgCommand",
        (char *) NULL, Tk_Offset(PlPlotter, bopCmd), TK_CONFIG_NULL_OK},
    {TK_CONFIG_PIXELS, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_PLFRAME_BORDER_WIDTH, Tk_Offset(PlPlotter, borderWidth), 0},
    {TK_CONFIG_BOOLEAN, "-doublebuffer", (char *) NULL, (char *) NULL,
        "0", Tk_Offset(PlPlotter, double_buffer), TK_CONFIG_DONT_SET_DEFAULT},
    {TK_CONFIG_ACTIVE_CURSOR, "-cursor", "cursor", "Cursor",
        DEF_PLFRAME_CURSOR, Tk_Offset(PlPlotter, cursor), TK_CONFIG_NULL_OK},
    {TK_CONFIG_STRING, "-eopcmd", "eopcmd", "PgCommand",
        (char *) NULL, Tk_Offset(PlPlotter, eopCmd), TK_CONFIG_NULL_OK},
    {TK_CONFIG_PIXELS, "-height", "height", "Height",
        DEF_PLFRAME_HEIGHT, Tk_Offset(PlPlotter, height), 0},
    {TK_CONFIG_RELIEF, "-relief", "relief", "Relief",
        DEF_PLFRAME_RELIEF, Tk_Offset(PlPlotter, relief), 0},
    {TK_CONFIG_PIXELS, "-width", "width", "Width",
        DEF_PLFRAME_WIDTH, Tk_Offset(PlPlotter, width), 0},
    {TK_CONFIG_BOOLEAN, "-xhairs", (char *) NULL, (char *) NULL,
        "0", Tk_Offset(PlPlotter, xhairs), TK_CONFIG_DONT_SET_DEFAULT},
    {TK_CONFIG_BOOLEAN, "-rubberband", (char *) NULL, (char *) NULL,
        "0", Tk_Offset(PlPlotter, rband), TK_CONFIG_DONT_SET_DEFAULT},
	{TK_CONFIG_STRING, "-xscrollcommand", "xScrollCommand", "ScrollCommand",
        (char *) NULL, Tk_Offset(PlPlotter, xScrollCmd), TK_CONFIG_NULL_OK},
    {TK_CONFIG_STRING, "-yscrollcommand", "yScrollCommand", "ScrollCommand",
        (char *) NULL, Tk_Offset(PlPlotter, yScrollCmd), TK_CONFIG_NULL_OK},
    {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
        (char *) NULL, 0, 0}
};

/* Forward declarations for procedures defined later in this file: */

/* Externals */

int   plPlotterCmd        (ClientData, Tcl_Interp *, int, char **);
void PlplotterAtEop(Tcl_Interp *interp, register PlPlotter *plPlotterPtr);
void PlplotterAtBop(Tcl_Interp *interp, register PlPlotter *plPlotterPtr);

/* These are invoked by the TK dispatcher */

static void  DestroyPlPlotter     (ClientData);
static void  DisplayPlPlotter     (ClientData);
static void  PlPlotterInit        (ClientData);
static void  PlPlotterFirstInit   (ClientData clientData);
static void  PlPlotterConfigureEH (ClientData, XEvent *);
static void  PlPlotterExposeEH    (ClientData, XEvent *);
static void  PlPlotterMotionEH    (ClientData, register XEvent *);
static void  PlPlotterEnterEH     (ClientData, register XEvent *);
static void  PlPlotterLeaveEH     (ClientData, register XEvent *);
static void  PlPlotterButtonPressEH (ClientData    clientData, register XEvent *);
static int   PlPlotterWidgetCmd   (ClientData, Tcl_Interp *, int, CONST char **);
static int   ReadData           (ClientData, int);
static void  Install_cmap       (PlPlotter *plPlotterPtr);

/* These are invoked by PlPlotterWidgetCmd to process widget commands */

static int   Closelink          (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   Cmd                (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   ConfigurePlPlotter (Tcl_Interp *, PlPlotter *, int, CONST char **, int);
static int   Draw               (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   Info               (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   Openlink           (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   Orient             (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   Page               (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   NextPage           (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   Print              (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   Redraw             (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   Save               (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   View               (Tcl_Interp *, PlPlotter *, int, CONST char **);
static int   Scroll             (Tcl_Interp *, PlPlotter *);
static int   report             (Tcl_Interp *, PlPlotter *, int, CONST char **);

/* Routines for manipulating graphic crosshairs */

static void  ActiveState        (register PlPlotter *plPlotterPtr, int on);
static void  CreateXhairs       (PlPlotter *);
static void  DestroyXhairs      (PlPlotter *);
static void  DrawXhairs         (PlPlotter *, int, int);
static void  UpdateXhairs       (PlPlotter *);

/* Routines for manipulating the rubberband line */

static void  CreateRband        (PlPlotter *);
static void  DestroyRband       (PlPlotter *);
static void  DrawRband          (PlPlotter *, int, int);
static void  UpdateRband        (PlPlotter *);

/* Utility routines */

static void  gbox               (PLFLT *, PLFLT *, PLFLT *, PLFLT *, CONST char **);
static void  UpdateVScrollbar   (register PlPlotter *);
static void  UpdateHScrollbar   (register PlPlotter *);

/*
 *---------------------------------------------------------------------------
 *
 * plPlotterCmd --
 *
 *      This procedure is invoked to process the "plframe" Tcl
 *      command.  See the user documentation for details on what it
 *      does.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      See the user documentation.
 *
 *---------------------------------------------------------------------------
 */

int
plPlotterCmd(ClientData clientData, Tcl_Interp *interp,
           int argc, char **argv)
{
    Tk_Window tkwin;
    register PlPlotter *plPlotterPtr;
    register PLRDev *plr;
    int i, ndev;

    dbug_enter("plPlotterCmd");

    if (argc < 2) {
        Tcl_AppendResult(interp, "wrong # args: should be \"",
                argv[0], " pathName ?options?\"", (char *) NULL);
        return TCL_ERROR;
    }

/* Create the window. */

    tkwin = Tk_CreateWindowFromPath(interp, Tk_MainWindow(interp), argv[1], (char *) NULL);
    if (tkwin == NULL) {
        return TCL_ERROR;
    }
    Tk_SetClass(tkwin, "Plframe");

    plPlotterPtr = (PlPlotter *) malloc(sizeof(PlPlotter));
    plPlotterPtr->tkwin = tkwin;
    plPlotterPtr->display = Tk_Display(tkwin);
    plPlotterPtr->interp = interp;
    plPlotterPtr->widgetCmd =
      Tcl_CreateCommand(interp, Tk_PathName(plPlotterPtr->tkwin),
			(Tcl_CmdProc*) PlPlotterWidgetCmd, (ClientData) plPlotterPtr,
			(Tcl_CmdDeleteProc*) NULL);
    plPlotterPtr->xorGC = NULL;
    plPlotterPtr->border = NULL;
    plPlotterPtr->cursor = None;
    plPlotterPtr->xhair_cursor = None;
    plPlotterPtr->flags = 0;
    plPlotterPtr->width  = Tk_Width(plPlotterPtr->tkwin);
    plPlotterPtr->height = Tk_Height(plPlotterPtr->tkwin);
    plPlotterPtr->prevWidth = 0;
    plPlotterPtr->prevHeight = 0;
    plPlotterPtr->continue_draw = 0;
    plPlotterPtr->ipls = 0;
    plPlotterPtr->ipls_save = 0;
    plPlotterPtr->tkwin_initted = 0;
    plPlotterPtr->plpr_cmd = NULL;
    plPlotterPtr->bopCmd = NULL;
    plPlotterPtr->eopCmd = NULL;
    plPlotterPtr->xhairs = 0;
    plPlotterPtr->active_plot = 1;
    plPlotterPtr->isActive = 0;
    plPlotterPtr->drawing_xhairs = 0;
    plPlotterPtr->rband = 0;
    plPlotterPtr->drawing_rband = 0;
    plPlotterPtr->xScrollCmd = NULL;
    plPlotterPtr->yScrollCmd = NULL;
    plPlotterPtr->xl = 0.;
    plPlotterPtr->yl = 0.;
    plPlotterPtr->xr = 1.;
    plPlotterPtr->yr = 1.;
    plPlotterPtr->SaveFnam = NULL;

    plPlotterPtr->plr = (PLRDev *) malloc(sizeof(PLRDev));
    plr = plPlotterPtr->plr;
    plr->pdfs = NULL;
    plr->iodev = (PLiodev *) malloc(sizeof(PLiodev));
    plr_start(plr);

/* Associate new PLplot stream with this widget */

    plmkstrm(&plPlotterPtr->ipls);
    plgpls(&plPlotterPtr->pls);
    /* Tell the stream about myself */
    plPlotterPtr->pls->plPlotterPtr = plPlotterPtr;

/* Set up stuff for rubber-band drawing */

    plPlotterPtr->xhair_cursor =
        Tk_GetCursor(plPlotterPtr->interp, plPlotterPtr->tkwin, "crosshair");

/* Partially initialize Tk driver. */

    plD_open_tkwin(plPlotterPtr->pls);

/* Create list of valid device names and keywords for page dumps */

    plPlotterPtr->devDesc = (char **) malloc(NDEV * sizeof(char **));
    plPlotterPtr->devName = (char **) malloc(NDEV * sizeof(char **));
    for (i = 0; i < NDEV; i++) {
        plPlotterPtr->devDesc[i] = NULL;
        plPlotterPtr->devName[i] = NULL;
    }
    ndev = NDEV;
    plgFileDevs(&plPlotterPtr->devDesc, &plPlotterPtr->devName, &ndev);

/* Start up event handlers and other good stuff */

    Tk_CreateEventHandler(plPlotterPtr->tkwin, StructureNotifyMask,
                          PlPlotterConfigureEH, (ClientData) plPlotterPtr);

    Tk_CreateEventHandler(plPlotterPtr->tkwin, ExposureMask,
                          PlPlotterExposeEH, (ClientData) plPlotterPtr);

/* for active plot */
    ActiveState(plPlotterPtr, 1);

    if (ConfigurePlPlotter(interp, plPlotterPtr, argc-2, (CONST char**) argv+2, 0) != TCL_OK) {
        Tk_DestroyWindow(plPlotterPtr->tkwin);
        return TCL_ERROR;
    }
    Tk_MakeWindowExist(plPlotterPtr->tkwin);
    PlPlotterFirstInit((ClientData)plPlotterPtr);
    Tk_GeometryRequest(plPlotterPtr->tkwin,200,200);

    interp->result = Tk_PathName(plPlotterPtr->tkwin);

    return TCL_OK;
}

/*
 *---------------------------------------------------------------------------
 *
 * PlPlotterWidgetCmd --
 *
 *      This procedure is invoked to process the Tcl command that
 *      corresponds to a plframe widget.  See the user
 *      documentation for details on what it does.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      See the user documentation.
 *
 *---------------------------------------------------------------------------
 */

static int
PlPlotterWidgetCmd(ClientData clientData, Tcl_Interp *interp,
                 int argc, CONST char **argv)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;
    int result = TCL_OK;
    int length;
    char c;

    dbug_enter("PlPlotterWidgetCmd");

    if (argc < 2) {
        Tcl_AppendResult(interp, "wrong # args: should be \"",
                argv[0], " option ?arg arg ...?\"", (char *) NULL);
        return TCL_ERROR;
    }
    Tk_Preserve((ClientData) plPlotterPtr);
    c = argv[1][0];
    length = strlen(argv[1]);

/* cmd -- issue a command to the PLplot library */

    if ((c == 'c') && (strncmp(argv[1], "cmd", length) == 0)) {
        result = Cmd(interp, plPlotterPtr, argc-2, argv+2);
    }

/* configure */

    else if ((c == 'c') && (strncmp(argv[1], "cget", length) == 0)
	    && (length >= 2)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " cget option\"",
		    (char *) NULL);
	    result = TCL_ERROR;
	    goto done;
	}
	result = Tk_ConfigureValue(interp, plPlotterPtr->tkwin, configSpecs,
		(char *) plPlotterPtr, argv[2], 0);
    } else if ((c == 'c') && (strncmp(argv[1], "configure", length) == 0)) {
        if (argc == 2) {
            result = Tk_ConfigureInfo(interp, plPlotterPtr->tkwin, configSpecs,
                    (char *) plPlotterPtr, (char *) NULL, 0);
        }
        else if (argc == 3) {
            result = Tk_ConfigureInfo(interp, plPlotterPtr->tkwin, configSpecs,
                    (char *) plPlotterPtr, argv[2], 0);
        }
        else {
            result = ConfigurePlPlotter(interp, plPlotterPtr, argc-2, argv+2,
                    TK_CONFIG_ARGV_ONLY);
        }
    }

/* closelink -- Close a binary data link previously opened with openlink */

    else if ((c == 'c') && (strncmp(argv[1], "closelink", length) == 0)) {
        if (argc > 2) {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
                    argv[0], (char *) NULL);
            result = TCL_ERROR;
            goto done;
        }
        else {
            result = Closelink(interp, plPlotterPtr, argc-2, argv+2);
        }
    }

/* draw -- rubber-band draw used in region selection */

    else if ((c == 'd') && (strncmp(argv[1], "draw", length) == 0)) {
        if (argc == 2) {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
                    argv[0], " draw op ?options?\"", (char *) NULL);
            result = TCL_ERROR;
            goto done;
        }
        else {
            result = Draw(interp, plPlotterPtr, argc-2, argv+2);
        }
    }

/* info -- returns requested info */

    else if ((c == 'i') && (strncmp(argv[1], "info", length) == 0)) {
        result = Info(interp, plPlotterPtr, argc-2, argv+2);
    }

/* next page.  called to cancel wait for page in tkwin driver */

    else if ((c == 'n') && (strncmp(argv[1], "nextpage", length) == 0)) {
        result = NextPage(interp, plPlotterPtr, argc-2, argv+2);
    }

/* orient -- Set plot orientation */

    else if ((c == 'o') && (strncmp(argv[1], "orient", length) == 0)) {
        result = Orient(interp, plPlotterPtr, argc-2, argv+2);
    }

/* openlink -- Open a binary data link (FIFO or socket) */

    else if ((c == 'o') && (strncmp(argv[1], "openlink", length) == 0)) {
        if (argc < 3) {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
                    argv[0], " option ?arg arg ...?\"", (char *) NULL);
            result = TCL_ERROR;
            goto done;
        }
        else {
            result = Openlink(interp, plPlotterPtr, argc-2, argv+2);
        }
    }

/* page -- change or return output page setup */

    else if ((c == 'p') && (strncmp(argv[1], "page", length) == 0)) {
        result = Page(interp, plPlotterPtr, argc-2, argv+2);
    }

/* print -- prints plot */

    else if ((c == 'p') && (strncmp(argv[1], "print", length) == 0)) {
        result = Print(interp, plPlotterPtr, argc-2, argv+2);
    }

/* redraw -- redraw plot */

    else if ((c == 'r') && (strncmp(argv[1], "redraw", length) == 0)) {
        if (argc > 2) {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
                    argv[0], " redraw\"", (char *) NULL);
            result = TCL_ERROR;
            goto done;
        }
        else {
            result = Redraw(interp, plPlotterPtr, argc-2, argv+2);
        }
    }

/* report -- find out useful info about the plframe (GMF) */

    else if ((c == 'r') && (strncmp(argv[1], "report", length) == 0)) {
        result = report( interp, plPlotterPtr, argc-2, argv+2 );
    }

/* save -- saves plot to the specified plot file type */

    else if ((c == 's') && (strncmp(argv[1], "save", length) == 0)) {
        result = Save(interp, plPlotterPtr, argc-2, argv+2);
    }

/* view -- change or return window into plot */

    else if ((c == 'v') && (strncmp(argv[1], "view", length) == 0)) {
        result = View(interp, plPlotterPtr, argc-2, argv+2);
    }

/* xscroll -- horizontally scroll window into plot */

    else if ((c == 'x') && (strncmp(argv[1], "xview", length) == 0)) {
	int count, type;
	double width = (double) (plPlotterPtr->xr - plPlotterPtr->xl);

	double fraction;

	type = Tk_GetScrollInfo(interp, argc, argv, &fraction, &count);
	switch (type) {
	  case TK_SCROLL_ERROR:
	    result = TCL_ERROR;
	    goto done;
	  case TK_SCROLL_MOVETO:
	    plPlotterPtr->xl = (PLFLT) fraction;
	    plPlotterPtr->xr = (PLFLT) (fraction + width);
	    break;
	  case TK_SCROLL_PAGES:
	    plPlotterPtr->xl += (PLFLT) (count * width * .9);
	    plPlotterPtr->xr += (PLFLT) (count * width * .9);
	    break;
	  case TK_SCROLL_UNITS:
	    plPlotterPtr->xl += (PLFLT) (count*width/50);
	    plPlotterPtr->xr += (PLFLT) (count*width/50);
	    break;
	}
	if(plPlotterPtr->xr > 1.0) {
	    plPlotterPtr->xr = 1.0;
	    plPlotterPtr->xl = (PLFLT) (1.0 - width);
	} else if(plPlotterPtr->xl < 0.0) {
	    plPlotterPtr->xl = 0.0;
	    plPlotterPtr->xr = (PLFLT) width;
	}
	Scroll(interp,plPlotterPtr);
    }

/* yscroll -- vertically scroll window into plot */

    else if ((c == 'y') && (strncmp(argv[1], "yview", length) == 0)) {
	int count, type;
	double height = plPlotterPtr->yr - plPlotterPtr->yl;

	double fraction;

	type = Tk_GetScrollInfo(interp, argc, argv, &fraction, &count);
	switch (type) {
	  case TK_SCROLL_ERROR:
	    result = TCL_ERROR;
	    goto done;
	  case TK_SCROLL_MOVETO:
	    plPlotterPtr->yl = (PLFLT) (1.0-fraction -height);
	    plPlotterPtr->yr = (PLFLT) (1.0-fraction);
	    break;
	  case TK_SCROLL_PAGES:
	    plPlotterPtr->yl -= (PLFLT) (count * height * .9);
	    plPlotterPtr->yr -= (PLFLT) (count * height * .9);
	    break;
	  case TK_SCROLL_UNITS:
	    plPlotterPtr->yl -= (PLFLT)(count*height/50);
	    plPlotterPtr->yr -= (PLFLT)(count*height/50);
	    break;
	}
	if(plPlotterPtr->yr > 1.0) {
	    plPlotterPtr->yr = 1.0;
	    plPlotterPtr->yl = (PLFLT) (1.0 - height);
	} else if(plPlotterPtr->yl < 0.0) {
	    plPlotterPtr->yl = 0.0;
	    plPlotterPtr->yr = (PLFLT)height;
	}
	Scroll(interp,plPlotterPtr);
    }

/* unrecognized widget command */

    else {
        Tcl_AppendResult(interp, "bad option \"", argv[1],
         "\":  must be closelink, cmd, configure, draw, info, nextpage ",
         "openlink, orient, page, print, redraw, save, view, ",
         "xview, or yview", (char *) NULL);

        result = TCL_ERROR;
    }

 done:
    Tk_Release((ClientData) plPlotterPtr);
    return result;
}

/*
 *---------------------------------------------------------------------------
 *
 * DestroyPlPlotter --
 *
 *      This procedure is invoked by Tk_EventuallyFree or Tk_Release to
 *      clean up the internal structure of a plframe at a safe time
 *      (when no-one is using it anymore).
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Everything associated with the plframe is freed up.
 *
 *---------------------------------------------------------------------------
 */

static void DestroyPlPlotter(ClientData clientData)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;
    register PLRDev *plr = plPlotterPtr->plr;
    TkwDev *dev = (TkwDev *) plPlotterPtr->pls->dev;

    dbug_enter("DestroyPlPlotter");

    dev->flags |= 0x3;

    if (plPlotterPtr->border != NULL) {
        Tk_Free3DBorder(plPlotterPtr->border);
    }
    if (plPlotterPtr->plpr_cmd != NULL) {
        free((char *) plPlotterPtr->plpr_cmd);
    }
    if (plPlotterPtr->cursor != None) {
        Tk_FreeCursor(plPlotterPtr->display, plPlotterPtr->cursor);
    }
    if (plPlotterPtr->xhair_cursor != None) {
        Tk_FreeCursor(plPlotterPtr->display, plPlotterPtr->xhair_cursor);
    }
    if (plPlotterPtr->xorGC != NULL) {
        Tk_FreeGC(plPlotterPtr->display, plPlotterPtr->xorGC);
    }
    if (plPlotterPtr->yScrollCmd != NULL) {
        free((char *) plPlotterPtr->yScrollCmd);
    }
    if (plPlotterPtr->xScrollCmd != NULL) {
        free((char *) plPlotterPtr->xScrollCmd);
    }
    if (plPlotterPtr->SaveFnam != NULL) {
        free((char *) plPlotterPtr->SaveFnam);
    }
    if (plPlotterPtr->devDesc != NULL) {
        free((char *) plPlotterPtr->devDesc);
    }
    if (plPlotterPtr->devName != NULL) {
        free((char *) plPlotterPtr->devName);
    }

/* Clean up data connection */

    pdf_close(plr->pdfs);
    free((char *) plPlotterPtr->plr->iodev);

/* Tell PLplot to clean up */

    plsstrm( plPlotterPtr->ipls );
    plend1();

/* Delete main data structures */

    free((char *) plPlotterPtr->plr);
    free((char *) plPlotterPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * PlPlotterConfigureEH --
 *
 *      Invoked by the Tk dispatcher on structure changes to a plframe.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      When the window gets deleted, internal structures get cleaned up.
 *      When it gets resized, it is redrawn.
 *
 *---------------------------------------------------------------------------
 */

static void
PlPlotterConfigureEH(ClientData clientData, register XEvent *eventPtr)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;
    register Tk_Window tkwin = plPlotterPtr->tkwin;

    dbug_enter("PlPlotterConfigureEH");

    switch (eventPtr->type) {

    case ConfigureNotify:
        pldebug("PLFrameConfigureEH", "ConfigureNotify\n");
        plPlotterPtr->flags |= RESIZE_PENDING;
        plPlotterPtr->width  = Tk_Width(tkwin);
        plPlotterPtr->height = Tk_Height(tkwin);
        if ((tkwin != NULL) && !(plPlotterPtr->flags & REFRESH_PENDING)) {
            Tcl_DoWhenIdle(DisplayPlPlotter, (ClientData) plPlotterPtr);
            plPlotterPtr->flags |= REFRESH_PENDING;
            plPlotterPtr->flags |= UPDATE_V_SCROLLBAR|UPDATE_H_SCROLLBAR;
        }
        break;

    case DestroyNotify:
        pldebug("PLFrameConfigureEH", "DestroyNotify\n");
        Tcl_DeleteCommand(plPlotterPtr->interp, Tk_PathName(tkwin));
        plPlotterPtr->tkwin = NULL;
        if (plPlotterPtr->flags & REFRESH_PENDING) {
            Tcl_CancelIdleCall(DisplayPlPlotter, (ClientData) plPlotterPtr);
        }
        Tk_EventuallyFree((ClientData) plPlotterPtr, (Tcl_FreeProc*) DestroyPlPlotter);
        break;

    case MapNotify:
        pldebug("PLFrameConfigureEH", "MapNotify\n");
        if (plPlotterPtr->flags & REFRESH_PENDING) {
            Tcl_CancelIdleCall(DisplayPlPlotter, (ClientData) plPlotterPtr);
        }

/* Vince thinks we don't want these lines any more */
/* We forced the window into existence when we created it. */
#if 0
    /* For some reason, "." must be mapped or PlPlotterInit will die (Note:
     * mapped & withdrawn or mapped in the withdrawn state is OK). Issuing
     * an update fixes this.  I'd love to know why this occurs.
     */

        if (!plPlotterPtr->tkwin_initted) {
            Tcl_VarEval(plPlotterPtr->interp, "update", (char *) NULL);
        }
#endif
	/* Set up window parameters and arrange for window to be refreshed */

	Tcl_DoWhenIdle(PlPlotterInit, (ClientData) plPlotterPtr);
	break;
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * PlPlotterExposeEH --
 *
 *      Invoked by the Tk dispatcher on exposes of a plframe.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Widget is redisplayed.
 *
 * Note: it's customary in Tk to collapse multiple exposes, so for best
 * performance without losing the window contents, I keep track of the
 * smallest single rectangle that can satisfy all expose events.  If there
 * are any overlaid graphics (like crosshairs), however, we need to refresh
 * the entire plot in order to have a predictable outcome.
 *
 *--------------------------------------------------------------------------- */

static void
PlPlotterExposeEH(ClientData clientData, register XEvent *eventPtr)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;
    XExposeEvent *event = (XExposeEvent *) eventPtr;
    register Tk_Window tkwin = plPlotterPtr->tkwin;

    dbug_enter("PlPlotterExposeEH");

    pldebug("PLFrameExposeEH", "Expose\n");

/* Set up the area to refresh */

    if ( ! (plPlotterPtr->drawing_xhairs || plPlotterPtr->drawing_rband) ) {
        int x0_old, x1_old, y0_old, y1_old, x0_new, x1_new, y0_new, y1_new;

        x0_old = plPlotterPtr->pldis.x;
        y0_old = plPlotterPtr->pldis.y;
        x1_old = x0_old + plPlotterPtr->pldis.width;
        y1_old = y0_old + plPlotterPtr->pldis.height;

        x0_new = event->x;
        y0_new = event->y;
        x1_new = x0_new + event->width;
        y1_new = y0_new + event->height;

        plPlotterPtr->pldis.x      = MIN(x0_old, x0_new);
        plPlotterPtr->pldis.y      = MIN(y0_old, y0_new);
        plPlotterPtr->pldis.width  = MAX(x1_old, x1_new) - plPlotterPtr->pldis.x;
        plPlotterPtr->pldis.height = MAX(y1_old, y1_new) - plPlotterPtr->pldis.y;
    }

/* Invoke DoWhenIdle handler to redisplay widget. */

    if (event->count == 0) {
        if ((tkwin != NULL) && !(plPlotterPtr->flags & REFRESH_PENDING)) {
            Tcl_DoWhenIdle(DisplayPlPlotter, (ClientData) plPlotterPtr);
            plPlotterPtr->width  = Tk_Width(tkwin);
            plPlotterPtr->height = Tk_Height(tkwin);
            plPlotterPtr->flags |= REFRESH_PENDING;
        }
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * PlPlotterMotionEH --
 *
 *      Invoked by the Tk dispatcher on MotionNotify events in a plframe.
 *      Not invoked unless we are drawing graphic crosshairs.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Graphic crosshairs are drawn.
 *
 *---------------------------------------------------------------------------
 */

static void
PlPlotterMotionEH(ClientData clientData, register XEvent *eventPtr)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;
    XMotionEvent *event = (XMotionEvent *) eventPtr;

    dbug_enter("PlPlotterMotionEH");

    if (plPlotterPtr->drawing_xhairs) {
        DrawXhairs(plPlotterPtr, event->x, event->y);
    }
    if (plPlotterPtr->drawing_rband) {
        DrawRband(plPlotterPtr, event->x, event->y);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * PlPlotterEnterEH --
 *
 *      Invoked by the Tk dispatcher on EnterNotify events in a plframe.
 *      Not invoked unless we are drawing graphic crosshairs.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Graphic crosshairs are updated.
 *
 *--------------------------------------------------------------------------- */

static void
PlPlotterEnterEH(ClientData clientData, register XEvent *eventPtr)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;
    XCrossingEvent *crossingEvent = (XCrossingEvent *) eventPtr;

    dbug_enter("PlPlotterEnterEH");

    if (plPlotterPtr->xhairs) {
        DrawXhairs(plPlotterPtr, crossingEvent->x, crossingEvent->y);
        plPlotterPtr->drawing_xhairs = 1;
    }
    if (plPlotterPtr->rband) {
        plPlotterPtr->drawing_rband = 1;
        UpdateRband(plPlotterPtr);
        DrawRband(plPlotterPtr, crossingEvent->x, crossingEvent->y);
    }
}

/*
 *---------------------------------------------------------------------------
 *
 * PlPlotterLeaveEH --
 *
 *      Invoked by the Tk dispatcher on LeaveNotify events in a plframe.
 *      Not invoked unless we are drawing graphic crosshairs.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Graphic crosshairs are updated.
 *
 *--------------------------------------------------------------------------- */

static void
PlPlotterLeaveEH(ClientData clientData, register XEvent *eventPtr)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;

    dbug_enter("PlPlotterLeaveEH");

    if (plPlotterPtr->drawing_xhairs) {
        UpdateXhairs(plPlotterPtr);
        plPlotterPtr->drawing_xhairs = 0;
    }
    if (plPlotterPtr->drawing_rband) {
        UpdateRband(plPlotterPtr);
        plPlotterPtr->drawing_rband = 0;
    }
}

static void
PlPlotterButtonPressEH(ClientData clientData, register XEvent *eventPtr)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;
    XButtonEvent *event = (XButtonEvent *) eventPtr;

    /* Get modifier keys */
    switch (event->state) {
      case 256: /* plain */
        if(plPlotterPtr->drawing_rband) {
            UpdateRband(plPlotterPtr);
        }
        if(plPlotterPtr->rband)
            CreateRband(plPlotterPtr);
        break;
    }
}

/*--------------------------------------------------------------------------*\
 * CreateXhairs()
 *
 * Creates graphic crosshairs at current pointer location.
\*--------------------------------------------------------------------------*/

static void
CreateXhairs(PlPlotter *plPlotterPtr)
{
    register Tk_Window tkwin = plPlotterPtr->tkwin;
    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;


/* Find current pointer location and draw graphic crosshairs if pointer is */
/* inside our window. */

    if (XQueryPointer(plPlotterPtr->display, Tk_WindowId(tkwin),
                      &root, &child, &root_x, &root_y, &win_x, &win_y,
                      &mask)) {

        #ifdef MAC_TCL
        /* Mac Tk only has a partial implementation of the above function
           so we must fix it */
        Tk_GetRootCoords(tkwin,&win_x,&win_y);
        win_x = root_x - win_x;
        win_y = root_y - win_y;
        #endif
        if (win_x >= 0 && win_x < Tk_Width(tkwin) &&
            win_y >= 0 && win_y < Tk_Height(tkwin)) {
            DrawXhairs(plPlotterPtr, win_x, win_y);
            plPlotterPtr->drawing_xhairs = 1;
        }
    }

}

/*--------------------------------------------------------------------------*\
 * DestroyXhairs()
 *
 * Destroys graphic crosshairs.
\*--------------------------------------------------------------------------*/

static void
DestroyXhairs(PlPlotter *plPlotterPtr)
{

/* This draw removes the last set of graphic crosshairs */

    UpdateXhairs(plPlotterPtr);
    plPlotterPtr->drawing_xhairs = 0;
}

/*--------------------------------------------------------------------------*\
 * DrawXhairs()
 *
 * Draws graphic crosshairs at (x0, y0).  The first draw erases the old set.
\*--------------------------------------------------------------------------*/

static void
DrawXhairs(PlPlotter *plPlotterPtr, int x0, int y0)
{
    register Tk_Window tkwin = plPlotterPtr->tkwin;
    int xmin = 0, xmax = Tk_Width(tkwin) - 1;
    int ymin = 0, ymax = Tk_Height(tkwin) - 1;

    if (plPlotterPtr->drawing_xhairs)
        UpdateXhairs(plPlotterPtr);

    plPlotterPtr->xhair_x[0].x = xmin; plPlotterPtr->xhair_x[0].y = y0;
    plPlotterPtr->xhair_x[1].x = xmax; plPlotterPtr->xhair_x[1].y = y0;

    plPlotterPtr->xhair_y[0].x = x0; plPlotterPtr->xhair_y[0].y = ymin;
    plPlotterPtr->xhair_y[1].x = x0; plPlotterPtr->xhair_y[1].y = ymax;

    UpdateXhairs(plPlotterPtr);
}

/*--------------------------------------------------------------------------*\
 * UpdateXhairs()
 *
 * Updates graphic crosshairs.  If already there, they are erased.
\*--------------------------------------------------------------------------*/

static void
UpdateXhairs(PlPlotter *plPlotterPtr)
{
    register Tk_Window tkwin = plPlotterPtr->tkwin;

    XDrawLines(Tk_Display(tkwin), Tk_WindowId(tkwin),
               plPlotterPtr->xorGC, plPlotterPtr->xhair_x, 2,
               CoordModeOrigin);

    XDrawLines(Tk_Display(tkwin), Tk_WindowId(tkwin),
               plPlotterPtr->xorGC, plPlotterPtr->xhair_y, 2,
               CoordModeOrigin);
}

/*--------------------------------------------------------------------------*\
 * CreateRband()
 *
 * Initiate rubber banding.
\*--------------------------------------------------------------------------*/

static void
CreateRband(PlPlotter *plPlotterPtr)
{
    register Tk_Window tkwin = plPlotterPtr->tkwin;
    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;

/* Find current pointer location, and initiate rubber banding. */

    if (XQueryPointer(plPlotterPtr->display, Tk_WindowId(tkwin),
                      &root, &child, &root_x, &root_y, &win_x, &win_y,
                      &mask)) {
        #ifdef MAC_TCL
        /* Mac Tk only has a partial implementation of the above function
           so we must fix it */
        Tk_GetRootCoords(tkwin,&win_x,&win_y);
        win_x = root_x - win_x;
        win_y = root_y - win_y;
        #endif
        if (win_x >= 0 && win_x < Tk_Width(tkwin) &&
            win_y >= 0 && win_y < Tk_Height(tkwin)) {

        /* Okay, pointer is in our window. */
            plPlotterPtr->rband_pt[0].x = win_x;
            plPlotterPtr->rband_pt[0].y = win_y;

            DrawRband(plPlotterPtr, win_x, win_y);
            plPlotterPtr->drawing_rband = 1;
        } else {
        /* Hmm, somehow they turned it on without even being in the window.
           Just put the anchor in top left, they'll soon realize this is a
           mistake... */

            plPlotterPtr->rband_pt[0].x = 0;
            plPlotterPtr->rband_pt[0].y = 0;

            DrawRband(plPlotterPtr, win_x, win_y);
            plPlotterPtr->drawing_rband = 1;
        }
    }

}

/*--------------------------------------------------------------------------*\
 * DestroyRband()
 *
 * Turn off rubber banding.
\*--------------------------------------------------------------------------*/

static void
DestroyRband(PlPlotter *plPlotterPtr)
{

/* This draw removes the residual rubber band. */

    UpdateRband(plPlotterPtr);
    plPlotterPtr->drawing_rband = 0;
}

/*--------------------------------------------------------------------------*\
 * DrawRband()
 *
 * Draws a rubber band from the anchor to the current cursor location.
\*--------------------------------------------------------------------------*/

static void
DrawRband(PlPlotter *plPlotterPtr, int x0, int y0)
{
/* If the line is already up, clear it. */

    if (plPlotterPtr->drawing_rband)
        UpdateRband(plPlotterPtr);

    plPlotterPtr->rband_pt[1].x = x0; plPlotterPtr->rband_pt[1].y = y0;

    UpdateRband(plPlotterPtr);
}

/*--------------------------------------------------------------------------*\
 * UpdateRband()
 *
 * Updates rubber band.  If already there, it is erased.
\*--------------------------------------------------------------------------*/

static void
UpdateRband(PlPlotter *plPlotterPtr)
{
    register Tk_Window tkwin = plPlotterPtr->tkwin;

    XDrawLines(Tk_Display(tkwin), Tk_WindowId(tkwin),
               plPlotterPtr->xorGC, plPlotterPtr->rband_pt, 2,
               CoordModeOrigin);
}

/* First-time initialization */
static void PlPlotterFirstInit(ClientData clientData) {
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;
    register Tk_Window tkwin = plPlotterPtr->tkwin;

    plsstrm(plPlotterPtr->ipls);
    plsdev("tkwin");
/* We should probably rename plsxwin to plstkwin */
    plsxwin(Tk_WindowId(tkwin));
    plspause(0);
    plinit();
    if (plplot_tkwin_ccmap) {
	Install_cmap(plPlotterPtr);
    }
    plbop();

    plPlotterPtr->tkwin_initted = 1;
    plPlotterPtr->width  = Tk_Width(tkwin);
    plPlotterPtr->height = Tk_Height(tkwin);
    plPlotterPtr->prevWidth = plPlotterPtr->width;
    plPlotterPtr->prevHeight = plPlotterPtr->height;
}

/*
 *---------------------------------------------------------------------------
 *
 * PlPlotterInit --
 *
 *      Invoked to handle miscellaneous initialization after window gets
 *      mapped.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      PLplot internal parameters and device driver are initialized.
 *
 *--------------------------------------------------------------------------- */

static void
PlPlotterInit(ClientData clientData)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;

/* Set up window parameters and arrange for window to be refreshed */

    plPlotterPtr->flags |= REFRESH_PENDING;
    plPlotterPtr->flags |= UPDATE_V_SCROLLBAR | UPDATE_H_SCROLLBAR;

/* Draw plframe */

    DisplayPlPlotter(clientData);

    if (plPlotterPtr->xhairs)
        CreateXhairs(plPlotterPtr);

    if (plPlotterPtr->rband)
        CreateRband(plPlotterPtr);
}

/*
 *---------------------------------------------------------------------------
 *
 * Install_cmap --
 *
 *      Installs X driver color map as necessary when custom color maps
 *      are used.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Parent color maps may get changed.
 *
 *---------------------------------------------------------------------------
 */

static void
Install_cmap(PlPlotter *plPlotterPtr)
{
    TkwDev *dev;

#define INSTALL_COLORMAP_IN_TK
#ifdef  INSTALL_COLORMAP_IN_TK
    dev = (TkwDev *) plPlotterPtr->pls->dev;
    Tk_SetWindowColormap(Tk_MainWindow(plPlotterPtr->interp), dev->tkwd->map);

/*
 * If the colormap is local to this widget, the WM must be informed that
 * it should be installed when the widget gets the focus.  The top level
 * window must be added to the end of its own list, because otherwise the
 * window manager adds it to the front (as required by the ICCCM).  Thanks
 * to Paul Mackerras for providing this info in his TK photo widget.
 */

#else
    int count = 0;
    Window top, colormap_windows[5];

    top = Tk_WindowId(Tk_MainWindow(plPlotterPtr->interp));

    colormap_windows[count++] = Tk_WindowId(plPlotterPtr->tkwin);
    colormap_windows[count++] = top;

    if ( ! XSetWMColormapWindows(plPlotterPtr->display,
                                 top, colormap_windows, count))
      fprintf(stderr, "Unable to set color map property!\n");
#endif
}

/*
 *---------------------------------------------------------------------------
 *
 * DisplayPlPlotter --
 *
 *      This procedure is invoked to display a plframe widget.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Commands are output to X to display the plframe in its
 *      current mode.
 *
 *---------------------------------------------------------------------------
 */

static void
DisplayPlPlotter(ClientData clientData)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;
    register Tk_Window tkwin = plPlotterPtr->tkwin;

    dbug_enter("DisplayPlPlotter");

/* Update scrollbars if needed */

    if (plPlotterPtr->flags & UPDATE_V_SCROLLBAR) {
        UpdateVScrollbar(plPlotterPtr);
    }
    if (plPlotterPtr->flags & UPDATE_H_SCROLLBAR) {
        UpdateHScrollbar(plPlotterPtr);
    }
    plPlotterPtr->flags &= ~(UPDATE_V_SCROLLBAR|UPDATE_H_SCROLLBAR);

/* If not mapped yet, just return and cancel pending refresh */

    if ((plPlotterPtr->tkwin == NULL) || ! Tk_IsMapped(tkwin)) {
        plPlotterPtr->flags &= ~REFRESH_PENDING;
        return;
    }

/* All refresh events */

    if (plPlotterPtr->flags & REFRESH_PENDING) {
        plPlotterPtr->flags &= ~REFRESH_PENDING;

    /* Reschedule resizes to avoid occasional ordering conflicts with */
    /* the packer's resize of the window (this call must come last). */

        if (plPlotterPtr->flags & RESIZE_PENDING) {
            plPlotterPtr->flags |= REFRESH_PENDING;
            plPlotterPtr->flags &= ~RESIZE_PENDING;
            Tcl_DoWhenIdle(DisplayPlPlotter, clientData);
            return;
        }

    /* Redraw border if necessary */

	if ((plPlotterPtr->border != NULL) &&
	    (plPlotterPtr->relief != TK_RELIEF_FLAT)) {
	    Tk_Draw3DRectangle(plPlotterPtr->tkwin, Tk_WindowId(tkwin),
			       plPlotterPtr->border, 0, 0, Tk_Width(tkwin), Tk_Height(tkwin),
			       plPlotterPtr->borderWidth, plPlotterPtr->relief);
	}

    /* Redraw -- replay contents of plot buffer */

        if (plPlotterPtr->flags & REDRAW_PENDING) {
            plPlotterPtr->flags &= ~REDRAW_PENDING;
            plsstrm(plPlotterPtr->ipls);
            pl_cmd(PLESC_REDRAW, (void *) NULL);
        }

    /* Resize -- if window bounds have changed */

        else if ((plPlotterPtr->width != plPlotterPtr->prevWidth) ||
                 (plPlotterPtr->height != plPlotterPtr->prevHeight)) {

            plPlotterPtr->pldis.width = plPlotterPtr->width;
            plPlotterPtr->pldis.height = plPlotterPtr->height;

            plsstrm(plPlotterPtr->ipls);
            pl_cmd(PLESC_RESIZE, (void *) &(plPlotterPtr->pldis));
            plPlotterPtr->prevWidth = plPlotterPtr->width;
            plPlotterPtr->prevHeight = plPlotterPtr->height;
        }

    /* Expose -- if window bounds are unchanged */

        else {
            if (plPlotterPtr->drawing_xhairs) {
                #ifdef MAC_TCL
                /* For MacTk we just zap the whole window like this */
                Tk_Draw3DRectangle(plPlotterPtr->tkwin, Tk_WindowId(tkwin),
                        plPlotterPtr->border, 0, 0, Tk_Width(tkwin), Tk_Height(tkwin),
                        plPlotterPtr->borderWidth, plPlotterPtr->relief);
                #else
                XClearWindow(plPlotterPtr->display, Tk_WindowId(tkwin));
                #endif
                XFlush(plPlotterPtr->display);
		plsstrm(plPlotterPtr->ipls);
                pl_cmd(PLESC_EXPOSE, NULL);
            }
            else {
		plsstrm(plPlotterPtr->ipls);
                pl_cmd(PLESC_EXPOSE, (void *) &(plPlotterPtr->pldis));
            }

        /* Reset window bounds so that next time they are set fresh */

            plPlotterPtr->pldis.x      = Tk_X(tkwin) + Tk_Width(tkwin);
            plPlotterPtr->pldis.y      = Tk_Y(tkwin) + Tk_Height(tkwin);
            plPlotterPtr->pldis.width  = -Tk_Width(tkwin);
            plPlotterPtr->pldis.height = -Tk_Height(tkwin);
        }

    /* Update graphic crosshairs if necessary */

        if (plPlotterPtr->drawing_xhairs) {
            UpdateXhairs(plPlotterPtr);
        }

    /* Update rubber band if necessary. */

        if (plPlotterPtr->drawing_rband) {
            UpdateRband(plPlotterPtr);
        }
    }
}

/*--------------------------------------------------------------------------*\
 * Routines to process widget commands.
\*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*\
 * scol0
 *
 * Sets a color in cmap0.
\*--------------------------------------------------------------------------*/

static int
scol0(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
      int i, CONST char *col, int *p_changed)
{
    PLStream *pls = plPlotterPtr->pls;
    XColor xcol;
    PLINT r, g, b;

    if ( col == NULL ) {
        Tcl_AppendResult(interp, "color value not specified",
                         (char *) NULL);
        return TCL_ERROR;
    }

    if ( ! XParseColor(plPlotterPtr->display,
                       Tk_Colormap(plPlotterPtr->tkwin), col, &xcol)) {
        Tcl_AppendResult(interp, "Couldn't parse color ", col,
                         (char *) NULL);
        return TCL_ERROR;
    }

    r = (unsigned) (xcol.red   & 0xFF00) >> 8;
    g = (unsigned) (xcol.green & 0xFF00) >> 8;
    b = (unsigned) (xcol.blue  & 0xFF00) >> 8;

    if ( (pls->cmap0[i].r != r) ||
         (pls->cmap0[i].g != g) ||
         (pls->cmap0[i].b != b) ) {

        pls->cmap0[i].r = r;
        pls->cmap0[i].g = g;
        pls->cmap0[i].b = b;
        *p_changed = 1;
    }

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * scol1
 *
 * Sets a color in cmap1.
\*--------------------------------------------------------------------------*/

static int
scol1(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
      int i, CONST char *col, CONST char *pos, CONST char *rev, int *p_changed)
{
    PLStream *pls = plPlotterPtr->pls;
    XColor xcol;
    PLFLT h, l, s, r, g, b, p;
    int reverse;

    if ( col == NULL ) {
        Tcl_AppendResult(interp, "color value not specified",
                         (char *) NULL);
        return TCL_ERROR;
    }

    if ( pos == NULL ) {
        Tcl_AppendResult(interp, "control point position not specified",
                         (char *) NULL);
        return TCL_ERROR;
    }

    if ( rev == NULL ) {
        Tcl_AppendResult(interp, "interpolation sense not specified",
                         (char *) NULL);
        return TCL_ERROR;
    }

    if ( ! XParseColor(plPlotterPtr->display,
                       Tk_Colormap(plPlotterPtr->tkwin), col, &xcol)) {
        Tcl_AppendResult(interp, "Couldn't parse color ", col,
                         (char *) NULL);
        return TCL_ERROR;
    }

    r = (PLFLT)(((unsigned) (xcol.red   & 0xFF00) >> 8) / 255.0);
    g = (PLFLT)(((unsigned) (xcol.green & 0xFF00) >> 8) / 255.0);
    b = (PLFLT)(((unsigned) (xcol.blue  & 0xFF00) >> 8) / 255.0);

    plrgbhls(r, g, b, &h, &l, &s);

    p = (PLFLT) (atof(pos) / 100.0);
    reverse = atoi(rev);

    if ( (pls->cmap1cp[i].h != h) ||
         (pls->cmap1cp[i].l != l) ||
         (pls->cmap1cp[i].s != s) ||
         (pls->cmap1cp[i].p != p) ||
         (pls->cmap1cp[i].rev != reverse) ) {

        pls->cmap1cp[i].h = h;
        pls->cmap1cp[i].l = l;
        pls->cmap1cp[i].s = s;
        pls->cmap1cp[i].p = p;
        pls->cmap1cp[i].rev = reverse;
        *p_changed = 1;
    }
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * Cmd
 *
 * Processes "cmd" widget command.
 * Handles commands that go more or less directly to the PLplot library.
 * Most of these come out of the PLplot Tcl API support file.
\*--------------------------------------------------------------------------*/

static int
Cmd(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
    int argc, CONST char **argv)
{
    PLStream *pls = plPlotterPtr->pls;
    int length;
    char c3;
    int result = TCL_OK;
    char cmdlist[] = "plgcmap0 plgcmap1 plscmap0 plscmap1 plscol0 plscol1";

#ifdef DEBUG
    if (pls->debug) {
        int i;
        fprintf(stderr, "There are %d arguments to Cmd:", argc);
        for (i = 0; i < argc; i++) {
            fprintf(stderr, " %s", argv[i]);
        }
        fprintf(stderr, "\n");
    }
#endif

/* no option -- return list of available PLplot commands */

    if (argc == 0)
        return plTclCmd(cmdlist, interp, argc, (char **) argv);

/* Make sure widget has been initialized before going any further */

    if ( ! plPlotterPtr->tkwin_initted) {
        Tcl_VarEval(plPlotterPtr->interp, "update", (char *) NULL);
    }

/* Set stream number and get ready to process the command */

    plsstrm(plPlotterPtr->ipls);

    c3 = argv[0][2];
    length = strlen(argv[0]);

/* plgcmap0 -- get color map 0 */
/* first arg is number of colors, the rest are hex number specifications */

    if ((c3 == 'g') && (strncmp(argv[0], "plgcmap0", length) == 0)) {
        int i;
        unsigned long plcolor;
        char str[10];

        sprintf(str, "%d", (int) pls->ncol0);
        Tcl_AppendElement(interp, str);
        for (i = 0; i < pls->ncol0; i++) {
            plcolor = ((pls->cmap0[i].r << 16) |
                       (pls->cmap0[i].g << 8) |
                       (pls->cmap0[i].b));

            sprintf(str, "#%06lx", (plcolor & 0xFFFFFF));
            Tcl_AppendElement(interp, str);
        }
        result = TCL_OK;
    }

/* plgcmap1 -- get color map 1 */
/* first arg is number of control points */
/* the rest are hex number specifications followed by positions (0-100) */

    else if ((c3 == 'g') && (strncmp(argv[0], "plgcmap1", length) == 0)) {
        int i;
        unsigned long plcolor;
        char str[10];
        PLFLT h, l, s, r, g, b;
        int r1, g1, b1;

        sprintf(str, "%d", (int) pls->ncp1);
        Tcl_AppendElement(interp, str);
        for (i = 0; i < pls->ncp1; i++) {
            h = pls->cmap1cp[i].h;
            l = pls->cmap1cp[i].l;
            s = pls->cmap1cp[i].s;

            plhlsrgb(h, l, s, &r, &g, &b);

            r1 = MAX(0, MIN(255, (int) (256. * r)));
            g1 = MAX(0, MIN(255, (int) (256. * g)));
            b1 = MAX(0, MIN(255, (int) (256. * b)));

            plcolor = ((r1 << 16) | (g1 << 8) | (b1));

            sprintf(str, "#%06lx", (plcolor & 0xFFFFFF));
            Tcl_AppendElement(interp, str);

            sprintf(str, "%02d", (int) (100*pls->cmap1cp[i].p));
            Tcl_AppendElement(interp, str);

            sprintf(str, "%01d", (int) (pls->cmap1cp[i].rev));
            Tcl_AppendElement(interp, str);
        }
        result = TCL_OK;
    }

/* plscmap0 -- set color map 0 */
/* first arg is number of colors, the rest are hex number specifications */

    else if ((c3 == 's') && (strncmp(argv[0], "plscmap0", length) == 0)) {
        int i, changed = 1, ncol0 = atoi(argv[1]);
        char *col;

        if (ncol0 > 16 || ncol0 < 1) {
            Tcl_AppendResult(interp, "illegal number of colors in cmap0: ",
                             argv[1], (char *) NULL);
            return TCL_ERROR;
        }

        pls->ncol0 = ncol0;
        for (i = 0; i < pls->ncol0; i++) {
            col = strtok((char *) argv[2+i], " ");
            if ( col == NULL )
                break;

            if (scol0(interp, plPlotterPtr, i, col, &changed) != TCL_OK)
                return TCL_ERROR;

        }

        if (changed)
            plP_state(PLSTATE_CMAP0);
    }

/* plscmap1 -- set color map 1 */
/* first arg is number of colors, the rest are hex number specifications */

    else if ((c3 == 's') && (strncmp(argv[0], "plscmap1", length) == 0)) {
        int i, changed = 1, ncp1 = atoi(argv[1]);
        char *col, *pos, *rev;

        if (ncp1 > 32 || ncp1 < 1) {
            Tcl_AppendResult(interp,
                             "illegal number of control points in cmap1: ",
                             argv[1], (char *) NULL);
            return TCL_ERROR;
        }

        col = strtok((char *) argv[2], " ");
        pos = strtok(NULL, " ");
        rev = strtok(NULL, " ");
        for (i = 0; i < ncp1; i++) {
            if ( col == NULL )
                break;

            if (scol1(interp, plPlotterPtr,
                      i, col, pos, rev, &changed) != TCL_OK)
                return TCL_ERROR;

            col = strtok(NULL, " ");
            pos = strtok(NULL, " ");
            rev = strtok(NULL, " ");
        }

        if (changed) {
            plsc->ncp1 = ncp1;
            plcmap1_calc();
        }
    }

/* plscol0 -- set single color in cmap0 */
/* first arg is the color number, the next is the color in hex */

    else if ((c3 == 's') && (strncmp(argv[0], "plscol0", length) == 0)) {
        int i = atoi(argv[1]), changed = 1;

        if (i > pls->ncol0 || i < 0) {
            Tcl_AppendResult(interp, "illegal color number in cmap0: ",
                             argv[1], (char *) NULL);
            return TCL_ERROR;
        }

        if (scol0(interp, plPlotterPtr, i, argv[2], &changed) != TCL_OK)
            return TCL_ERROR;

        if (changed)
            plP_state(PLSTATE_CMAP0);
    }

/* plscol1 -- set color of control point in cmap1 */
/* first arg is the control point, the next two are the color in hex and pos */

    else if ((c3 == 's') && (strncmp(argv[0], "plscol1", length) == 0)) {
        int i = atoi(argv[1]), changed = 1;

        if (i > pls->ncp1 || i < 0) {
            Tcl_AppendResult(interp, "illegal control point number in cmap1: ",
                             argv[1], (char *) NULL);
            return TCL_ERROR;
        }

        if (scol1(interp, plPlotterPtr,
                  i, argv[2], argv[3], argv[4], &changed) != TCL_OK)
            return TCL_ERROR;

        if (changed)
            plcmap1_calc();
    }

/* Added by Vince, disabled by default since we want a minimal patch */
#ifdef USING_PLESC_COPY
/* plcopy -- copy a region of the plot; useful for scrolling plots */
/* first 4 args are the source rectangle, next 2 args are the destination */

    else if ((c3 == 'c') && (strncmp(argv[0], "plcopy", length) == 0)) {
	PLFLT xx[3], yy[3];
	if (argc != 7) {
	    Tcl_AppendResult(interp, "Need exactly 6 arguments to copy.",
			     (char *) NULL);
	    return TCL_ERROR;
	}
	xx[0] = atof(argv[1]);
	yy[0] = atof(argv[2]);
	xx[1] = atof(argv[3]);
	yy[1] = atof(argv[4]);
	xx[2] = atof(argv[5]);
	yy[2] = atof(argv[6]);
	plcopy(xx,yy);
    }
#endif

/* unrecognized, so give it to plTclCmd to take care of */

    else
        result = plTclCmd(cmdlist, interp, argc, (char **)argv);

    plflush();
    return result;
}

static void ActiveState(register PlPlotter *plPlotterPtr, int on) {
    if(on ) {
        if(!plPlotterPtr->isActive) {
            Tk_CreateEventHandler(plPlotterPtr->tkwin, ButtonPressMask,
                                  PlPlotterButtonPressEH, (ClientData) plPlotterPtr);

            Tk_CreateEventHandler(plPlotterPtr->tkwin, PointerMotionMask,
                                  PlPlotterMotionEH, (ClientData) plPlotterPtr);

            Tk_CreateEventHandler(plPlotterPtr->tkwin, EnterWindowMask,
                                  PlPlotterEnterEH, (ClientData) plPlotterPtr);

            Tk_CreateEventHandler(plPlotterPtr->tkwin, LeaveWindowMask,
                                  PlPlotterLeaveEH, (ClientData) plPlotterPtr);
        /* Switch to crosshair cursor. */

            Tk_DefineCursor(plPlotterPtr->tkwin, plPlotterPtr->xhair_cursor);
        }
    } else {
        if(plPlotterPtr->isActive) {
            Tk_DeleteEventHandler(plPlotterPtr->tkwin, ButtonPressMask,
                                  PlPlotterButtonPressEH, (ClientData) plPlotterPtr);
            Tk_DeleteEventHandler(plPlotterPtr->tkwin, PointerMotionMask,
                                  PlPlotterMotionEH, (ClientData) plPlotterPtr);

            Tk_DeleteEventHandler(plPlotterPtr->tkwin, EnterWindowMask,
                                  PlPlotterEnterEH, (ClientData) plPlotterPtr);

            Tk_DeleteEventHandler(plPlotterPtr->tkwin, LeaveWindowMask,
                                  PlPlotterLeaveEH, (ClientData) plPlotterPtr);
        /* Switch back to boring old pointer */

            Tk_DefineCursor(plPlotterPtr->tkwin, plPlotterPtr->cursor);
        }
    }
}


/*
 *---------------------------------------------------------------------------
 *
 * ConfigurePlPlotter --
 *
 *      This procedure is called to process an argv/argc list, plus the Tk
 *      option database, in order to configure (or reconfigure) a
 *      plframe widget.
 *
 * Results:
 *      The return value is a standard Tcl result.  If TCL_ERROR is
 *      returned, then interp->result contains an error message.
 *
 * Side effects:
 *      Configuration information, such as text string, colors, font, etc.
 *      get set for plPlotterPtr; old resources get freed, if there were
 *      any.
 *
 *---------------------------------------------------------------------------
 */

static int
ConfigurePlPlotter(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
                 int argc, CONST char **argv, int flags)
{
    register Tk_Window tkwin = plPlotterPtr->tkwin;
    PLStream *pls = plPlotterPtr->pls;
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;
    XGCValues gcValues;
    unsigned long mask;
    int need_redisplay = 0;

#ifdef DEBUG
    if (pls->debug) {
        int i;
        fprintf(stderr, "Arguments to configure are:");
        for (i = 0; i < argc; i++) {
            fprintf(stderr, " %s", argv[i]);
        }
        fprintf(stderr, "\n");
    }
#endif

    dbug_enter("ConfigurePlPlotter");

    if (Tk_ConfigureWidget(interp, tkwin, configSpecs,
            argc, argv, (char *) plPlotterPtr, flags) != TCL_OK) {
        return TCL_ERROR;
    }

/*
 * Set background color using tkwin driver's pixel value.  Done this way so
 * that (a) we can use r/w color cells, and (b) the BG pixel values as set
 * here and in the tkwin driver are consistent.
 */

    plsstrm(plPlotterPtr->ipls);
    if(PLColor_from_TkColor_Changed(&pls->cmap0[0],
      Tk_3DBorderColor(plPlotterPtr->border))) {
    	need_redisplay = 1;
    	/* need to redraw as well as simply refresh the window */
        plPlotterPtr->flags |= REDRAW_PENDING;
    }
    pltkwin_setBGFG(pls);

    Tk_SetWindowBackground(tkwin, tkwd->cmap0[0].pixel);
    Tk_SetWindowBorder(tkwin, tkwd->cmap0[0].pixel);

    /* Set up GC for rubber-band draws */

    gcValues.background = tkwd->cmap0[0].pixel;
    gcValues.foreground = 0xFF;
    gcValues.function = GXxor;
    mask = GCForeground | GCBackground | GCFunction;

    if (plPlotterPtr->xorGC != NULL)
        Tk_FreeGC(plPlotterPtr->display, plPlotterPtr->xorGC);

    plPlotterPtr->xorGC = Tk_GetGC(plPlotterPtr->tkwin, mask, &gcValues);

/* Geometry settings */

    Tk_SetInternalBorder(tkwin, plPlotterPtr->borderWidth);
    if ((plPlotterPtr->width > 0) || (plPlotterPtr->height > 0)) {
        Tk_GeometryRequest(tkwin, plPlotterPtr->width, plPlotterPtr->height);
        if ((plPlotterPtr->width != plPlotterPtr->prevWidth) ||
            (plPlotterPtr->height != plPlotterPtr->prevHeight))
            need_redisplay = 1;
    }

/* Create or destroy graphic crosshairs as specified */

    if (Tk_IsMapped(tkwin)) {
        if (plPlotterPtr->xhairs) {
            if (! plPlotterPtr->drawing_xhairs)
                CreateXhairs(plPlotterPtr);
        }
        else {
            if (plPlotterPtr->drawing_xhairs)
                DestroyXhairs(plPlotterPtr);
        }
    }

/* Create or destroy rubber band as specified */

    if (Tk_IsMapped(tkwin)) {
        if (plPlotterPtr->rband) {
            if (! plPlotterPtr->drawing_rband)
                CreateRband(plPlotterPtr);
        }
        else {
            if (plPlotterPtr->drawing_rband)
                DestroyRband(plPlotterPtr);
        }
    }
/* Sets or clears events for the plot */
    ActiveState(plPlotterPtr,plPlotterPtr->active_plot);

    if (!pls->nopixmap) {
	/* can only adjust if this flag not set */
	if(plPlotterPtr->double_buffer != pls->db) {
	    pls->db = plPlotterPtr->double_buffer;
	    /* turn on/off dbl-buffering in the driver */
	    dev->write_to_window = !pls->db;
	}
    } else {
	plPlotterPtr->double_buffer = 0;
    }

/* Arrange for window to be refreshed if necessary */
    if (need_redisplay && Tk_IsMapped(tkwin)
            && !(plPlotterPtr->flags & REFRESH_PENDING)) {
        Tcl_DoWhenIdle(DisplayPlPlotter, (ClientData) plPlotterPtr);
        plPlotterPtr->flags |= REFRESH_PENDING;
        plPlotterPtr->flags |= UPDATE_V_SCROLLBAR|UPDATE_H_SCROLLBAR;
    }

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * Draw
 *
 * Processes "draw" widget command.
 * Handles rubber-band drawing.
\*--------------------------------------------------------------------------*/

static int
Draw(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
     int argc, CONST char **argv)
{
    register Tk_Window tkwin = plPlotterPtr->tkwin;
    int result = TCL_OK;
    char c = argv[0][0];
    int length = strlen(argv[0]);

/* Make sure widget has been initialized before going any further */

    if ( ! plPlotterPtr->tkwin_initted) {
        Tcl_VarEval(plPlotterPtr->interp, "update", (char *) NULL);
    }

/* init -- sets up for rubber-band drawing */

    if ((c == 'i') && (strncmp(argv[0], "init", length) == 0)) {
        Tk_DefineCursor(tkwin, plPlotterPtr->xhair_cursor);
    }

/* end -- ends rubber-band drawing */

    else if ((c == 'e') && (strncmp(argv[0], "end", length) == 0)) {

        Tk_DefineCursor(tkwin, plPlotterPtr->cursor);
        if (plPlotterPtr->continue_draw) {
            XDrawLines(Tk_Display(tkwin), Tk_WindowId(tkwin),
                       plPlotterPtr->xorGC, plPlotterPtr->pts, 5,
                       CoordModeOrigin);
            XSync(Tk_Display(tkwin), 0);
        }

        plPlotterPtr->continue_draw = 0;
    }

/* rect -- draw a rectangle, used to select rectangular areas */
/* first draw erases old outline */

    else if ((c == 'r') && (strncmp(argv[0], "rect", length) == 0)) {
        if (argc < 5) {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
                             " draw rect x0 y0 x1 y1\"", (char *) NULL);
            result = TCL_ERROR;
        }
        else {
            int x0, y0, x1, y1;
            int xmin = 0, xmax = Tk_Width(tkwin) - 1;
            int ymin = 0, ymax = Tk_Height(tkwin) - 1;

            x0 = atoi(argv[1]);
            y0 = atoi(argv[2]);
            x1 = atoi(argv[3]);
            y1 = atoi(argv[4]);

            x0 = MAX(xmin, MIN(xmax, x0));
            y0 = MAX(ymin, MIN(ymax, y0));
            x1 = MAX(xmin, MIN(xmax, x1));
            y1 = MAX(ymin, MIN(ymax, y1));

            if (plPlotterPtr->continue_draw) {
                XDrawLines(Tk_Display(tkwin), Tk_WindowId(tkwin),
                           plPlotterPtr->xorGC, plPlotterPtr->pts, 5,
                           CoordModeOrigin);
                XSync(Tk_Display(tkwin), 0);
            }

            plPlotterPtr->pts[0].x = x0; plPlotterPtr->pts[0].y = y0;
            plPlotterPtr->pts[1].x = x1; plPlotterPtr->pts[1].y = y0;
            plPlotterPtr->pts[2].x = x1; plPlotterPtr->pts[2].y = y1;
            plPlotterPtr->pts[3].x = x0; plPlotterPtr->pts[3].y = y1;
            plPlotterPtr->pts[4].x = x0; plPlotterPtr->pts[4].y = y0;

            XDrawLines(Tk_Display(tkwin), Tk_WindowId(tkwin),
                       plPlotterPtr->xorGC, plPlotterPtr->pts, 5,
                       CoordModeOrigin);
            XSync(Tk_Display(tkwin), 0);

            plPlotterPtr->continue_draw = 1;
        }
    }

    return result;
}

/*--------------------------------------------------------------------------*\
 * Info
 *
 * Processes "info" widget command.
 * Returns requested info.
\*--------------------------------------------------------------------------*/

static int
Info(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
     int argc, CONST char **argv)
{
    int length;
    char c;
    int result = TCL_OK;

/* no option -- return list of available info commands */

    if (argc == 0) {
        Tcl_SetResult(interp, "devkeys devnames", TCL_STATIC);
        return TCL_OK;
    }

    c = argv[0][0];
    length = strlen(argv[0]);

/* devkeys -- return list of supported device keywords */

    if ((c == 'd') && (strncmp(argv[0], "devkeys", length) == 0)) {
        int i = 0;
        while (plPlotterPtr->devName[i] != NULL)
            Tcl_AppendElement(interp, plPlotterPtr->devName[i++]);

        result = TCL_OK;
    }

/* devkeys -- return list of supported device types */

    else if ((c == 'd') && (strncmp(argv[0], "devnames", length) == 0)) {
        int i = 0;
        while (plPlotterPtr->devDesc[i] != NULL)
            Tcl_AppendElement(interp, plPlotterPtr->devDesc[i++]);

        result = TCL_OK;
    }

/* unrecognized */

    else {
        Tcl_AppendResult(interp, "bad option to \"info\": must be ",
         "devkeys, devnames", (char *) NULL);

        result = TCL_ERROR;
    }

    return result;
}

/*--------------------------------------------------------------------------*\
 * Openlink
 *
 * Processes "openlink" widget command.
 * Opens channel (FIFO or socket) for binary data transfer between client
 * and server.
\*--------------------------------------------------------------------------*/

static int
Openlink(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
         int argc, CONST char **argv)
{
#if !defined(MAC_TCL) && !defined(__WIN32__)
    register PLRDev *plr = plPlotterPtr->plr;
    register PLiodev *iodev = plr->iodev;

    char c = argv[0][0];
    int length = strlen(argv[0]);

    dbug_enter("Openlink");

/* Open fifo */

    if ((c == 'f') && (strncmp(argv[0], "fifo", length) == 0)) {

        if (argc < 1) {
            Tcl_AppendResult(interp, "bad command -- must be: ",
                             "openlink fifo <pathname>",
                             (char *) NULL);
            return TCL_ERROR;
        }
        if ((iodev->fd = open (argv[1], O_RDONLY)) == -1) {
            Tcl_AppendResult(interp, "cannot open fifo ", argv[1],
                             " for read", (char *) NULL);
            return TCL_ERROR;
        }
        iodev->type = 0;
        iodev->typeName = "fifo";
        iodev->file = fdopen(iodev->fd, "rb");
    }

/* Open socket */

    else if ((c == 's') && (strncmp(argv[0], "socket", length) == 0)) {

        if (argc < 1) {
            Tcl_AppendResult(interp, "bad command -- must be: ",
                             "openlink socket <sock-id>",
                             (char *) NULL);
            return TCL_ERROR;
        }
        iodev->type = 1;
        iodev->typeName = "socket";
        iodev->fileHandle = (char *) argv[1];

        if (Tcl_GetOpenFile(interp, iodev->fileHandle,
                            0, 1, (ClientData) &iodev->file) != TCL_OK) {
            return TCL_ERROR;
        }
        iodev->fd = fileno(iodev->file);
    }

/* unrecognized */

    else {
        Tcl_AppendResult(interp, "bad option to \"openlink\": must be ",
         "fifo or socket", (char *) NULL);

        return TCL_ERROR;
    }

    plr->pdfs = pdf_bopen( NULL, 4200 );
    Tcl_CreateFileHandler( iodev->fd,
                           TK_READABLE, (Tk_FileProc *) ReadData,
                           (ClientData) plPlotterPtr );

#endif

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * Closelink
 *
 * Processes "closelink" widget command.
 * CLoses channel previously opened with the "openlink" widget command.
\*--------------------------------------------------------------------------*/

static int
Closelink(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
         int argc, CONST char **argv)
{
#if !defined(MAC_TCL) && !defined(__WIN32__)
    register PLRDev *plr = plPlotterPtr->plr;
    register PLiodev *iodev = plr->iodev;

    dbug_enter("Closelink");

    if (iodev->fd == 0) {
        Tcl_AppendResult(interp, "no link currently open", (char *) NULL);
        return TCL_ERROR;
    }

    Tcl_DeleteFileHandler(iodev->fd);
    pdf_close(plr->pdfs);
    iodev->fd = 0;
#endif
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * process_data
 *
 * Utility function for processing data and other housekeeping.
\*--------------------------------------------------------------------------*/

static int
process_data(Tcl_Interp *interp, register PlPlotter *plPlotterPtr)
{
    register PLRDev *plr = plPlotterPtr->plr;
    register PLiodev *iodev = plr->iodev;
    int result = TCL_OK;

/* Process data */

    if (plr_process(plr) == -1) {
        Tcl_AppendResult(interp, "unable to read from ", iodev->typeName,
                         (char *) NULL);
        result = TCL_ERROR;
    }

/* Signal bop if necessary */

    if (plr->at_bop && plPlotterPtr->bopCmd != NULL) {
        plr->at_bop = 0;
        if (Tcl_Eval(interp, plPlotterPtr->bopCmd) != TCL_OK)
            fprintf(stderr, "Command \"%s\" failed:\n\t %s\n",
                    plPlotterPtr->bopCmd, interp->result);
    }

/* Signal eop if necessary */

    if (plr->at_eop && plPlotterPtr->eopCmd != NULL) {
        plr->at_eop = 0;
        if (Tcl_Eval(interp, plPlotterPtr->eopCmd) != TCL_OK)
            fprintf(stderr, "Command \"%s\" failed:\n\t %s\n",
                    plPlotterPtr->eopCmd, interp->result);
    }

    return result;
}

void PlplotterAtEop(Tcl_Interp *interp, register PlPlotter *plPlotterPtr) {
    if (plPlotterPtr->eopCmd != NULL) {
        if (Tcl_Eval(interp, plPlotterPtr->eopCmd) != TCL_OK)
            fprintf(stderr, "Command \"%s\" failed:\n\t %s\n",
                    plPlotterPtr->eopCmd, interp->result);
    }
}

void PlplotterAtBop(Tcl_Interp *interp, register PlPlotter *plPlotterPtr) {
    if (plPlotterPtr->bopCmd != NULL) {
        if (Tcl_Eval(interp, plPlotterPtr->bopCmd) != TCL_OK)
            fprintf(stderr, "Command \"%s\" failed:\n\t %s\n",
                    plPlotterPtr->bopCmd, interp->result);
    }
}

/*--------------------------------------------------------------------------*\
 * ReadData
 *
 * Reads & processes data.
 * Intended to be installed as a filehandler command.
\*--------------------------------------------------------------------------*/

static int
ReadData(ClientData clientData, int mask)
{
    register PlPlotter *plPlotterPtr = (PlPlotter *) clientData;
    register Tcl_Interp *interp = plPlotterPtr->interp;

    register PLRDev *plr = plPlotterPtr->plr;
    register PLiodev *iodev = plr->iodev;
    register PDFstrm *pdfs = plr->pdfs;
    int result = TCL_OK;

    if (mask & TK_READABLE) {

    /* Read from FIFO or socket */

        plsstrm(plPlotterPtr->ipls);
        #ifndef MAC_TCL
        if (pl_PacketReceive(interp, iodev, pdfs)) {
        #else
        if (1) {
        #endif
            Tcl_AppendResult(interp, "Packet receive failed:\n\t %s\n",
                             interp->result, (char *) NULL);
            return TCL_ERROR;
        }

    /* If the packet isn't complete it will be put back and we just return.
     * Otherwise, the buffer pointer is saved and then cleared so that reads
     * from the buffer start at the beginning.
     */
        if (pdfs->bp == 0)
            return TCL_OK;

        plr->nbytes = pdfs->bp;
        pdfs->bp = 0;
        result = process_data(interp, plPlotterPtr);
    }

    return result;
}

/*--------------------------------------------------------------------------*\
 * Orient
 *
 * Processes "orient" widget command.
 * Handles orientation of plot.
\*--------------------------------------------------------------------------*/

static int
Orient(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
       int argc, CONST char **argv)
{
    int result = TCL_OK;

/* orient -- return orientation of current plot window */

    plsstrm(plPlotterPtr->ipls);

    if (argc == 0) {
        PLFLT rot;
        char result_str[128];
        plgdiori(&rot);
        sprintf(result_str, "%f", rot);
        Tcl_SetResult(interp, result_str, TCL_VOLATILE);
    }

/* orient <rot> -- Set orientation to <rot> */

    else {
        plsdiori((PLFLT)atof(argv[0]));
        result = Redraw(interp, plPlotterPtr, argc-1, argv+1);
    }

    return result;
}

/*--------------------------------------------------------------------------*\
 * Print
 *
 * Processes "print" widget command.
 * Handles printing of plot, duh.
 *
 * Creates a temporary file, dumps the current plot to it in metafile
 * form, and then execs the "plpr" script to actually print it.  Since we
 * output it in metafile form here, plpr must invoke plrender to drive the
 * output to the appropriate file type.  The script is responsible for the
 * deletion of the plot metafile.
\*--------------------------------------------------------------------------*/

static int
Print(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
       int argc, CONST char **argv)
{
    PLINT ipls;
    int result = TCL_OK;
    char *sfnam;
    FILE *sfile;
#if !defined(MAC_TCL) && !defined(__WIN32__)
    pid_t pid;
#endif

/* Make sure widget has been initialized before going any further */

    if ( ! plPlotterPtr->tkwin_initted) {
        Tcl_AppendResult(interp, "Error -- widget not plotted to yet",
                         (char *) NULL);
        return TCL_ERROR;
    }

/* Create stream for save */

    plmkstrm(&ipls);
    if (ipls < 0) {
        Tcl_AppendResult(interp, "Error -- cannot create stream",
                         (char *) NULL);
        return TCL_ERROR;
    }

/* Open file for writes */

    sfnam = (char *) tmpnam(NULL);

    if ((sfile = fopen(sfnam, "wb+")) == NULL) {
        Tcl_AppendResult(interp,
                         "Error -- cannot open plot file for writing",
                         (char *) NULL);
        plend1();
        return TCL_ERROR;
    }

/* Initialize stream */

    plsdev("plmeta");
    plsfile(sfile);
    plcpstrm(plPlotterPtr->ipls, 0);
    pladv(0);

/* Remake current plot, close file, and switch back to original stream */

    plreplot();
    plend1();
    plsstrm(plPlotterPtr->ipls);

/* So far so good.  Time to exec the print script. */

    if (plPlotterPtr->plpr_cmd == NULL)
	plPlotterPtr->plpr_cmd = plFindCommand("plpr");

#if !defined(MAC_TCL) && !defined(__WIN32__)
    if ((plPlotterPtr->plpr_cmd == NULL) || (pid = fork()) < 0) {
	Tcl_AppendResult(interp,
			 "Error -- cannot fork print process",
			 (char *) NULL);
	result = TCL_ERROR;
    } else if (pid == 0) {
	if (execl(plPlotterPtr->plpr_cmd, plPlotterPtr->plpr_cmd, sfnam,
		  (char *) 0)) {
	    fprintf(stderr, "Unable to exec print command.\n");
	    _exit(1);
	}
    }
#endif
    return result;
}

/*--------------------------------------------------------------------------*\
 * NextPage
 *
 * Tells the tkwin driver to move along to the next page.
\*--------------------------------------------------------------------------*/

static int
NextPage(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
     int argc, CONST char **argv)
{
    TkwDev *dev = (TkwDev *) plPlotterPtr->pls->dev;
    if(argc == 0) {
    	dev->flags |= 2;
    } else {
        Tcl_AppendResult(interp, "wrong # args: should be \"",
                         " nextpage\"", (char *) NULL);
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * Page
 *
 * Processes "page" widget command.
 * Handles parameters such as margin, aspect ratio, and justification
 * of final plot.
\*--------------------------------------------------------------------------*/

static int
Page(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
     int argc, CONST char **argv)
{

/* page -- return current device window parameters */

    plsstrm(plPlotterPtr->ipls);

    if (argc == 0) {
        PLFLT mar, aspect, jx, jy;
        char result_str[128];

        plgdidev(&mar, &aspect, &jx, &jy);
        sprintf(result_str, "%g %g %g %g", mar, aspect, jx, jy);
        Tcl_SetResult(interp, result_str, TCL_VOLATILE);
        return TCL_OK;
    }

/* page <mar> <aspect> <jx> <jy> -- set up page */

    if (argc < 4) {
        Tcl_AppendResult(interp, "wrong # args: should be \"",
                         " page mar aspect jx jy\"", (char *) NULL);
        return TCL_ERROR;
    }

    plsdidev((PLFLT)atof(argv[0]), (PLFLT)atof(argv[1]),
	     (PLFLT)atof(argv[2]), (PLFLT)atof(argv[3]));
    return (Redraw(interp, plPlotterPtr, argc-1, argv+1));
}

/*--------------------------------------------------------------------------*\
 * Redraw
 *
 * Processes "redraw" widget command.
 * Turns loose a DoWhenIdle command to redraw plot by replaying contents
 * of plot buffer.
\*--------------------------------------------------------------------------*/

static int
Redraw(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
       int argc, CONST char **argv)
{
    dbug_enter("Redraw");

    plPlotterPtr->flags |= REDRAW_PENDING;
    if ((plPlotterPtr->tkwin != NULL) &&
        !(plPlotterPtr->flags & REFRESH_PENDING)) {

        Tcl_DoWhenIdle(DisplayPlPlotter, (ClientData) plPlotterPtr);
        plPlotterPtr->flags |= REFRESH_PENDING;
    }

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * Save
 *
 * Processes "save" widget command.
 * Saves plot to a file.
\*--------------------------------------------------------------------------*/

static int
Save(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
     int argc, CONST char **argv)
{
    int length;
    char c;
    FILE *sfile;

/* Make sure widget has been initialized before going any further */

    if ( ! plPlotterPtr->tkwin_initted) {
        Tcl_AppendResult(interp, "Error -- widget not plotted to yet",
                         (char *) NULL);
        return TCL_ERROR;
    }

/* save -- save to already open file */

    if (argc == 0) {
        if ( ! plPlotterPtr->ipls_save) {
            Tcl_AppendResult(interp, "Error -- no current save file",
                             (char *) NULL);
            return TCL_ERROR;
        }
        plsstrm(plPlotterPtr->ipls_save);
        plcpstrm(plPlotterPtr->ipls, 0);
        pladv(0);
        plreplot();
        plflush();
        plsstrm(plPlotterPtr->ipls);
        return TCL_OK;
    }

    c = argv[0][0];
    length = strlen(argv[0]);

/* save to specified device & file */

    if ((c == 'a') && (strncmp(argv[0], "as", length) == 0)) {
        if (argc < 3) {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
                             " save as device file\"", (char *) NULL);
            return TCL_ERROR;
        }

    /* If save previously in effect, delete old stream */

        if (plPlotterPtr->ipls_save) {
            plsstrm(plPlotterPtr->ipls_save);
            plend1();
        }

    /* Create stream for saves to selected device & file */

        plmkstrm(&plPlotterPtr->ipls_save);
        if (plPlotterPtr->ipls_save < 0) {
            Tcl_AppendResult(interp, "Error -- cannot create stream",
                             (char *) NULL);
            plPlotterPtr->ipls_save = 0;
            return TCL_ERROR;
        }

    /* Open file for writes */

        if ((sfile = fopen(argv[2], "wb+")) == NULL) {
            Tcl_AppendResult(interp, "Error -- cannot open file ", argv[2],
                             " for writing", (char *) NULL);
            plPlotterPtr->ipls_save = 0;
            plend1();
            return TCL_ERROR;
        }

    /* Initialize stream */

        plsdev(argv[1]);
        plsfile(sfile);
        plcpstrm(plPlotterPtr->ipls, 0);
        pladv(0);

    /* Remake current plot and then switch back to original stream */

        plreplot();
        plflush();
        plsstrm(plPlotterPtr->ipls);
    }

/* close save file */

    else if ((c == 'c') && (strncmp(argv[0], "close", length) == 0)) {

        if ( ! plPlotterPtr->ipls_save) {
            Tcl_AppendResult(interp, "Error -- no current save file",
                             (char *) NULL);
            return TCL_ERROR;
        }
        else {
            plsstrm(plPlotterPtr->ipls_save);
            plend1();
            plPlotterPtr->ipls_save = 0;
        }
    }

/* unrecognized */

    else {
        Tcl_AppendResult(interp, "bad option to \"save\": must be ",
         "as or close", (char *) NULL);

        return TCL_ERROR;
    }

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * View
 *
 * Processes "view" widget command.
 * Handles translation & scaling of view into plot.
\*--------------------------------------------------------------------------*/

static int
View(Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
     int argc, CONST char **argv)
{
    int length;
    int dontRedraw = 0;
    char c;
    PLFLT xl, xr, yl, yr;

/* view -- return current relative plot window coordinates */

    plsstrm(plPlotterPtr->ipls);

    if (argc == 0) {
        char result_str[128];
        plgdiplt(&xl, &yl, &xr, &yr);
        sprintf(result_str, "%g %g %g %g", xl, yl, xr, yr);
        Tcl_SetResult(interp, result_str, TCL_VOLATILE);
        return TCL_OK;
    }

    c = argv[0][0];
    length = strlen(argv[0]);

/* view bounds -- return relative device coordinates of bounds on current */
/* plot window */

    if ((c == 'b') && (strncmp(argv[0], "bounds", length) == 0)) {
        char result_str[128];
        xl = 0.; yl = 0.;
        xr = 1.; yr = 1.;
        pldip2dc(&xl, &yl, &xr, &yr);
        sprintf(result_str, "%g %g %g %g", xl, yl, xr, yr);
        Tcl_SetResult(interp, result_str, TCL_VOLATILE);
        return TCL_OK;
    }

/* view reset -- Resets plot */

    if ((c == 'r') && (strncmp(argv[0], "reset", length) == 0)) {
        xl = 0.; yl = 0.;
        xr = 1.; yr = 1.;
        plsdiplt(xl, yl, xr, yr);
	if (argc > 1 && (strcmp(argv[1], "wait") == 0)) {
	    /* We're going to update in a while, so don't do it now */
	    dontRedraw = 1;
	}
    }

/* view select -- set window into plot space */
/* Specifies in terms of plot window coordinates, not device coordinates */

    else if ((c == 's') && (strncmp(argv[0], "select", length) == 0)) {
        if (argc < 5) {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
                             " view select xmin ymin xmax ymax\"",
                             (char *) NULL);
            return TCL_ERROR;
        }
        else {
            gbox(&xl, &yl, &xr, &yr, argv+1);
            plsdiplt(xl, yl, xr, yr);
        }
    }

/* view zoom -- set window into plot space incrementally (zoom) */
/* Here we need to take the page (device) offsets into account */

    else if ((c == 'z') && (strncmp(argv[0], "zoom", length) == 0)) {
        if (argc < 5) {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
                             " view zoom xmin ymin xmax ymax\"",
                             (char *) NULL);
            return TCL_ERROR;
        } else {
            gbox(&xl, &yl, &xr, &yr, argv+1);
            pldid2pc(&xl, &yl, &xr, &yr);
            plsdiplz(xl, yl, xr, yr);
	    if (argc > 5 && (strcmp(argv[5], "wait") == 0)) {
		/* We're going to update in a while, so don't do it now */
		dontRedraw = 1;
	    }
        }
    }

/* unrecognized */

    else {
        Tcl_AppendResult(interp, "bad option \"", argv[1],
         "\":  options to \"view\" are: bounds, reset, select, or zoom",
         (char *) NULL);

        return TCL_ERROR;
    }

/* Update plot window bounds and arrange for plot to be updated */

    plgdiplt(&xl, &yl, &xr, &yr);
    plPlotterPtr->xl = xl;
    plPlotterPtr->yl = yl;
    plPlotterPtr->xr = xr;
    plPlotterPtr->yr = yr;
    plPlotterPtr->flags |= UPDATE_V_SCROLLBAR|UPDATE_H_SCROLLBAR;

    if (dontRedraw) {
        return TCL_OK;
    } else {
	return Redraw(interp, plPlotterPtr, argc, argv);
    }
}

/*--------------------------------------------------------------------------*\
 * Scroll
 *
 * Processes "xview or yview" widget command.
 * Handles horizontal/vert scroll-bar invoked translation of view into plot.
\*--------------------------------------------------------------------------*/

static int
Scroll(Tcl_Interp *interp, register PlPlotter *plPlotterPtr)
{
    plsstrm(plPlotterPtr->ipls);
    plsdiplt(plPlotterPtr->xl, plPlotterPtr->yl, plPlotterPtr->xr, plPlotterPtr->yr);

    plPlotterPtr->flags |= UPDATE_V_SCROLLBAR | UPDATE_H_SCROLLBAR;
    return (Redraw(interp, plPlotterPtr, 0, NULL));
}


/*--------------------------------------------------------------------------*\
 * report
 *
 * 4/17/95 GMF
 * Processes "report" widget command.
\*--------------------------------------------------------------------------*/

static int
report( Tcl_Interp *interp, register PlPlotter *plPlotterPtr,
        int argc, CONST char **argv )
{
    PLFLT x, y;
/*    fprintf( stdout, "Made it into report, argc=%d\n", argc ); */

    if (argc == 0) {
        interp->result = "report what?";
        return TCL_ERROR;
    }

    if (!strcmp( argv[0], "wc" )) {

        TkwDev *dev = (TkwDev *) plPlotterPtr->pls->dev;
        PLGraphicsIn *gin = &(dev->gin);

        if (argc != 3) {
            interp->result = "Wrong # of args: report wc x y";
            return TCL_ERROR;
        }

        x = (PLFLT) atof( argv[1] );
        y = (PLFLT) atof( argv[2] );

        gin->dX = (PLFLT) x / (dev->width - 1);
        gin->dY = (PLFLT) 1.0 - (PLFLT) y / (dev->height - 1);

    /* Try to locate cursor */

        if (plTranslateCursor(gin)) {
            sprintf( interp->result, "%f %f", gin->wX, gin->wY );
            return TCL_OK;
        }

        interp->result = "Cannot locate";
        return TCL_OK;
    }

    interp->result = "nonsensical request.";
    return TCL_ERROR;
}

/*--------------------------------------------------------------------------*\
 * Utility routines
\*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*\
 * UpdateVScrollbar
 *
 * Updates vertical scrollbar if needed.
\*--------------------------------------------------------------------------*/

static void
UpdateVScrollbar(register PlPlotter *plPlotterPtr)
{
    char string[60];
    int result;

    if (plPlotterPtr->yScrollCmd == NULL)
        return;

    sprintf(string, " %f %f",1.-plPlotterPtr->yr,1.-plPlotterPtr->yl);

    result = Tcl_VarEval(plPlotterPtr->interp, plPlotterPtr->yScrollCmd, string,
                         (char *) NULL);

    if (result != TCL_OK) {
        Tcl_BackgroundError(plPlotterPtr->interp);
    }
}

/*--------------------------------------------------------------------------*\
 * UpdateHScrollbar
 *
 * Updates horizontal scrollbar if needed.
\*--------------------------------------------------------------------------*/

static void
UpdateHScrollbar(register PlPlotter *plPlotterPtr)
{
    char string[60];
	int result;

    if (plPlotterPtr->xScrollCmd == NULL)
        return;

    sprintf(string, " %f %f",plPlotterPtr->xl,plPlotterPtr->xr);

    result = Tcl_VarEval(plPlotterPtr->interp, plPlotterPtr->xScrollCmd, string,
                         (char *) NULL);

    if (result != TCL_OK) {
        Tcl_BackgroundError(plPlotterPtr->interp);
    }
}

/*--------------------------------------------------------------------------*\
 * gbox
 *
 * Returns selection box coordinates.  It's best if the TCL script does
 * bounds checking on the input but I do it here as well just to be safe.
\*--------------------------------------------------------------------------*/

static void
gbox(PLFLT *xl, PLFLT *yl, PLFLT *xr, PLFLT *yr, CONST char **argv)
{
    PLFLT x0, y0, x1, y1;

    x0 = (PLFLT)atof(argv[0]);
    y0 = (PLFLT)atof(argv[1]);
    x1 = (PLFLT)atof(argv[2]);
    y1 = (PLFLT)atof(argv[3]);

    x0 = MAX((PLFLT)0., MIN((PLFLT)1., x0));
    y0 = MAX((PLFLT)0., MIN((PLFLT)1., y0));
    x1 = MAX((PLFLT)0., MIN((PLFLT)1., x1));
    y1 = MAX((PLFLT)0., MIN((PLFLT)1., y1));

/* Only need two vertices, pick the lower left and upper right */

    *xl = MIN(x0, x1);
    *yl = MIN(y0, y1);
    *xr = MAX(x0, x1);
    *yr = MAX(y0, y1);
}















