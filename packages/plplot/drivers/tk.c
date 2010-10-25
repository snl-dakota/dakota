/* $Id: tk.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 *	PLplot Tcl/Tk and Tcl-DP device drivers.
 *	Should be broken up somewhat better to allow use of DP w/o X.
 *
 *	Maurice LeBrun
 *	30-Apr-93
 *
 * Copyright (C) 2004  Maurice LeBrun
 * Copyright (C) 2004  Joao Cardoso
 * Copyright (C) 2004  Andrew Ross
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

/*
#define DEBUG_ENTER
*/

#define DEBUG

#include "plDevs.h"

#ifdef PLD_tk

#define NEED_PLDEBUG
#include "pltkd.h"
#include "plxwd.h"
#include "pltcl.h"
#include "tcpip.h"
#include "drivers.h"
#include "metadefs.h"
#include "plevent.h"

#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/types.h>
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#ifdef PLD_dp
# include <dp.h>
#endif

/* Device info */
char* plD_DEVICE_INFO_tk = "tk:Tcl/TK Window:1:tk:7:tk";


/* Number of instructions to skip between updates */

#define MAX_INSTR 100

/* Pixels/mm */

#define PHYSICAL	0		/* Enables physical scaling.. */

/* These need to be distinguished since the handling is slightly different. */

#define LOCATE_INVOKED_VIA_API		1
#define LOCATE_INVOKED_VIA_DRIVER	2

/* A handy command wrapper */

#define tk_wr(code) \
if (code) { abort_session(pls, "Unable to write to PDFstrm"); }

/*--------------------------------------------------------------------------*/
/* Function prototypes */

/* Driver entry and dispatch setup */

void plD_dispatch_init_tk	( PLDispatchTable *pdt );

void plD_init_tk		(PLStream *);
void plD_line_tk		(PLStream *, short, short, short, short);
void plD_polyline_tk		(PLStream *, short *, short *, PLINT);
void plD_eop_tk			(PLStream *);
void plD_bop_tk			(PLStream *);
void plD_tidy_tk		(PLStream *);
void plD_state_tk		(PLStream *, PLINT);
void plD_esc_tk			(PLStream *, PLINT, void *);

/* various */

static void  init		(PLStream *pls);
static void  tk_start		(PLStream *pls);
static void  tk_stop		(PLStream *pls);
static void  tk_di		(PLStream *pls);
static void  tk_fill		(PLStream *pls);
static void  WaitForPage	(PLStream *pls);
static void  CheckForEvents	(PLStream *pls);
static void  HandleEvents	(PLStream *pls);
static void  init_server	(PLStream *pls);
static void  launch_server	(PLStream *pls);
static void  flush_output	(PLStream *pls);
static void  plwindow_init	(PLStream *pls);
static void  link_init		(PLStream *pls);
static void  GetCursor		(PLStream *pls, PLGraphicsIn *ptr);
static void  tk_XorMod          (PLStream *pls, PLINT *ptr);
static void  set_windowname	(PLStream *pls);

/* performs Tk-driver-specific initialization */

static int   pltkdriver_Init	(PLStream *pls);

/* Tcl/TK utility commands */

static void  tk_wait		(PLStream *pls, char *);
static void  abort_session	(PLStream *pls, char *);
static void  server_cmd		(PLStream *pls, char *, int);
static void  tcl_cmd		(PLStream *pls, char *);
static void  copybuf		(PLStream *pls, char *cmd);
static int   pltk_toplevel	(Tk_Window *w, Tcl_Interp *interp);

static void  ProcessKey		(PLStream *pls);
static void  ProcessButton	(PLStream *pls);
static void  LocateKey		(PLStream *pls);
static void  LocateButton	(PLStream *pls);
static void  Locate		(PLStream *pls);

/* These are internal TCL commands */

static int   Abort		(ClientData, Tcl_Interp *, int, char **);
static int   Plfinfo		(ClientData, Tcl_Interp *, int, char **);
static int   KeyEH		(ClientData, Tcl_Interp *, int, char **);
static int   ButtonEH		(ClientData, Tcl_Interp *, int, char **);
static int   LookupTkKeyEvent	(PLStream *pls, Tcl_Interp *interp,
				 int argc, char **argv);
static int   LookupTkButtonEvent(PLStream *pls, Tcl_Interp *interp,
				 int argc, char **argv);

static char *drvoptcmd = NULL;    /* tcl command from command line option parsing */

static DrvOpt tk_options[] = {{"tcl_cmd", DRV_STR, &drvoptcmd, "Execute tcl command"},
	                      {NULL,DRV_INT,NULL,NULL}};

void plD_dispatch_init_tk( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "Tcl/TK Window";
    pdt->pl_DevName  = "tk";
#endif
    pdt->pl_type     = plDevType_Interactive;
    pdt->pl_seq      = 7;
    pdt->pl_init     = (plD_init_fp)     plD_init_tk;
    pdt->pl_line     = (plD_line_fp)     plD_line_tk;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_tk;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_tk;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_tk;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_tk;
    pdt->pl_state    = (plD_state_fp)    plD_state_tk;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_tk;
}

/*--------------------------------------------------------------------------*\
 * plD_init_dp()
 * plD_init_tk()
 * init_tk()
 *
 * Initialize device.
 * TK-dependent stuff done in tk_start().  You can set the display by
 * calling plsfnam() with the display name as the (string) argument.
\*--------------------------------------------------------------------------*/

void
plD_init_tk(PLStream *pls)
{
    pls->dp = 0;
    plParseDrvOpts(tk_options);
    init(pls);
}

void
plD_init_dp(PLStream *pls)
{
#ifdef PLD_dp
    pls->dp = 1;
#else
    fprintf(stderr, "The Tcl-DP driver hasn't been installed!\n");
    pls->dp = 0;
#endif
    init(pls);
}

static void
tk_wr_header(PLStream *pls, char *header)
{
    tk_wr( pdf_wr_header(pls->pdfs, header) );
}

static void
init(PLStream *pls)
{
    U_CHAR c = (U_CHAR) INITIALIZE;
    TkDev *dev;
    PLFLT pxlx, pxly;
    int xmin = 0;
    int xmax = PIXELS_X - 1;
    int ymin = 0;
    int ymax = PIXELS_Y - 1;

    dbug_enter("plD_init_tk");

    pls->color = 1;		/* Is a color device */
    pls->termin = 1;		/* Is an interactive terminal */
    pls->dev_di = 1;		/* Handle driver interface commands */
    pls->dev_flush = 1;		/* Handle our own flushes */
    pls->dev_fill0 = 1;		/* Handle solid fills */
    pls->dev_fill1 = 1;		/* Handle pattern fills */
    pls->server_nokill = 1;     /* don't kill if ^C */
    pls->dev_xor = 1;           /* device support xor mode */

/* Activate plot buffer. To programmatically save a file we can't call
   plreplot(), but instead one must send a command to plserver. As there is
   no API call for this, the user must use the plserver "save/print" menu
   entries. Activating the plot buffer enables the normal
   plmkstrm/plcpstrm/plreplot/plend1 way of saving plots.
*/
    pls->plbuf_write = 1;

/* Specify buffer size if not yet set (can be changed by -bufmax option).  */
/* A small buffer works best for socket communication */

    if (pls->bufmax == 0) {
	if (pls->dp)
	    pls->bufmax = 450;
	else
	    pls->bufmax = 3500;
    }

/* Allocate and initialize device-specific data */

    if (pls->dev != NULL)
	free((void *) pls->dev);

    pls->dev = calloc(1, (size_t) sizeof(TkDev));
    if (pls->dev == NULL)
	plexit("plD_init_tk: Out of memory.");

    dev = (TkDev *) pls->dev;

    dev->iodev = (PLiodev *) calloc(1, (size_t) sizeof(PLiodev));
    if (dev->iodev == NULL)
	plexit("plD_init_tk: Out of memory.");

    dev->exit_eventloop = 0;

/* Variables used in querying plserver for events */

    dev->instr = 0;
    dev->max_instr = MAX_INSTR;

/* Start interpreter and spawn server process */

    tk_start(pls);

/* Get ready for plotting */

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;

#if PHYSICAL
    pxlx = (double) PIXELS_X / dev->width  * DPMM;
    pxly = (double) PIXELS_Y / dev->height * DPMM;
#else
    pxlx = (double) PIXELS_X / LPAGE_X;
    pxly = (double) PIXELS_Y / LPAGE_Y;
#endif

    plP_setpxl(pxlx, pxly);
    plP_setphy(xmin, xmax, ymin, ymax);

/* Send init info */

    tk_wr( pdf_wr_1byte(pls->pdfs, c) );

/* The header and version fields are useful when the client & server */
/* reside on different machines */

    tk_wr_header(pls, PLSERV_HEADER);
    tk_wr_header(pls, PLSERV_VERSION);

    tk_wr_header(pls, "xmin");
    tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) xmin) );

    tk_wr_header(pls, "xmax");
    tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) xmax) );

    tk_wr_header(pls, "ymin");
    tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) ymin) );

    tk_wr_header(pls, "ymax");
    tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) ymax) );

    tk_wr_header(pls, "");

