/*
  $Id: pbm.c 3186 2006-02-15 18:17:33Z slbrow $

  PLplot PBM (PPM) device driver.

  Contributed by John C. Atkinson and Zulfi Cumali.
  Slightly modified by Geoffrey Furnish.
*/
#include "plDevs.h"

#ifdef PLD_pbm

#include "plplotP.h"
#include "drivers.h"

/* Device info */
char* plD_DEVICE_INFO_pbm = "pbm:PDB (PPM) Driver:0:pbm:38:pbm";


void plD_dispatch_init_pbm	( PLDispatchTable *pdt );

void plD_init_pbm		(PLStream *);
void plD_line_pbm		(PLStream *, short, short, short, short);
void plD_polyline_pbm		(PLStream *, short *, short *, PLINT);
void plD_eop_pbm		(PLStream *);
void plD_bop_pbm		(PLStream *);
void plD_tidy_pbm		(PLStream *);
void plD_state_pbm		(PLStream *, PLINT);
void plD_esc_pbm		(PLStream *, PLINT, void *);

#undef PIXELS_X
#undef PIXELS_Y
#define PIXELS_X 640
#define PIXELS_Y 480

static char *cmap;

#undef MAX
#undef ABS
#define MAX(a,b) ((a>b) ? a : b)
#define ABS(a) ((a<0) ? -a : a)

#define MAX_INTENSITY 255

void plD_dispatch_init_pbm( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "PDB (PPM) Driver";
    pdt->pl_DevName  = "pbm";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 38;
    pdt->pl_init     = (plD_init_fp)     plD_init_pbm;
    pdt->pl_line     = (plD_line_fp)     plD_line_pbm;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_pbm;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_pbm;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_pbm;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_pbm;
    pdt->pl_state    = (plD_state_fp)    plD_state_pbm;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_pbm;
}

/*--------------------------------------------------------------------------*\
 * plD_init_pbm()
 *
 * Initialize device (terminal).
\*--------------------------------------------------------------------------*/

void
plD_init_pbm(PLStream *pls)
{
#if 1

/* Initialize family file info */

    plFamInit(pls);

    plP_setpxl((PLFLT) 5.905, (PLFLT) 5.905);

#endif

    pls->color = 1;		/* Is a color device */
    pls->dev_fill0 = 0;		/* Handle solid fills */
    pls->dev_fill1 = 0;		/* Handle pattern fills */
    pls->nopause = 1;		/* Don`t pause between frames */

/* Prompt for a file name if not already set */

    plOpenFile(pls);
    pls->pdfs = pdf_finit(pls->OutFile);

/* Allocate and initialize device-specific data */

    pls->dev = NULL;

/* Set up device parameters */

    if (pls->xlength <= 0 || pls->ylength <= 0) {
      plspage(0., 0., PIXELS_X, PIXELS_Y, 0, 0);
    }

    plP_setphy(0, pls->xlength, 0, pls->ylength);
}

#if 0
void
plD_line_pbm(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    int steps, i, dx, dy;
    double x_off, y_off, dx_step, dy_step;

/* This algoritm is by Steven Harrington
   From "Computer Graphics: A Proogramming Approach */

    dx = x2a - x1a;
    dy = y2a - y1a;
    steps = MAX(ABS(dx), ABS(dy)) + 1;
    steps *= 2;
    dx_step = dx/steps;
    dy_step = dy/steps;
    x_off = x1a + 0.5;
    y_off = y1a + 0.5;

    for (i = 0; i < steps; i++) {
	cmap[(int)y_off][(int)x_off][0] = pls->curcolor.r;
	cmap[(int)y_off][(int)x_off][1] = pls->curcolor.g;
	cmap[(int)y_off][(int)x_off][2] = pls->curcolor.b;
	x_off += dx_step;
	y_off += dy_step;
    }

    cmap[(int)y_off][(int)x_off][0] = pls->curcolor.r;
    cmap[(int)y_off][(int)x_off][1] = pls->curcolor.g;
    cmap[(int)y_off][(int)x_off][2] = pls->curcolor.b;

    return;
}
#endif

#define sign(a) ((a<0) ? -1 : ((a == 0) ? 0 : 1))

