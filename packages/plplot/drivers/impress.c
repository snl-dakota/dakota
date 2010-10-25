/* $Id: impress.c 3186 2006-02-15 18:17:33Z slbrow $

	PLplot ImPress device driver.
*/
#include "plDevs.h"

#ifdef PLD_imp

#include "plplotP.h"
#include "drivers.h"

/* Device info */
char* plD_DEVICE_INFO_impress = "imp:Impress File:0:impress:37:imp";

/* Function prototypes */

void plD_dispatch_init_imp	( PLDispatchTable *pdt );

void plD_init_imp		(PLStream *);
void plD_line_imp		(PLStream *, short, short, short, short);
void plD_polyline_imp		(PLStream *, short *, short *, PLINT);
void plD_eop_imp		(PLStream *);
void plD_bop_imp		(PLStream *);
void plD_tidy_imp		(PLStream *);
void plD_state_imp		(PLStream *, PLINT);
void plD_esc_imp		(PLStream *, PLINT, void *);

static void flushline(PLStream *);

/* top level declarations */

#define IMPX		2999
#define IMPY		2249

#define BUFFPTS		50
#define BUFFLENG	2*BUFFPTS

/* Graphics control characters. */

#define SET_HV_SYSTEM	0315
#define OPBYTE1		031
#define OPBYTE2		0140
#define SET_ABS_H	0207
#define SET_ABS_V	0211
#define OPWORDH1	0
#define OPWORDH2	150
#define OPWORDV1	0
#define OPWORDV2	150
#define ENDPAGE		0333

#define SET_PEN		0350
#define CREATE_PATH	0346
#define DRAW_PATH	0352
#define OPTYPE		017

static short *LineBuff;
static short FirstLine;
static int penchange = 0, penwidth = 1;
static short count;

void plD_dispatch_init_imp( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "Impress File";
    pdt->pl_DevName  = "imp";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 37;
    pdt->pl_init     = (plD_init_fp)     plD_init_imp;
    pdt->pl_line     = (plD_line_fp)     plD_line_imp;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_imp;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_imp;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_imp;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_imp;
    pdt->pl_state    = (plD_state_fp)    plD_state_imp;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_imp;
}

/*--------------------------------------------------------------------------*\
 * plD_init_imp()
 *
 * Initialize device (terminal).
\*--------------------------------------------------------------------------*/

void
plD_init_imp(PLStream *pls)
{
    PLDev *dev;

/* Initialize family file info */

    plFamInit(pls);

/* Prompt for a file name if not already set */

    plOpenFile(pls);

/* Allocate and initialize device-specific data */

    dev = plAllocDev(pls);

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;
    dev->xmin = 0;
    dev->ymin = 0;
    dev->xmax = IMPX;
    dev->ymax = IMPY;
    dev->xlen = dev->xmax - dev->xmin;
    dev->ylen = dev->ymax - dev->ymin;

    plP_setpxl((PLFLT) 11.81, (PLFLT) 11.81);
    plP_setphy(dev->xmin, dev->xmax, dev->ymin, dev->ymax);

    LineBuff = (short *) malloc(BUFFLENG * sizeof(short));
    if (LineBuff == NULL) {
	plexit("Error in memory alloc in plD_init_imp().");
    }
    fprintf(pls->OutFile, "@Document(Language ImPress, jobheader off)");
    fprintf(pls->OutFile, "%c%c", SET_HV_SYSTEM, OPBYTE1);
    fprintf(pls->OutFile, "%c%c%c", SET_ABS_H, OPWORDH1, OPWORDH2);
    fprintf(pls->OutFile, "%c%c%c", SET_ABS_V, OPWORDV1, OPWORDV2);
    fprintf(pls->OutFile, "%c%c", SET_HV_SYSTEM, OPBYTE2);
}