/* Write color map state info */
    plD_state_tk(pls, PLSTATE_CMAP0);
    plD_state_tk(pls, PLSTATE_CMAP1);

/* Good place to make sure the data transfer is working OK */

    flush_output(pls);
}

/*--------------------------------------------------------------------------*\
 * plD_line_tk()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_line_tk(PLStream *pls, short x1, short y1, short x2, short y2)
{
    U_CHAR c;
    U_SHORT xy[4];
    TkDev *dev = (TkDev *) pls->dev;

    CheckForEvents(pls);

    if (x1 == dev->xold && y1 == dev->yold) {
	c = (U_CHAR) LINETO;
	tk_wr( pdf_wr_1byte(pls->pdfs, c) );

	xy[0] = x2;
	xy[1] = y2;
	tk_wr( pdf_wr_2nbytes(pls->pdfs, xy, 2) );
    }
    else {
	c = (U_CHAR) LINE;
	tk_wr( pdf_wr_1byte(pls->pdfs, c) );

	xy[0] = x1;
	xy[1] = y1;
	xy[2] = x2;
	xy[3] = y2;
	tk_wr( pdf_wr_2nbytes(pls->pdfs, xy, 4) );
    }
    dev->xold = x2;
    dev->yold = y2;

    if (pls->pdfs->bp > pls->bufmax)
	flush_output(pls);
}

/*--------------------------------------------------------------------------*\
 * plD_polyline_tk()
 *
 * Draw a polyline in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_polyline_tk(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    U_CHAR c = (U_CHAR) POLYLINE;
    TkDev *dev = (TkDev *) pls->dev;

    CheckForEvents(pls);

    tk_wr( pdf_wr_1byte(pls->pdfs, c) );
    tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) npts) );

    tk_wr( pdf_wr_2nbytes(pls->pdfs, (U_SHORT *) xa, npts) );
    tk_wr( pdf_wr_2nbytes(pls->pdfs, (U_SHORT *) ya, npts) );

    dev->xold = xa[npts - 1];
    dev->yold = ya[npts - 1];

    if (pls->pdfs->bp > pls->bufmax)
	flush_output(pls);
}

/*--------------------------------------------------------------------------*\
 * plD_eop_tk()
 *
 * End of page.
 * User must hit <RETURN> to continue.
\*--------------------------------------------------------------------------*/

void
plD_eop_tk(PLStream *pls)
{
    U_CHAR c = (U_CHAR) EOP;

    dbug_enter("plD_eop_tk");

    tk_wr( pdf_wr_1byte(pls->pdfs, c) );
    flush_output(pls);
    if ( ! pls->nopause)
	WaitForPage(pls);
}

/*--------------------------------------------------------------------------*\
 * plD_bop_tk()
 *
 * Set up for the next page.
\*--------------------------------------------------------------------------*/

void
plD_bop_tk(PLStream *pls)
{
    U_CHAR c = (U_CHAR) BOP;
    TkDev *dev = (TkDev *) pls->dev;

    dbug_enter("plD_bop_tk");

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;
    pls->page++;
    tk_wr( pdf_wr_1byte(pls->pdfs, c) );
}

/*--------------------------------------------------------------------------*\
 * plD_tidy_tk()
 *
 * Close graphics file
\*--------------------------------------------------------------------------*/

void
plD_tidy_tk(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;

    dbug_enter("plD_tidy_tk");

    if (dev != NULL)
	tk_stop(pls);
}

/*--------------------------------------------------------------------------*\
 * plD_state_tk()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*--------------------------------------------------------------------------*/

void
plD_state_tk(PLStream *pls, PLINT op)
{
    U_CHAR c = (U_CHAR) CHANGE_STATE;
    int i;

    dbug_enter("plD_state_tk");

    tk_wr( pdf_wr_1byte(pls->pdfs, c) );
    tk_wr( pdf_wr_1byte(pls->pdfs, op) );

    switch (op) {

    case PLSTATE_WIDTH:
	tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) (pls->width)) );
	break;

    case PLSTATE_COLOR0:
	tk_wr( pdf_wr_2bytes(pls->pdfs, (short) pls->icol0) );

	if (pls->icol0 == PL_RGB_COLOR) {
	    tk_wr( pdf_wr_1byte(pls->pdfs, pls->curcolor.r) );
	    tk_wr( pdf_wr_1byte(pls->pdfs, pls->curcolor.g) );
	    tk_wr( pdf_wr_1byte(pls->pdfs, pls->curcolor.b) );
	}
	break;

    case PLSTATE_COLOR1:
	tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) pls->icol1) );
	break;

    case PLSTATE_FILL:
	tk_wr( pdf_wr_1byte(pls->pdfs, (U_CHAR) pls->patt) );
	break;

    case PLSTATE_CMAP0:
	tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) pls->ncol0) );
	for (i = 0; i < pls->ncol0; i++) {
	    tk_wr( pdf_wr_1byte(pls->pdfs, pls->cmap0[i].r) );
	    tk_wr( pdf_wr_1byte(pls->pdfs, pls->cmap0[i].g) );
	    tk_wr( pdf_wr_1byte(pls->pdfs, pls->cmap0[i].b) );
	}
	break;

    case PLSTATE_CMAP1:
	tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) pls->ncol1) );
	for (i = 0; i < pls->ncol1; i++) {
	    tk_wr( pdf_wr_1byte(pls->pdfs, pls->cmap1[i].r) );
	    tk_wr( pdf_wr_1byte(pls->pdfs, pls->cmap1[i].g) );
	    tk_wr( pdf_wr_1byte(pls->pdfs, pls->cmap1[i].b) );
	}
    /* Need to send over the control points too! */
	tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) pls->ncp1) );
	for (i = 0; i < pls->ncp1; i++) {
	    tk_wr( pdf_wr_ieeef(pls->pdfs, pls->cmap1cp[i].h) );
	    tk_wr( pdf_wr_ieeef(pls->pdfs, pls->cmap1cp[i].l) );
	    tk_wr( pdf_wr_ieeef(pls->pdfs, pls->cmap1cp[i].s) );
	    tk_wr( pdf_wr_1byte(pls->pdfs, pls->cmap1cp[i].rev) );
	}
	break;
    }

    if (pls->pdfs->bp > pls->bufmax)
	flush_output(pls);
}

/*--------------------------------------------------------------------------*\
 * plD_esc_tk()
 *
 * Escape function.
 * Functions:
 *
 *	PLESC_EXPOSE	Force an expose (just passes token)
 *	PLESC_RESIZE	Force a resize (just passes token)
 *	PLESC_REDRAW	Force a redraw
 *	PLESC_FLUSH	Flush X event buffer
 *	PLESC_FILL	Fill polygon
 *	PLESC_EH	Handle events only
 *	PLESC_XORMOD	Xor mode
 *
\*--------------------------------------------------------------------------*/

