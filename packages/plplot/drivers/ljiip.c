/* $Id: ljiip.c 3186 2006-02-15 18:17:33Z slbrow $

	PLplot Laser Jet IIp device driver.
	Based on old LJII driver, modifications by Wesley Ebisuzaki

	DPI = 300, 150, 100	(dots per inch)
		default: Unix 300 dpi, MS-DOS 150 dpi
		higher = better output, more memory, longer to print
	GCMODE = 0, 2 (graphics compression mode)
		default: 2,  old laser jets should use 0
		compression can speed up the printing up to 3x

*/
#include "plDevs.h"

#ifdef PLD_ljiip

#include "plplotP.h"
#include "drivers.h"
#include <math.h>
#include <string.h>

#ifdef __GO32__			/* dos386/djgpp */
#ifdef MSDOS
#undef MSDOS
#endif
#endif

/* Device info */
char* plD_DEVICE_INFO_ljiip =
  "ljiip:LaserJet IIp/deskjet compressed graphics:0:ljiip:32:ljiip";

/* Function prototypes */

void plD_dispatch_init_ljiip	( PLDispatchTable *pdt );

void plD_init_ljiip		(PLStream *);
void plD_line_ljiip		(PLStream *, short, short, short, short);
void plD_polyline_ljiip		(PLStream *, short *, short *, PLINT);
void plD_eop_ljiip		(PLStream *);
void plD_bop_ljiip		(PLStream *);
void plD_tidy_ljiip		(PLStream *);
void plD_state_ljiip		(PLStream *, PLINT);
void plD_esc_ljiip		(PLStream *, PLINT, void *);

static void setpoint(PLINT, PLINT);

/* top level declarations */

/* GCMODE = graphics compression mode, 0=old laser jets, 2=ljiip or later */
#define GCMODE   2

/* DPI = dots per inch, more dots=better plot, more memory, more time */
/* possible DPI = 75, 100, 150, 300 */
/* if DPI=300 then your machine must have a free 1Mb block of memory */

#define DPI      300

#ifdef MSDOS
#undef  DPI
#define DPI      150
#endif

#define OF	pls->OutFile
#define CURX	((long) (DPI / 5))
#define CURY	((long) (DPI / 7))
#define XDOTS	(376 * (DPI / 50))	/* # dots across */
#define YDOTS	(500 * (DPI / 50))	/* # dots down */
#define JETX    (XDOTS-1)
#define JETY    (YDOTS-1)


#define BPROW	(XDOTS/8L)		/* # bytes across */
#define MAX_WID	8			/* max pen width in pixels */
#define BPROW1	(BPROW + (MAX_WID+7)/8)	/* pen has width, make bitmap bigger */
#define NBYTES	BPROW1*(YDOTS+MAX_WID)	/* total number of bytes */

/* Graphics control characters. */

#define ESC      0x1b
#define FF       0x0c

static char mask[8] =
{'\200', '\100', '\040', '\020', '\010', '\004', '\002', '\001'};

#ifndef MSDOS
#define _HUGE
#else
#define _HUGE _huge
#endif

static unsigned char _HUGE *bitmap;	/* memory area NBYTES in size */

void plD_dispatch_init_ljiip( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "LaserJet IIp/deskjet compressed graphics";
    pdt->pl_DevName  = "ljiip";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 32;
    pdt->pl_init     = (plD_init_fp)     plD_init_ljiip;
    pdt->pl_line     = (plD_line_fp)     plD_line_ljiip;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_ljiip;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_ljiip;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_ljiip;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_ljiip;
    pdt->pl_state    = (plD_state_fp)    plD_state_ljiip;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_ljiip;
}

/*--------------------------------------------------------------------------*\
 * plD_init_ljiip()
 *
 * Initialize device.
\*--------------------------------------------------------------------------*/

