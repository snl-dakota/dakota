/* $Id: plserver.h 3186 2006-02-15 18:17:33Z slbrow $
 *
 * Maurice LeBrun
 * 6-May-93
 *
 * Declarations for plserver and associated files.  
 */

#ifndef __PLSERVER_H__
#define __PLSERVER_H__

#include "plDevs.h"
#include "pltkd.h"
#include "pltcl.h"

/* State info for the rendering code */

typedef struct {
    char  *client;			/* Name of client main window */
    PDFstrm *pdfs;			/* PDF stream descriptor */
    PLiodev *iodev;			/* I/O info */
    int   nbytes;			/* data bytes waiting to be read */
    int   at_bop, at_eop;		/* bop/eop condition flags */

    short xmin, xmax, ymin, ymax;	/* Data minima and maxima */
    PLFLT xold, yold;			/* Endpoints of last line plotted */
} PLRDev;

/* External function prototypes. */
/* Note that tcl command functions are added during Pltk_Init and don't */
/* need to be called directly by the user */

/* plframe.c */
/* Invoked to process the "plframe" Tcl command. */

int
plFrameCmd(ClientData clientData, Tcl_Interp *interp,
	   int argc, char **argv);
int
plPlotterCmd(ClientData clientData, Tcl_Interp *interp,
	   int argc, char **argv);

/* plr.c */
/* Set default state parameters before anyone else has a chance to. */

void
plr_start(PLRDev *plr);

/* Read & process commands until "nbyte_max" bytes have been read. */

int
plr_process(PLRDev *plr);

#endif	/* __PLSERVER_H__ */