/*--------------------------------------------------------------------------*\
 * plD_line_imp()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_line_imp(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    PLDev *dev = (PLDev *) pls->dev;
    int x1 = x1a, y1 = y1a, x2 = x2a, y2 = y2a;

    if (FirstLine) {
	if (penchange) {
	    fprintf(pls->OutFile, "%c%c", SET_PEN, (char) penwidth);
	    penchange = 0;
	}

    /* Add both points to path */

	count = 0;
	FirstLine = 0;
	*(LineBuff + count++) = (short) x1;
	*(LineBuff + count++) = (short) y1;
	*(LineBuff + count++) = (short) x2;
	*(LineBuff + count++) = (short) y2;
    }
    else if ((count + 2) < BUFFLENG && x1 == dev->xold && y1 == dev->yold) {

    /* Add new point to path */

	*(LineBuff + count++) = (short) x2;
	*(LineBuff + count++) = (short) y2;
    }
    else {

    /* Write out old path */

	count /= 2;
	fprintf(pls->OutFile, "%c%c%c", CREATE_PATH, (char) count / 256, (char) count % 256);
	fwrite((char *) LineBuff, sizeof(short), 2 * count, pls->OutFile);
	fprintf(pls->OutFile, "%c%c", DRAW_PATH, OPTYPE);

    /* And start a new path */

	if (penchange) {
	    fprintf(pls->OutFile, "%c%c", SET_PEN, (char) penwidth);
	    penchange = 0;
	}
	count = 0;
	*(LineBuff + count++) = (short) x1;
	*(LineBuff + count++) = (short) y1;
	*(LineBuff + count++) = (short) x2;
	*(LineBuff + count++) = (short) y2;
    }
    dev->xold = x2;
    dev->yold = y2;
}

/*--------------------------------------------------------------------------*\
 * plD_polyline_imp()
 *
 * Draw a polyline in the current color.
\*--------------------------------------------------------------------------*/

void
plD_polyline_imp(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i = 0; i < npts - 1; i++)
	plD_line_imp(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}

/*--------------------------------------------------------------------------*\
 * plD_eop_imp()
 *
 * End of page.
\*--------------------------------------------------------------------------*/

void
plD_eop_imp(PLStream *pls)
{
    flushline(pls);
    fprintf(pls->OutFile, "%c", ENDPAGE);
}

/*--------------------------------------------------------------------------*\
 * plD_bop_imp()
 *
 * Set up for the next page.
\*--------------------------------------------------------------------------*/

void
plD_bop_imp(PLStream *pls)
{
    PLDev *dev = (PLDev *) pls->dev;

    FirstLine = 1;
    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;

    if (!pls->termin)
	plGetFam(pls);

    pls->page++;
}

/*--------------------------------------------------------------------------*\
 * plD_tidy_imp()
 *
 * Close graphics file or otherwise clean up.
\*--------------------------------------------------------------------------*/

void
plD_tidy_imp(PLStream *pls)
{
    free((void *) LineBuff);
    fclose(pls->OutFile);
}

/*--------------------------------------------------------------------------*\
 * plD_state_imp()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*--------------------------------------------------------------------------*/

void 
plD_state_imp(PLStream *pls, PLINT op)
{
    switch (op) {

    case PLSTATE_WIDTH:
	if (pls->width > 0 && pls->width <= 20) {
	    penwidth = pls->width;
	    penchange = 1;
	}
	break;

    case PLSTATE_COLOR0:
	break;

    case PLSTATE_COLOR1:
	break;
    }
}

/*--------------------------------------------------------------------------*\
 * plD_esc_imp()
 *
 * Escape function.
\*--------------------------------------------------------------------------*/

void
plD_esc_imp(PLStream *pls, PLINT op, void *ptr)
{
}

/*--------------------------------------------------------------------------*\
 * flushline()
 *
 * Spits out the line buffer.
\*--------------------------------------------------------------------------*/

static void
flushline(PLStream *pls)
{
    count /= 2;
    fprintf(pls->OutFile, "%c%c%c", CREATE_PATH, (char) count / 256, (char) count % 256);
    fwrite((char *) LineBuff, sizeof(short), 2 * count, pls->OutFile);
    fprintf(pls->OutFile, "%c%c", DRAW_PATH, OPTYPE);
    FirstLine = 1;
}

#else
int 
pldummy_impress()
{
    return 0;
}

#endif				/* PLD_imp */