void
plD_init_ljiip(PLStream *pls)
{
    PLDev *dev;

    if (pls->width == 0)	/* Is 0 if uninitialized */
	pls->width = DPI / 100;

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

/* number of pixels / mm */

    plP_setpxl((PLFLT) (DPI/25.4), (PLFLT) (DPI/25.4));

/* Rotate by 90 degrees since portrait mode addressing is used */

    dev->xmin = 0;
    dev->ymin = 0;
    dev->xmax = JETY;
    dev->ymax = JETX;
    dev->xlen = dev->xmax - dev->xmin;
    dev->ylen = dev->ymax - dev->ymin;

    plP_setphy(dev->xmin, dev->xmax, dev->ymin, dev->ymax);

/* If portrait mode is specified, then set up an additional rotation 
 * transformation with aspect ratio allowed to adjust via freeaspect.  
 * Default orientation is landscape (ORIENTATION == 3 or 90 deg rotation 
 * counter-clockwise from portrait).  (Legacy PLplot used seascape
 * which was equivalent to ORIENTATION == 1 or 90 deg clockwise rotation 
 * from portrait.) */

    if (pls->portrait) {
       plsdiori((PLFLT)(4 - ORIENTATION));
       pls->freeaspect = 1;
    }

/* Allocate storage for bit map matrix */

#ifdef MSDOS
    if ((bitmap = (unsigned char _HUGE *)
	 halloc((long) NBYTES, sizeof(char))) == NULL)
	plexit("Out of memory in call to calloc");
#else
    if ((bitmap = (unsigned char *) calloc(NBYTES, sizeof(char))) == NULL)
	plexit("Out of memory in call to calloc");
#endif

/* Reset Printer */

    fprintf(OF, "%cE", ESC);
}

/*--------------------------------------------------------------------------*\
 * plD_line_ljiip()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_line_ljiip(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    PLDev *dev = (PLDev *) pls->dev;
    int x1 = x1a, y1 = y1a, x2 = x2a, y2 = y2a;
    int abs_dx, abs_dy, dx, dy, incx, incy;
    int i, j, width, residual;
    PLFLT tmp;

    width = MIN(pls->width, MAX_WID);
 
/* Take mirror image, since PCL expects (0,0) to be at top left */

    y1 = dev->ymax - (y1 - dev->ymin);
    y2 = dev->ymax - (y2 - dev->ymin);

/* Rotate by 90 degrees */

    plRotPhy(ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax, &x1, &y1);
    plRotPhy(ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax, &x2, &y2);

    dx = x2 - x1;
    dy = y2 - y1;

    if (dx < 0) {
	abs_dx = -dx;
	incx = -1;
    }
    else {
	abs_dx = dx;
	incx = 1;
    }
    if (dy < 0) {
	abs_dy = -dy;
	incy = -1;
    }
    else {
	abs_dy = dy;
	incy = 1;
    }

/* make pixel width narrower for diag lines */

    if (abs_dy <= abs_dx) {
	if (abs_dx == 0)
	    tmp = 1.0;
	else
	    tmp = 1.0 - (PLFLT) abs_dy / abs_dx;
    }
    else {
	tmp = 1.0 - (PLFLT) abs_dx / abs_dy;
    }

    width = floor(0.5 + width * (tmp*tmp*tmp*(1.0-0.707107) + 0.707107));

    if (width < 1) width = 1;
    if (width > 1) {
	for (i = 0; i < width; i++) {
	    for (j = 0; j < width; j++) {
	        setpoint((PLINT) (x1+i), (PLINT) (y1+j));
	        setpoint((PLINT) (x2+i), (PLINT) (y2+j));
	    }
	}
    }
    if (abs_dx >= abs_dy) {
	residual = -(abs_dx >> 1);
	if (width == 1) {
            for (i = 0; i <= abs_dx; i++, x1 += incx) {
                setpoint((PLINT) (x1), (PLINT) (y1));
                if ((residual += abs_dy) >= 0) {
                    residual -= abs_dx;
                    y1 += incy;
                }
            }
	}
	else {
	    for (i = 0; i <= abs_dx; i++, x1 += incx) {
	       for (j = 0; j < width; j++) {
	           setpoint((PLINT) (x1), (PLINT) (y1+j));
	           setpoint((PLINT) (x1+width-1), (PLINT) (y1+j));
	       }
	       if ((residual += abs_dy) >= 0) {
		   residual -= abs_dx;
		   y1 += incy;
	       }
	    }
	}
    }
    else {
	residual = -(abs_dy >> 1);
        if (width == 1) {
            for (i = 0; i <= abs_dy; i++, y1 += incy) {
                setpoint((PLINT) (x1), (PLINT) (y1));
                if ((residual += abs_dx) >= 0) {
                    residual -= abs_dy;
                    x1 += incx;
                }
            }
        }
        else {
            for (i = 0; i <= abs_dy; i++, y1 += incy) {
               for (j = 0; j < width; j++) {
                   setpoint((PLINT) (x1+j), (PLINT) (y1));
                   setpoint((PLINT) (x1+j), (PLINT) (y1+width-1));
               }
               if ((residual += abs_dx) >= 0) {
                   residual -= abs_dy;
                   x1 += incx;
               }
            }
        }
    }
}

/*--------------------------------------------------------------------------*\
 * plD_polyline_ljiip()
 *
 * Draw a polyline in the current color.
\*--------------------------------------------------------------------------*/