void
plD_esc_tk(PLStream *pls, PLINT op, void *ptr)
{
    U_CHAR c = (U_CHAR) ESCAPE;

    dbug_enter("plD_esc_tk");

    switch (op) {

    case PLESC_DI:
	tk_wr( pdf_wr_1byte(pls->pdfs, c) );
	tk_wr( pdf_wr_1byte(pls->pdfs, op) );
	tk_di(pls);
	break;

    case PLESC_EH:
	tk_wr( pdf_wr_1byte(pls->pdfs, c) );
	tk_wr( pdf_wr_1byte(pls->pdfs, op) );
	HandleEvents(pls);
	break;

    case PLESC_FLUSH:
	tk_wr( pdf_wr_1byte(pls->pdfs, c) );
	tk_wr( pdf_wr_1byte(pls->pdfs, op) );
	flush_output(pls);
	break;

    case PLESC_FILL:
	tk_wr( pdf_wr_1byte(pls->pdfs, c) );
	tk_wr( pdf_wr_1byte(pls->pdfs, op) );
	tk_fill(pls);
	break;

    case PLESC_GETC:
	GetCursor(pls, (PLGraphicsIn *) ptr);
	break;

    case PLESC_XORMOD:
	tk_XorMod(pls, (PLINT *) ptr);
	break;

    default:
	tk_wr( pdf_wr_1byte(pls->pdfs, c) );
	tk_wr( pdf_wr_1byte(pls->pdfs, op) );
    }
}

/*--------------------------------------------------------------------------*\
 * tk_XorMod()
 *
 * enter (mod = 1) or leave (mod = 0) xor mode
 *
\*--------------------------------------------------------------------------*/

static void
tk_XorMod(PLStream *pls, PLINT *ptr)
{
  if (*ptr != 0)
    server_cmd( pls, "$plwidget cmd plxormod 1 st", 1 );
  else
    server_cmd( pls, "$plwidget cmd plxormod 0 st", 1 );
}


/*--------------------------------------------------------------------------*\
 * GetCursor()
 *
 * Waits for a graphics input event and returns coordinates.
\*--------------------------------------------------------------------------*/