#if 0
#define plot(x,y,c) {cmap[y-1][x-1][0] = (c)->curcolor.r; \
					 cmap[y-1][x-1][1] = (c)->curcolor.g; \
					 cmap[y-1][x-1][2] = (c)->curcolor.b; }

/* Bresnham's  algorithm for line plotting on a scan lines */

void
plD_line_pbm(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    int e, x, y, dx, dy, s1, s2, temp, change, i;

    x = x1a;
    y = y1a;

    dx = ABS(x2a - x1a);
    dy = ABS(y2a - y1a);
    s1 = sign(x2a - x1a);
    s2 = sign(y2a - y1a);

    if( dy > dx ){
	temp= dx;
	dx  = dy;
	dy  = temp;
	change = 1;
    }
    else {
	change = 0;
    }
    e = 2 * dy - dx;

    for( i=1; i<dx;i++){
	plot(x,y,pls);
	while( e>= 0 ) {
	    if(change == 1) x += s1;
	    else            y += s2;		
	    e = e - 2 * dx;
	}
	if(change == 1) y += s2;
	else            x += s1;
	e = e + 2 * dy;
    }
}
#else
#define plot(x,y,c) {int i = 3*((y)*(c)->xlength+(x)); \
                     cmap[i+0] = (c)->curcolor.r; \
		     cmap[i+1] = (c)->curcolor.g; \
		     cmap[i+2] = (c)->curcolor.b; }

/* Modified version of the ljii routine (see ljii.c) */
void
plD_line_pbm(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    int i;
    int x1 = x1a, y1 = y1a, x2 = x2a, y2 = y2a;
    PLINT x1b, y1b, x2b, y2b;
    PLFLT length, fx, fy, dx, dy;

/* Take mirror image, since PCL expects (0,0) to be at top left */

    y1 = pls->ylength - (y1 - 0);
    y2 = pls->ylength - (y2 - 0);

    x1b = x1, x2b = x2, y1b = y1, y2b = y2;
    length = (PLFLT) sqrt((double)
			  ((x2b - x1b) * (x2b - x1b) + (y2b - y1b) * (y2b - y1b)));

    if (length == 0.)
	length = 1.;
    dx = (x2 - x1) / length;
    dy = (y2 - y1) / length;

    fx = x1;
    fy = y1;
    plot((PLINT) x1, (PLINT) y1, pls);
    plot((PLINT) x2, (PLINT) y2, pls);

    for (i = 1; i <= (int) length; i++) {
	fx += dx; fy += dy;
	plot((PLINT) fx, (PLINT) fy, pls);
    }
}
#endif

void
plD_polyline_pbm(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    int i;
    for (i=0; i<npts - 1; i++)
	plD_line_pbm(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]); 
}

void
plD_eop_pbm(PLStream *pls)
{
    FILE *fp = pls->OutFile;

    if (fp != NULL) {
	fprintf(fp, "%s\n", "P6");
	fprintf(fp, "%d %d\n", pls->xlength, pls->ylength);
	fprintf(fp, "%d\n", MAX_INTENSITY);
    /*
	{
	    int i, j, k;
	    for (i=0; i<PIXELS_Y; i++)
		for (j=0; j<PIXELS_X; j++)
		    for (k=0; k<3; k++)
			fprintf(fp, "%c", cmap[i][j][k]);
	}
    */
	fwrite( cmap, 1, pls->xlength * pls->ylength * 3, fp );

	fclose(fp);
    } 
    free(cmap);
    cmap = 0;
}

void
plD_bop_pbm(PLStream *pls)
{
  int i,j,k;
  cmap = (char*)malloc(pls->xlength*pls->ylength*3);
  for (i=0; i<pls->ylength; i++)
    for (j=0; j<pls->xlength; j++) {
      k = (i*pls->xlength + j)*3;
      cmap[k+0] = pls->cmap0[0].r;
      cmap[k+1] = pls->cmap0[0].g;
      cmap[k+2] = pls->cmap0[0].b;
    }
}

void
plD_tidy_pbm(PLStream *pls)
{
/* Nothing to do here */
}

void 
plD_state_pbm(PLStream *pls, PLINT op)
{
/* Nothing to do here */
}

void
plD_esc_pbm(PLStream *pls, PLINT op, void *ptr)
{
/* Nothing to do here */
}

#else
int 
pldummy_pbm()
{
    return 0;
}

#endif				/* PLD_pbm */