void
plD_polyline_ljiip(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i = 0; i < npts - 1; i++)
	plD_line_ljiip(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}

/*--------------------------------------------------------------------------*\
 * plD_eop_ljiip()
 *
 * End of page.(prints it here).
\*--------------------------------------------------------------------------*/

void
plD_eop_ljiip(PLStream *pls)
{
    PLINT j;
    unsigned char _HUGE *p;
#if GCMODE > 0
    int i, iy, last, n, jmax;
    unsigned char _HUGE t_buf[BPROW*2];
    unsigned char c;
#endif

/* PCL III setup: ref. Deskjet Plus Printer Owner's Manual */

    fprintf(OF,"\033*rB");      	/* end raster graphics */
    fprintf(OF,"\033*t%3dR", DPI);	/* set DPI */

#if GCMODE != 0
    fprintf(OF,"\033*r%dS", XDOTS);	/* raster width */
    fprintf(OF,"\033*b%1dM", GCMODE);	/* graphics mode */
#endif

/* First move cursor to origin */

    fprintf(OF,"\033*p%ldX", CURX);
    fprintf(OF,"\033*p%ldY", CURY);
    fprintf(OF,"\033*r0A");		/* start graphics */

/* Write out raster data */

#if GCMODE == 0
    for (j = 0, p = bitmap; j < YDOTS; j++, p += BPROW1) {
	fprintf(OF,"\033*b>%dW", BPROW);
	fwrite(p, BPROW, sizeof(char), OF);
    }
#endif
#if GCMODE == 2
    for (iy = 0, p = bitmap; iy < YDOTS; iy++, p += BPROW1) {

    /* find last non-zero byte */

	last = BPROW - 1;
	while (last > 0 && p[last] == 0) last--;
	last++;

    /* translate to mode 2, save results in t_buf[] */

	i = n = 0;
	while (i < last) {
	    c = p[i];
	    jmax = i + 127;
	    jmax = last < jmax ? last : jmax;
	    if (i < last - 2 && (c == p[i+1]) && (c == p[i+2])) {
		j = i + 3;
		while (j < jmax && c == p[j]) j++;
		t_buf[n++] = (i-j+1) & 0xff;
		t_buf[n++] = c;
		i = j;
	    }
	    else {
		for (j = i + 1; j < jmax; j++) {
		    if (j < last - 2 && (p[j] == p[j+1]) &&
			(p[j+1] == p[j+2]) ) break;
		}
		t_buf[n++] = j - i - 1;
		while (i < j) {
		    t_buf[n++] = p[i++];
		}
	    }
	}
	fprintf(OF,"\033*b%dW", (int) n);
	fwrite(t_buf, (int) n, sizeof(char), OF);
    }
#endif

    pls->bytecnt += NBYTES;

/* End raster graphics and send Form Feed */

    fprintf(OF, "\033*rB");
    fprintf(OF, "%c", FF);

/* Finally, clear out bitmap storage area */

    memset((void *) bitmap, '\0', NBYTES);
}

/*--------------------------------------------------------------------------*\
 * plD_bop_ljiip()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*--------------------------------------------------------------------------*/

void
plD_bop_ljiip(PLStream *pls)
{
    if (!pls->termin)
	plGetFam(pls);

    pls->page++;
}

/*--------------------------------------------------------------------------*\
 * plD_tidy_ljiip()
 *
 * Close graphics file or otherwise clean up.
\*--------------------------------------------------------------------------*/

void
plD_tidy_ljiip(PLStream *pls)
{
/* Reset Printer */

    fprintf(OF, "%cE", ESC);
    fclose(OF);
    free((char *) bitmap);
}

/*--------------------------------------------------------------------------*\
 * plD_state_ljiip()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*--------------------------------------------------------------------------*/

void 
plD_state_ljiip(PLStream *pls, PLINT op)
{
}

/*--------------------------------------------------------------------------*\
 * plD_esc_ljiip()
 *
 * Escape function.
\*--------------------------------------------------------------------------*/

void
plD_esc_ljiip(PLStream *pls, PLINT op, void *ptr)
{
}

/*--------------------------------------------------------------------------*\
 * setpoint()
 *
 * Sets a bit in the bitmap.
\*--------------------------------------------------------------------------*/

static void
setpoint(PLINT x, PLINT y)
{
    PLINT index;
    index = x / 8 + y * BPROW1;
    *(bitmap + index) = *(bitmap + index) | mask[x % 8];
}

#else
int 
pldummy_ljiip()
{
    return 0;
}

#endif				/* PLD_ljii */