static void
GetCursor(PLStream *pls, PLGraphicsIn *ptr)
{
    TkDev *dev = (TkDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

/* Initialize */

    plGinInit(gin);
    dev->locate_mode = LOCATE_INVOKED_VIA_API;
    plD_esc_tk(pls, PLESC_FLUSH, NULL);
    server_cmd( pls, "$plwidget configure -xhairs on", 1 );

/* Run event loop until a point is selected */

    while (gin->pX < 0 && dev->locate_mode) {
	Tk_DoOneEvent(0);
    }

/* Clean up */

    server_cmd( pls, "$plwidget configure -xhairs off", 1 );
    *ptr = *gin;
}

/*--------------------------------------------------------------------------*\
 * tk_di
 *
 * Process driver interface command.
 * Just send the command to the remote PLplot library.
\*--------------------------------------------------------------------------*/

static void
tk_di(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    char str[10];

    dbug_enter("tk_di");

/* Safety feature, should never happen */

    if (dev == NULL) {
	plabort("tk_di: Illegal call to driver (not yet initialized)");
	return;
    }

/* Flush the buffer before proceeding */

    flush_output(pls);

/* Change orientation */

    if (pls->difilt & PLDI_ORI) {
	sprintf(str, "%f", pls->diorot);
	Tcl_SetVar(dev->interp, "rot", str, 0);

	server_cmd( pls, "$plwidget cmd plsetopt -ori $rot", 1 );
	pls->difilt &= ~PLDI_ORI;
    }

/* Change window into plot space */

    if (pls->difilt & PLDI_PLT) {
	sprintf(str, "%f", pls->dipxmin);
	Tcl_SetVar(dev->interp, "xl", str, 0);
	sprintf(str, "%f", pls->dipymin);
	Tcl_SetVar(dev->interp, "yl", str, 0);
	sprintf(str, "%f", pls->dipxmax);
	Tcl_SetVar(dev->interp, "xr", str, 0);
	sprintf(str, "%f", pls->dipymax);
	Tcl_SetVar(dev->interp, "yr", str, 0);

	server_cmd( pls, "$plwidget cmd plsetopt -wplt $xl,$yl,$xr,$yr", 1 );
	pls->difilt &= ~PLDI_PLT;
    }

/* Change window into device space */

    if (pls->difilt & PLDI_DEV) {
	sprintf(str, "%f", pls->mar);
	Tcl_SetVar(dev->interp, "mar", str, 0);
	sprintf(str, "%f", pls->aspect);
	Tcl_SetVar(dev->interp, "aspect", str, 0);
	sprintf(str, "%f", pls->jx);
	Tcl_SetVar(dev->interp, "jx", str, 0);
	sprintf(str, "%f", pls->jy);
	Tcl_SetVar(dev->interp, "jy", str, 0);

	server_cmd( pls, "$plwidget cmd plsetopt -mar $mar", 1 );
	server_cmd( pls, "$plwidget cmd plsetopt -a $aspect", 1 );
	server_cmd( pls, "$plwidget cmd plsetopt -jx $jx", 1 );
	server_cmd( pls, "$plwidget cmd plsetopt -jy $jy", 1 );
	pls->difilt &= ~PLDI_DEV;
    }

/* Update view */

    server_cmd( pls, "update", 1 );
    server_cmd( pls, "plw::update_view $plwindow", 1 );
}

/*--------------------------------------------------------------------------*\
 * tk_fill()
 *
 * Fill polygon described in points pls->dev_x[] and pls->dev_y[].
\*--------------------------------------------------------------------------*/

static void
tk_fill(PLStream *pls)
{
    PLDev *dev = (PLDev *) pls->dev;

    dbug_enter("tk_fill");

    tk_wr( pdf_wr_2bytes(pls->pdfs, (U_SHORT) pls->dev_npts) );

    tk_wr( pdf_wr_2nbytes(pls->pdfs, (U_SHORT *) pls->dev_x, pls->dev_npts) );
    tk_wr( pdf_wr_2nbytes(pls->pdfs, (U_SHORT *) pls->dev_y, pls->dev_npts) );

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;
}

/*--------------------------------------------------------------------------*\
 * tk_start
 *
 * Create TCL interpreter and spawn off server process.
 * Each stream that uses the tk driver gets its own interpreter.
\*--------------------------------------------------------------------------*/

static void
tk_start(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;

    dbug_enter("tk_start");

/* Instantiate a TCL interpreter, and get rid of the exec command */

    dev->interp = Tcl_CreateInterp();

    if (Tcl_Init(dev->interp) != TCL_OK) {
	fprintf(stderr, "%s\n", dev->interp->result);
	abort_session(pls, "Unable to initialize Tcl");
    }

    tcl_cmd(pls, "rename exec {}");

/* Set top level window name & initialize */

    set_windowname(pls);
    if (pls->dp) {
	Tcl_SetVar(dev->interp, "dp", "1", TCL_GLOBAL_ONLY);
	dev->updatecmd = "dp_update";
    }
    else {
	Tcl_SetVar(dev->interp, "dp", "0", TCL_GLOBAL_ONLY);

    /* tk_init needs this. Use pls->FileName first, then DISPLAY, then :0.0 */

        if (pls->FileName != NULL)
            Tcl_SetVar2(dev->interp, "env", "DISPLAY", pls->FileName, TCL_GLOBAL_ONLY);
	else if (getenv("DISPLAY") != NULL)
            Tcl_SetVar2(dev->interp, "env", "DISPLAY", getenv("DISPLAY"), TCL_GLOBAL_ONLY); /* tk_init need this */
	else
            Tcl_SetVar2(dev->interp, "env", "DISPLAY", "unix:0.0", TCL_GLOBAL_ONLY); /* tk_init need this */

	dev->updatecmd = "update";
	if (pltk_toplevel(&dev->w, dev->interp))
	    abort_session(pls, "Unable to create top-level window");
    }

/* Eval startup procs */

    if (pltkdriver_Init(pls) != TCL_OK) {
	abort_session(pls, "");
    }

    if (pls->debug)
	tcl_cmd(pls, "global auto_path; puts \"auto_path: $auto_path\"");

/* Other initializations. */
/* Autoloaded, so the user can customize it if desired */

    tcl_cmd(pls, "plclient_init");

/* A different way to customize the interface. */
/* E.g. used by plrender to add a back page button. */

    if (drvoptcmd)
	tcl_cmd(pls, drvoptcmd);

/* Initialize server process */

    init_server(pls);

/* By now we should be done with all autoloaded procs, so blow away */
/* the open command just in case security has been compromised */

    tcl_cmd(pls, "rename open {}");
    tcl_cmd(pls, "rename rename {}");

/* Initialize widgets */

    plwindow_init(pls);

/* Initialize data link */

    link_init(pls);

    return;
}

/*--------------------------------------------------------------------------*\
 * tk_stop
 *
 * Normal termination & cleanup.
\*--------------------------------------------------------------------------*/

static void
tk_stop(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;

    dbug_enter("tk_stop");

/* Safety check for out of control code */

    if (dev->pass_thru)
	return;

    dev->pass_thru = 1;

/* Kill plserver */

    tcl_cmd(pls, "plclient_link_end");

/* Wait for child process to complete */

    if (dev->child_pid) {
    	waitpid(dev->child_pid, NULL, 0);
/*
	problems if parent has not caught/ignore SIGCHLD. Returns -1 and errno=EINTR
	if (waitpid(dev->child_pid, NULL, 0) != dev->child_pid)
	    fprintf(stderr, "tk_stop: waidpid error");
*/
    }

/* Blow away interpreter */

    Tcl_DeleteInterp(dev->interp);
    dev->interp = NULL;

/* Free up memory and other miscellanea */

    pdf_close(pls->pdfs);
    if (dev->iodev != NULL) {
	if (dev->iodev->file != NULL)
	    fclose(dev->iodev->file);

	free((void *) dev->iodev);
    }
    free_mem(dev->cmdbuf);
}

/*--------------------------------------------------------------------------*\
 * abort_session
 *
 * Terminates with an error.
 * Cleanup is done here, and once pls->level is cleared the driver will
 * never be called again.
\*--------------------------------------------------------------------------*/

static void
abort_session(PLStream *pls, char *msg)
{
    TkDev *dev = (TkDev *) pls->dev;

    dbug_enter("abort_session");

/* Safety check for out of control code */

    if (dev->pass_thru)
	return;

    tk_stop(pls);
    pls->level = 0;

    plexit(msg);
}

/*--------------------------------------------------------------------------*\
 * pltkdriver_Init
 *
 * Performs PLplot/TK driver-specific Tcl initialization.
\*--------------------------------------------------------------------------*/

static int
pltkdriver_Init(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    Tcl_Interp *interp = (Tcl_Interp *) dev->interp;

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
#ifdef PLD_dp
    if (pls->dp) {
	if (Tdp_Init(interp) == TCL_ERROR) {
	    return TCL_ERROR;
	}
    }
#endif

/*
 * Call Tcl_CreateCommand for application-specific commands, if
 * they weren't already created by the init procedures called above.
 */

    Tcl_CreateCommand(interp, "wait_until", (Tcl_CmdProc*) plWait_Until,
		      (ClientData)  NULL, (Tcl_CmdDeleteProc*) NULL);

#ifdef PLD_dp
    if (pls->dp) {
	Tcl_CreateCommand(interp, "host_id", (Tcl_CmdProc*) plHost_ID,
			  (ClientData) NULL,  (Tcl_CmdDeleteProc*) NULL);
    }
#endif

    Tcl_CreateCommand(interp, "abort", (Tcl_CmdProc*) Abort,
		      (ClientData) pls, (Tcl_CmdDeleteProc*) NULL);

    Tcl_CreateCommand(interp, "plfinfo", (Tcl_CmdProc*) Plfinfo,
		      (ClientData) pls, (Tcl_CmdDeleteProc*) NULL);

    Tcl_CreateCommand(interp, "keypress", (Tcl_CmdProc*) KeyEH,
		      (ClientData) pls, (Tcl_CmdDeleteProc*) NULL);

    Tcl_CreateCommand(interp, "buttonpress", (Tcl_CmdProc*) ButtonEH,
		      (ClientData) pls, (Tcl_CmdDeleteProc*) NULL);

/* Set some relevant interpreter variables */

    if ( ! pls->dp)
	tcl_cmd(pls, "set client_name [winfo name .]");

    if (pls->server_name != NULL)
	Tcl_SetVar(interp, "server_name", pls->server_name, 0);

    if (pls->server_host != NULL)
	Tcl_SetVar(interp, "server_host", pls->server_host, 0);

    if (pls->server_port != NULL)
	Tcl_SetVar(interp, "server_port", pls->server_port, 0);

/* Set up auto_path */

    if (pls_auto_path(interp) == TCL_ERROR)
	return TCL_ERROR;

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * init_server
 *
 * Starts interaction with server process, launching it if necessary.
 *
 * There are several possibilities we must account for, depending on the
 * message protocol, input flags, and whether plserver is already running
 * or not.  From the point of view of the code, they are:
 *
 *    1. Driver: tk
 *	 Flags: <none>
 *	 Meaning: need to start up plserver (same host)
 *	 Actions: fork plserver, passing it our TK main window name
 *		  for communication.  Once started, plserver will send
 *		  back its main window name.
 *
 *    2. Driver: dp
 *	 Flags: <none>
 *	 Meaning: need to start up plserver (same host)
 *	 Actions: fork plserver, passing it our Tcl-DP communication port
 *		  for communication. Once started, plserver will send
 *		  back its created message port number.
 *
 *    3. Driver: tk
 *	 Flags: -server_name
 *	 Meaning: plserver already running (same host)
 *	 Actions: communicate to plserver our TK main window name.
 *
 *    4. Driver: dp
 *	 Flags: -server_port
 *	 Meaning: plserver already running (same host)
 *	 Actions: communicate to plserver our Tcl-DP port number.
 *
 *    5. Driver: dp
 *	 Flags: -server_host
 *	 Meaning: need to start up plserver (remote host)
 *	 Actions: rsh (remsh) plserver, passing it our host ID and Tcl-DP
 *		  port for communication. Once started, plserver will send
 *		  back its created message port number.
 *
 *    6. Driver: dp
 *	 Flags: -server_host -server_port
 *	 Meaning: plserver already running (remote host)
 *	 Actions: communicate to remote plserver our host ID and Tcl-DP
 *		  port number.
 *
 * For a bit more flexibility, you can change the name of the process
 * invoked from "plserver" to something else, using the -plserver flag.
 *
 * The startup procedure involves some rather involved handshaking between
 * client and server.  This is made easier by using the Tcl variables:
 *
 *	client_host client_port server_host server_port
 *
 * when using Tcl-DP sends and
 *
 *	client_name server_name
 *
 * when using TK sends.  The global Tcl variables
 *
 *	client server
 *
 * are used as the defining identification for the client and server
 * respectively -- they denote the main window name when TK sends are used
 * and the respective process's listening socket when Tcl-DP sends are
 * used.  Note that in the former case, $client is just the same as
 * $client_name.  In addition, since the server may need to communicate
 * with many different client processes, every command to the server
 * contains the sender's client id (so it knows how to report back if
 * necessary).  Thus the Tk driver's interpreter must know both $server as
 * well as $client.  It is most convenient to set $client from the server,
 * as a way to signal that communication has been set up and it is safe to
 * proceed.
 *
 * Often it is necessary to use constructs such as [list $server] instead
 * of just $server.  This occurs since you could have multiple copies
 * running on the display (resulting in names of the form "plserver #2",
 * etc).  Embedding such a string in a "[list ...]" construct prevents the
 * string from being interpreted as two separate strings.
\*--------------------------------------------------------------------------*/

static void
init_server(PLStream *pls)
{
    int server_exists = 0;

    dbug_enter("init_server");

    pldebug("init_server", "%s -- PID: %d, PGID: %d, PPID: %d\n",
	    __FILE__, (int) getpid(), (int) getpgrp(), (int) getppid());

/* If no means of communication provided, need to launch plserver */

    if (( ! pls->dp && pls->server_name != NULL ) ||
	(   pls->dp && pls->server_port != NULL ) )
	server_exists = 1;

/* So launch it */

    if ( ! server_exists)
	launch_server(pls);

/* Set up communication channel to server */

    if (pls->dp) {
	tcl_cmd(pls,
		"set server [dp_MakeRPCClient $server_host $server_port]");
    }
    else {
	tcl_cmd(pls, "set server $server_name");
    }

/* If server didn't need launching, contact it here */

    if (server_exists)
	tcl_cmd(pls, "plclient_link_init");
}

/*--------------------------------------------------------------------------*\
 * launch_server
 *
 * Launches plserver, locally or remotely.
\*--------------------------------------------------------------------------*/

static void
launch_server(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    char *argv[20], *plserver_exec=NULL, *ptr, *tmp=NULL;
    int i;

    dbug_enter("launch_server");

    if (pls->plserver == NULL)
	pls->plserver = plstrdup("plserver");

/* Build argument list */

    i = 0;

/* If we're doing a rsh, need to set up its arguments first. */

    if ( pls->dp && pls->server_host != NULL ) {
	argv[i++] = pls->server_host;	/* Host name for rsh */

	if (pls->user != NULL) {
	    argv[i++] = "-l";
	    argv[i++] = pls->user; 	/* User name on remote node */
	}
    }

/* The invoked executable name comes next */

    argv[i++] = pls->plserver;

/* The rest are arguments to plserver */

    argv[i++] = "-child";		/* Tell plserver its ancestry */

    argv[i++] = "-e";			/* Startup script */
    argv[i++] = "plserver_init";

/* aaahhh. This is it! Without the next statements, control is either
 * in tk or octave, because tcl/tk was in interative mode (I think).
 * This had the inconvenient of having to press the enter key or cliking a
 * mouse button in the plot window after every plot.
 *
 * This couldn't be done with
 *	Tcl_SetVar(dev->interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);
 * after plserver has been launched? It doesnt work, hoewever.
 * Tk_CreateFileHandler (0, TK_READABLE, NULL, 0) doesnt work also
 */

    argv[i++] = "-file";			/* Startup file */
    if (pls->tk_file)
        argv[i++] = pls->tk_file;
    else
    argv[i++] = "/dev/null";


/*
   Give interpreter the base name of the plwindow.
   Useful to know the interpreter name
*/

    if (pls->plwindow != NULL) {
        char *t, *tmp;
        argv[i++] = "-name";            /* plserver name */
	tmp = plstrdup(pls->plwindow + 1); /* get rid of the initial dot */
        argv[i++] = tmp;
        if ((t = strchr(tmp, '.')) != NULL)
            *t = '\0';			/* and keep only the base name */
    } else {
        argv[i++] = "-name";            /* plserver name */
        argv[i++] = pls->program;
    }

    if (pls->auto_path != NULL) {
	argv[i++] = "-auto_path";	/* Additional directory(s) */
	argv[i++] = pls->auto_path;	/* to autoload */
    }

    if (pls->geometry != NULL) {
	argv[i++] = "-geometry";	/* Top level window geometry */
	argv[i++] = pls->geometry;
    }

/* If communicating via Tcl-DP, specify communications port id */
/* If communicating via TK send, specify main window name */

    if (pls->dp) {
	argv[i++] = "-client_host";
	argv[i++] = (char *) Tcl_GetVar(dev->interp, "client_host", TCL_GLOBAL_ONLY);

	argv[i++] = "-client_port";
	argv[i++] = (char *) Tcl_GetVar(dev->interp, "client_port", TCL_GLOBAL_ONLY);

	if (pls->user != NULL) {
	    argv[i++] = "-l";
	    argv[i++] = pls->user;
	}
    }
    else {
	argv[i++] = "-client_name";
	argv[i++] = (char *) Tcl_GetVar(dev->interp, "client_name", TCL_GLOBAL_ONLY);
    }

/* The display absolutely must be set if invoking a remote server (by rsh) */
/* Use the DISPLAY environmental, if set.  Otherwise use the remote host. */

    if (pls->FileName != NULL) {
	argv[i++] = "-display";
	argv[i++] = pls->FileName;
    }
    else if ( pls->dp && pls->server_host != NULL ) {
	argv[i++] = "-display";
	if ((ptr = getenv("DISPLAY")) != NULL)
	    argv[i++] = ptr;
	else
	    argv[i++] = "unix:0.0";
    }

/* Add terminating null */

    argv[i++] = NULL;
#ifdef DEBUG
    if (pls->debug) {
	int j;
	fprintf(stderr, "argument list: \n   ");
	for (j = 0; j < i; j++)
	    fprintf(stderr, "%s ", argv[j]);
	fprintf(stderr, "\n");
    }
#endif

/* Start server process */
/* It's a fork/rsh if on a remote machine */

    if ( pls->dp && pls->server_host != NULL ) {
	if ((dev->child_pid = vfork()) < 0) {
	    abort_session(pls, "Unable to fork server process");
	}
	else if (dev->child_pid == 0) {
	    fprintf(stderr, "Starting up %s on node %s\n", pls->plserver,
		    pls->server_host);

	    if (execvp("rsh", argv)) {
		perror("Unable to exec server process");
		_exit(1);
	    }
	}
    }

/* Running locally, so its a fork/exec */

    else {
	plserver_exec = plFindCommand(pls->plserver);
	if ( (plserver_exec == NULL) || (dev->child_pid = vfork()) < 0) {
	    abort_session(pls, "Unable to fork server process");
	}
	else if (dev->child_pid == 0) {

	/* Don't kill plserver on a ^C if pls->server_nokill is set */

	    if (pls->server_nokill) {
		sigset_t set;
		sigemptyset(&set);
		sigaddset (&set, SIGINT);
		if (sigprocmask (SIG_BLOCK, &set, 0) < 0)
		    fprintf(stderr, "PLplot: sigprocmask failure\n");
	    }

	    pldebug("launch_server", "Starting up %s\n", plserver_exec);
	    if (execv(plserver_exec, argv)) {
		fprintf(stderr, "Unable to exec server process.\n");
		_exit(1);
	    }
	}
	free_mem(plserver_exec);
    }
    free_mem(tmp);

/* Wait for server to set up return communication channel */

    tk_wait(pls, "[info exists client]" );
}

/*--------------------------------------------------------------------------*\
 * plwindow_init
 *
 * Configures the widget hierarchy we are sending the data stream to.
 *
 * If a widget name (identifying the actual widget or a container widget)
 * hasn't been supplied already we assume it needs to be created.
 *
 * In order to achieve maximum flexibility, the PLplot tk driver requires
 * only that certain TCL procs must be defined in the server interpreter.
 * These can be used to set up the desired widget configuration.  The procs
 * invoked from this driver currently include:
 *
 *    $plw_create_proc		Creates the widget environment
 *    $plw_start_proc		Does any remaining startup necessary
 *    $plw_end_proc		Prepares for shutdown
 *    $plw_flash_proc		Invoked when waiting for page advance
 *
 * Since all of these are interpreter variables, they can be trivially
 * changed by the user.
 *
 * Each of these utility procs is called with a widget name ($plwindow)
 * as argument.  "plwindow" is set from the value of pls->plwindow, and
 * if null is generated from the name of the client main window (to
 * ensure uniqueness).  $plwindow usually indicates the container frame
 * for the actual PLplot widget, but can be arbitrary -- as long as the
 * usage in all the TCL procs is consistent.
 *
 * In order that the TK driver be able to invoke the actual PLplot
 * widget, the proc "$plw_create_proc" deposits the widget name in the local
 * interpreter variable "plwidget".
\*--------------------------------------------------------------------------*/

static void
plwindow_init(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    char command[100];
    unsigned int bg;

    dbug_enter("plwindow_init");

    Tcl_SetVar(dev->interp, "plwindow", pls->plwindow, 0);

/* Create the plframe widget & anything else you want with it. */

    server_cmd( pls,
	"$plw_create_proc $plwindow [list $client]", 1 );

    tk_wait(pls, "[info exists plwidget]" );

/* Now we should have the actual PLplot widget name in $plwidget */
/* Configure remote PLplot stream. */

/* Configure background color if anything other than black */
/* The default color is handled from a resource setting in plconfig.tcl */

    bg = pls->cmap0[0].b | (pls->cmap0[0].g << 8) | (pls->cmap0[0].r << 16);
    if (bg > 0) {
	sprintf(command, "$plwidget configure -plbg #%06x", bg);
	server_cmd( pls, command, 0 );
    }

/* nopixmap option */

    if (pls->nopixmap)
	server_cmd( pls, "$plwidget cmd plsetopt -nopixmap", 0 );

/* debugging */

    if (pls->debug)
	server_cmd( pls, "$plwidget cmd plsetopt -debug", 0 );

/* double buffering */

    if (pls->db)
	server_cmd( pls, "$plwidget cmd plsetopt -db", 0 );

/* color map options */

    if (pls->ncol0) {
	sprintf(command, "$plwidget cmd plsetopt -ncol0 %d", pls->ncol0);
	server_cmd( pls, command, 0 );
    }

    if (pls->ncol1) {
	sprintf(command, "$plwidget cmd plsetopt -ncol1 %d", pls->ncol1);
	server_cmd( pls, command, 0 );
    }

/* Start up remote PLplot */

    server_cmd( pls, "$plw_start_proc $plwindow", 1 );
    tk_wait(pls, "[info exists widget_is_ready]" );
}

/*--------------------------------------------------------------------------*\
 * set_windowname
 *
 * Set up top level window name.  Use pls->program, modified appropriately.
\*--------------------------------------------------------------------------*/

static void
set_windowname(PLStream *pls)
{
  char *pname;
  int i;

  /* Set to "plclient" if not initialized via plargs or otherwise */

  if (pls->program == NULL)
    pls->program = plstrdup("plclient");

  /* Eliminate any leading path specification */

  pname = strrchr(pls->program, '/');
  if (pname)
    pname++;
  else
    pname = pls->program;

  if (pls->plwindow == NULL) { /* dont override -plwindow cmd line option */
    pls->plwindow = (char *) malloc(10+(strlen(pname)) * sizeof(char));

    /* Allow for multiple widgets created by multiple streams */

    if (pls->ipls == 0)
      sprintf(pls->plwindow, ".%s", pname);
    else
      sprintf(pls->plwindow, ".%s_%d", pname, (int) pls->ipls);

    /* Replace any ' 's with '_'s to avoid quoting problems. */
    /* Replace any '.'s (except leading) with '_'s to avoid bad window names. */

    for (i = 0; i < (int)strlen(pls->plwindow); i++) {
      if (pls->plwindow[i] == ' ') pls->plwindow[i] = '_';
      if (i == 0) continue;
      if (pls->plwindow[i] == '.') pls->plwindow[i] = '_';
    }
  }
}

/*--------------------------------------------------------------------------*\
 * link_init
 *
 * Initializes the link between the client and the PLplot widget for
 * data transfer.  Defaults to a FIFO when the TK driver is selected and
 * a socket when the DP driver is selected.
\*--------------------------------------------------------------------------*/

static void
link_init(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    PLiodev *iodev = (PLiodev *) dev->iodev;
    long bufmax = pls->bufmax * 1.2;

    dbug_enter("link_init");

/* Create FIFO for data transfer to the plframe widget */

    if ( ! pls->dp) {

	iodev->fileName = (char *) tmpnam(NULL);
	if (mkfifo(iodev->fileName,
		   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0)
	    abort_session(pls, "mkfifo error");

    /* Tell plframe widget to open FIFO (for reading). */

	Tcl_SetVar(dev->interp, "fifoname", iodev->fileName, 0);
	server_cmd( pls, "$plwidget openlink fifo $fifoname", 1 );

    /* Open the FIFO for writing */
    /* This will block until the server opens it for reading */

	if ((iodev->fd = open(iodev->fileName, O_WRONLY)) == -1)
	    abort_session(pls, "Error opening fifo for write");

    /* Create stream interface (C file handle) to FIFO */

	iodev->type = 0;
	iodev->typeName = "fifo";
	iodev->file = fdopen(iodev->fd, "wb");

/* Unlink FIFO so that it isn't left around if program crashes. */
/* This also ensures no other program can mess with it. */

	if (unlink(iodev->fileName) == -1)
	    abort_session(pls, "Error removing fifo");
    }

/* Create socket for data transfer to the plframe widget */

    else {

	iodev->type = 1;
	iodev->typeName = "socket";
	tcl_cmd(pls, "plclient_dp_init");
	iodev->fileHandle = (char *) Tcl_GetVar(dev->interp, "data_sock", 0);

	if (Tcl_GetOpenFile(dev->interp, iodev->fileHandle,
			    0, 1, (ClientData) &iodev->file) != TCL_OK) {

	    fprintf(stderr, "Cannot get file info:\n\t %s\n",
		    dev->interp->result);
	    abort_session(pls, "");
	}
	iodev->fd = fileno(iodev->file);
    }

/* Create data buffer */

    pls->pdfs = pdf_bopen( NULL, bufmax );
}

/*--------------------------------------------------------------------------*\
 * WaitForPage()
 *
 * Waits for a page advance.
\*--------------------------------------------------------------------------*/

static void
WaitForPage(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;

    dbug_enter("WaitForPage");

    while ( ! dev->exit_eventloop) {
	Tk_DoOneEvent(0);
    }
    dev->exit_eventloop = 0;
}

/*--------------------------------------------------------------------------*\
 * CheckForEvents()
 *
 * A front-end to HandleEvents(), which is only called if certain conditions
 * are satisfied:
 *
 * - only check for events and process them every dev->max_instr times this
 *   function is called (good for performance since performing an update is
 *   a nontrivial performance hit).
\*--------------------------------------------------------------------------*/

static void
CheckForEvents(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;

    if (++dev->instr % dev->max_instr == 0) {
	dev->instr = 0;
	HandleEvents(pls);
    }
}

/*--------------------------------------------------------------------------*\
 * HandleEvents()
 *
 * Just a front-end to the update command, for use when not actually waiting
 * for an event but only checking the event queue.
\*--------------------------------------------------------------------------*/

static void
HandleEvents(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;

    dbug_enter("HandleEvents");

    Tcl_VarEval(dev->interp, dev->updatecmd, (char **) NULL);
}

/*--------------------------------------------------------------------------*\
 * flush_output()
 *
 * Sends graphics instructions to the {FIFO|socket} via a packet send.
 *
 * The packet i/o routines are modified versions of the ones from the
 * Tcl-DP package.  They have been altered to take a pointer to a PDFstrm
 * struct, and read-to or write-from pdfs->buffer.  The length of the
 * buffer is stored in pdfs->bp (the original Tcl-DP routine assumes the
 * message is character data and uses strlen).  Also, they can
 * send/receive from either a fifo or a socket.
\*--------------------------------------------------------------------------*/

static void
flush_output(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    PDFstrm *pdfs = (PDFstrm *) pls->pdfs;

    dbug_enter("flush_output");

    HandleEvents(pls);

/* Send packet -- plserver filehandler will be invoked automatically. */

    if (pdfs->bp > 0) {
#ifdef DEBUG_ENTER
	pldebug("flush_output", "%s: Flushing buffer, bytes = %ld\n",
		__FILE__, pdfs->bp);
#endif
	if (pl_PacketSend(dev->interp, dev->iodev, pls->pdfs)) {
	    fprintf(stderr, "Packet send failed:\n\t %s\n",
		    dev->interp->result);
	    abort_session(pls, "");
	}
	pdfs->bp = 0;
    }
}

/*--------------------------------------------------------------------------*\
 * Abort
 *
 * Just a TCL front-end to abort_session().
\*--------------------------------------------------------------------------*/

static int
Abort(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{
    PLStream *pls = (PLStream *) clientData;

    dbug_enter("Abort");

    abort_session(pls, "");
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * Plfinfo
 *
 * Sends info about the server plframe.  Usually issued after some
 * modification to the plframe is made by the user, such as a resize.
\*--------------------------------------------------------------------------*/

static int
Plfinfo(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{
    PLStream *pls = (PLStream *) clientData;
    TkDev *dev = (TkDev *) pls->dev;
    int result = TCL_OK;

    dbug_enter("Plfinfo");

    if (argc < 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
			 " plfinfo wx wy\"", (char *) NULL);
	result = TCL_ERROR;
    }
    else {
	dev->width = atoi(argv[1]);
	dev->height = atoi(argv[2]);
#if PHYSICAL
	{
	    PLFLT pxlx = (double) PIXELS_X / dev->width  * DPMM;
	    PLFLT pxly = (double) PIXELS_Y / dev->height * DPMM;
	    plP_setpxl(pxlx, pxly);
	}
#endif
    }

    return result;
}

/*--------------------------------------------------------------------------*\
 * KeyEH()
 *
 * This TCL command handles keyboard events.
\*--------------------------------------------------------------------------*/

static int
KeyEH(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{
    PLStream *pls = (PLStream *) clientData;
    TkDev *dev = (TkDev *) pls->dev;
    int result;

    dbug_enter("KeyEH");

    if ((result = LookupTkKeyEvent(pls, interp, argc, argv)) != TCL_OK)
	return result;

    if (dev->locate_mode)
	LocateKey(pls);
    else
	ProcessKey(pls);

    return result;
}

/*--------------------------------------------------------------------------*\
 * ButtonEH()
 *
 * This TCL command handles button events.
\*--------------------------------------------------------------------------*/

static int
ButtonEH(ClientData clientData, Tcl_Interp *interp, int argc, char **argv)
{
    PLStream *pls = (PLStream *) clientData;
    TkDev *dev = (TkDev *) pls->dev;
    int result;

    dbug_enter("ButtonEH");

    if ((result = LookupTkButtonEvent(pls, interp, argc, argv)) != TCL_OK)
	return result;

    if (dev->locate_mode)
	LocateButton(pls);
    else
	ProcessButton(pls);

    return result;
}

/*--------------------------------------------------------------------------*\
 * LookupTkKeyEvent()
 *
 * Fills in the PLGraphicsIn from a Tk KeyEvent.
 *
 * Contents of argv array:
 *	command name
 *	keysym value
 *	keysym state
 *	absolute x coordinate of cursor
 *	absolute y coordinate of cursor
 *	relative x coordinate (normalized to [0.0 1.0])
 *	relative y coordinate (normalized to [0.0 1.0])
 *	keysym name
 *	ASCII equivalent (optional)
 *
 * Note that the keysym name is only used for debugging, and the string is
 * not always passed (i.e. the character may not have an ASCII
 * representation).
\*--------------------------------------------------------------------------*/

static int
LookupTkKeyEvent(PLStream *pls, Tcl_Interp *interp, int argc, char **argv)
{
    TkDev *dev = (TkDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);
    char *keyname;

    dbug_enter("LookupTkKeyEvent");

    if (argc < 8) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " key-value state pX pY dX dY key-name ?ascii-value?\"",
			 (char *) NULL);
	return TCL_ERROR;
    }

    gin->keysym = atol(argv[1]);
    gin->state  = atol(argv[2]);
    gin->pX     = atol(argv[3]);
    gin->pY     = atol(argv[4]);
    gin->dX     = atof(argv[5]);
    gin->dY     = atof(argv[6]);

    keyname     = argv[7];

    gin->string[0] = '\0';
    if (argc > 8) {
	gin->string[0] = argv[8][0];
	gin->string[1] = '\0';
    }

/* Fix up keysym value -- see notes in xwin.c about key representation */

    switch (gin->keysym) {

    case XK_BackSpace:
    case XK_Tab:
    case XK_Linefeed:
    case XK_Return:
    case XK_Escape:
    case XK_Delete:
	gin->keysym &= 0xFF;
	break;
    }

    pldebug("LookupTkKeyEvent",
	    "KeyEH: stream: %d, Keyname %s, hex %x, ASCII: %s\n",
	    (int) pls->ipls, keyname, (unsigned int) gin->keysym, gin->string);

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * LookupTkButtonEvent()
 *
 * Fills in the PLGraphicsIn from a Tk ButtonEvent.
 *
 * Contents of argv array:
 *	command name
 *	button number
 *	state (decimal string)
 *	absolute x coordinate
 *	absolute y coordinate
 *	relative x coordinate (normalized to [0.0 1.0])
 *	relative y coordinate (normalized to [0.0 1.0])
\*--------------------------------------------------------------------------*/

static int
LookupTkButtonEvent(PLStream *pls, Tcl_Interp *interp, int argc, char **argv)
{
    TkDev *dev = (TkDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

    dbug_enter("LookupTkButtonEvent");

    if (argc != 7) {
        Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " button-number state pX pY dX dY\"", (char *) NULL);
	return TCL_ERROR;
    }

    gin->button = atol(argv[1]);
    gin->state  = atol(argv[2]);
    gin->pX     = atof(argv[3]);
    gin->pY     = atof(argv[4]);
    gin->dX     = atof(argv[5]);
    gin->dY     = atof(argv[6]);
    gin->keysym = 0x20;

    pldebug("LookupTkButtonEvent",
	    "button %d, state %d, pX: %d, pY: %d, dX: %f, dY: %f\n",
	    gin->button, gin->state, gin->pX, gin->pY, gin->dX, gin->dY);

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 * ProcessKey()
 *
 * Process keyboard events other than locate input.
\*--------------------------------------------------------------------------*/

static void
ProcessKey(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

    dbug_enter("ProcessKey");

/* Call user keypress event handler.  Since this is called first, the user
 * can disable all internal event handling by setting key.keysym to 0.
*/
    if (pls->KeyEH != NULL)
	(*pls->KeyEH) (gin, pls->KeyEH_data, &dev->exit_eventloop);

/* Handle internal events */

    switch (gin->keysym) {

    case PLK_Return:
    case PLK_Linefeed:
    case PLK_Next:
    /* Advance to next page (i.e. terminate event loop) on a <eol> */
    /* Check for both <CR> and <LF> for portability, also a <Page Down> */
	dev->exit_eventloop = TRUE;
	break;

    case 'Q':
    /* Terminate on a 'Q' (not 'q', since it's too easy to hit by mistake) */
	tcl_cmd(pls, "abort");
	break;

    case 'L':
    /* Begin locate mode */
	dev->locate_mode = LOCATE_INVOKED_VIA_DRIVER;
	server_cmd( pls, "$plwidget configure -xhairs on", 1 );
	break;
    }
}

/*--------------------------------------------------------------------------*\
 * ProcessButton()
 *
 * Process ButtonPress events other than locate input.
 * On:
 *   Button1: nothing (except when in locate mode, see ButtonLocate)
 *   Button2: nothing
 *   Button3: set page advance flag
\*--------------------------------------------------------------------------*/

static void
ProcessButton(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

    dbug_enter("ButtonEH");

/* Call user event handler.  Since this is called first, the user can
 * disable all PLplot internal event handling by setting gin->button to 0.
*/
    if (pls->ButtonEH != NULL)
	(*pls->ButtonEH) (gin, pls->ButtonEH_data, &dev->exit_eventloop);

/* Handle internal events */

    switch (gin->button) {
    case Button3:
	dev->exit_eventloop = TRUE;
	break;
    }
}

/*--------------------------------------------------------------------------*\
 * LocateKey()
 *
 * Front-end to locate handler for KeyPress events.
 * Only provides for:
 *
 *  <Escape>		Ends locate mode
\*--------------------------------------------------------------------------*/

static void
LocateKey(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

/* End locate mode on <Escape> */

    if (gin->keysym == PLK_Escape) {
	dev->locate_mode = 0;
	server_cmd( pls, "$plwidget configure -xhairs off", 1 );
	plGinInit(gin);
    }
    else {
	Locate(pls);
    }
}

/*--------------------------------------------------------------------------*\
 * LocateButton()
 *
 * Front-end to locate handler for ButtonPress events.
 * Only passes control to Locate() for Button1 presses.
\*--------------------------------------------------------------------------*/

static void
LocateButton(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

    switch (gin->button) {

    case Button1:
	Locate(pls);
	break;
    }
}

/*--------------------------------------------------------------------------*\
 * Locate()
 *
 * Handles locate mode events.
 *
 * In locate mode: move cursor to desired location and select by pressing a
 * key or by clicking on the mouse (if available).  Typically the world
 * coordinates of the selected point are reported.
 *
 * There are two ways to enter Locate mode -- via the API, or via a driver
 * command.  The API entry point is the call plGetCursor(), which initiates
 * locate mode and does not return until input has been obtained.  The
 * driver entry point is by entering a 'L' while the driver is waiting for
 * events.
 *
 * Locate mode input is reported in one of three ways:
 * 1. Through a returned PLGraphicsIn structure, when user has specified a
 *    locate handler via (*pls->LocateEH).
 * 2. Through a returned PLGraphicsIn structure, when locate mode is invoked
 *    by a plGetCursor() call.
 * 3. Through writes to stdout, when locate mode is invoked by a driver
 *    command and the user has not supplied a locate handler.
 *
 * Hitting <Escape> will at all times end locate mode.  Other keys will
 * typically be interpreted as locator input.  Selecting a point out of
 * bounds will end locate mode unless the user overrides with a supplied
 * Locate handler.
\*--------------------------------------------------------------------------*/

static void
Locate(PLStream *pls)
{
    TkDev *dev = (TkDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

/* Call user locate mode handler if provided */

    if (pls->LocateEH != NULL)
	(*pls->LocateEH) (gin, pls->LocateEH_data, &dev->locate_mode);

/* Use default procedure */

    else {

    /* Try to locate cursor */

	if (plTranslateCursor(gin)) {

	/* If invoked by the API, we're done */
	/* Otherwise send report to stdout */

	    if (dev->locate_mode == LOCATE_INVOKED_VIA_DRIVER) {
		pltext();
		if (gin->keysym < 0xFF && isprint(gin->keysym))
		    printf("%f %f %c\n", gin->wX, gin->wY, gin->keysym);
		else
		    printf("%f %f 0x%02x\n", gin->wX, gin->wY, gin->keysym);

		plgra();
	    }
	}
	else {

	/* Selected point is out of bounds, so end locate mode */

	    dev->locate_mode = 0;
	    server_cmd( pls, "$plwidget configure -xhairs off", 1 );
	}
    }
}

/*--------------------------------------------------------------------------*\
 *
 * pltk_toplevel --
 *
 *	Create top level window without mapping it.
 *
 * Results:
 *	Returns 1 on error.
 *
 * Side effects:
 *	Returns window ID as *w.
 *
\*--------------------------------------------------------------------------*/

static int
pltk_toplevel(Tk_Window *w, Tcl_Interp *interp)
{
    static char wcmd[] = "wm withdraw .";

/* Create the main window without mapping it */

    if (Tk_Init( interp )) {
        fprintf(stderr,"tk_init:%s\n", interp->result);
	return 1;
    }

    Tcl_VarEval(interp, wcmd, (char *) NULL);

    return 0;
}

/*--------------------------------------------------------------------------*\
 * tk_wait()
 *
 * Waits for the specified expression to evaluate to true before
 * proceeding.  While we are waiting to proceed, all events (for this
 * or other interpreters) are handled.
 *
 * Use a static string buffer to hold the command, to ensure it's in
 * writable memory (grrr...).
\*--------------------------------------------------------------------------*/

static void
tk_wait(PLStream *pls, char *cmd)
{
    TkDev *dev = (TkDev *) pls->dev;
    int result = 0;

    dbug_enter("tk_wait");

    copybuf(pls, cmd);
    for (;;) {
	if (Tcl_ExprBoolean(dev->interp, dev->cmdbuf, &result)) {
	    fprintf(stderr, "tk_wait command \"%s\" failed:\n\t %s\n",
		    cmd, dev->interp->result);
	    break;
	}
	if (result)
	    break;

	Tk_DoOneEvent(0);
    }
}

/*--------------------------------------------------------------------------*\
 * server_cmd
 *
 * Sends specified command to server, aborting on an error.
 * If nowait is set, the command is issued in the background.
 *
 * If commands MUST proceed in a certain order (e.g. initialization), it
 * is safest to NOT run them in the background.
 *
 * In order to protect args that have embedded spaces in them, I enclose
 * the entire command in a [list ...], but for TK sends ONLY.  If done with
 * Tcl-DP RPC, the sent command is no longer recognized.  Evidently an
 * extra scan of the line is done with TK sends for some reason.
\*--------------------------------------------------------------------------*/

static void
server_cmd(PLStream *pls, char *cmd, int nowait)
{
    TkDev *dev = (TkDev *) pls->dev;
    static char dpsend_cmd0[] = "dp_RPC $server ";
    static char dpsend_cmd1[] = "dp_RDO $server ";
    static char tksend_cmd0[] = "send $server ";
    static char tksend_cmd1[] = "send $server after 1 ";
    int result;

    dbug_enter("server_cmd");
    pldebug("server_cmd", "Sending command: %s\n", cmd);

    if (pls->dp) {
	if (nowait)
	    result = Tcl_VarEval(dev->interp, dpsend_cmd1, cmd,
				 (char **) NULL);
	else
	    result = Tcl_VarEval(dev->interp, dpsend_cmd0, cmd,
				 (char **) NULL);
    }
    else {
	if (nowait)
	    result = Tcl_VarEval(dev->interp, tksend_cmd1, "[list ",
				 cmd, "]", (char **) NULL);
	else
	    result = Tcl_VarEval(dev->interp, tksend_cmd0, "[list ",
				 cmd, "]", (char **) NULL);
    }

    if (result != TCL_OK) {
	fprintf(stderr, "Server command \"%s\" failed:\n\t %s\n",
		cmd, dev->interp->result);
	abort_session(pls, "");
    }
}

/*--------------------------------------------------------------------------*\
 * tcl_cmd
 *
 * Evals the specified command, aborting on an error.
\*--------------------------------------------------------------------------*/

static void
tcl_cmd(PLStream *pls, char *cmd)
{
    TkDev *dev = (TkDev *) pls->dev;

    dbug_enter("tcl_cmd");

    pldebug("tcl_cmd", "Evaluating command: %s\n", cmd);
    if (Tcl_VarEval(dev->interp, cmd, (char **) NULL) != TCL_OK) {
	fprintf(stderr, "TCL command \"%s\" failed:\n\t %s\n",
		cmd, dev->interp->result);
	abort_session(pls, "");
    }
}

/*--------------------------------------------------------------------------*\
 * copybuf
 *
 * Puts command in a static string buffer, to ensure it's in writable
 * memory (grrr...).
\*--------------------------------------------------------------------------*/

static void
copybuf(PLStream *pls, char *cmd)
{
    TkDev *dev = (TkDev *) pls->dev;

    if (dev->cmdbuf == NULL) {
	dev->cmdbuf_len = 100;
	dev->cmdbuf = (char *) malloc(dev->cmdbuf_len);
    }

    if ((int) strlen(cmd) >= dev->cmdbuf_len) {
	free((void *) dev->cmdbuf);
	dev->cmdbuf_len = strlen(cmd) + 20;
	dev->cmdbuf = (char *) malloc(dev->cmdbuf_len);
    }

    strcpy(dev->cmdbuf, cmd);
}

/*--------------------------------------------------------------------------*/
#else
int
pldummy_tk()
{
    return 0;
}

#endif				/* PLD_tk */
