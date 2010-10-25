/* $Id: gnusvga.c 3186 2006-02-15 18:17:33Z slbrow $
*/

#include "plDevs.h"

#if defined(GRX_DO_TIFF) || defined(GRX_DO_BMP) || defined(GRX_DO_JPEG) || defined (PLD_gnusvga) || defined(PLD_bmp) || defined(PLD_jpg) || defined(PLD_tiff)

#include "plplotP.h"
#include "drivers.h"
#include "plevent.h"

#include <grx20.h>

#ifdef HAVE_FREETYPE

/*
 *  Freetype support has been added to the GNUSVGA family of drivers using
 *  the plfreetype.c module, and implemented as a driver-specific optional
 *  extra invoked via the -drvopt command line toggle. It uses the
 *  "PLESC_HAS_TEXT" command for rendering within the driver.
 *
 *  Freetype support is turned on/off at compile time by defining
 *  "HAVE_FREETYPE".
 *
 *  To give the user some level of control over the fonts that are used,
 *  environmental variables can be set to over-ride the definitions used by
 *  the five default plplot fonts.
 *
 *  Freetype rendering is used with the command line "-drvopt text".
 *  Anti-aliased fonts can be used by issuing "-drvopt text,smooth"
 *
 *  Freetype rendering, and smoothing, can be turned on by default by
 *  setting any of the following environmental variables to 1:
 *  PLPLOT_GNUSVGA_TEXT,PLPLOT_GNUSVGA_SMOOTH,PLPLOT_TIFF_TEXT,
 *  PLPLOT_TIFF_SMOOTH,PLPLOT_JPG_TEXT PLPLOT_JPG_SMOOTH,PLPLOT_BMP_TEXT,
 *  and PLPLOT_BMP_SMOOTH.
 *
 *  If it has been set by default from enviro variables, it can be turned
 *  off on the command line like thus: "-drvopt text=0,smooth=0"
 */

#include "plplot/plfreetype.h"

#endif

/* In an attempt to fix a problem with the hidden line removal functions
 * that results in hidden lines *not* being removed from "small" plot
 * pages (ie, like a normal video screen), a "virtual" page of much
 * greater size is used to trick the algorithm into working correctly.
 * If, in future, this gets fixed on its own, then don't define
 * "use_experimental_hidden_line_hack"
 */

#define use_experimental_hidden_line_hack

#define isTrueColour() (GrNumColors()>256)

#ifndef bzero   /* not stanrard ANSI (boo hoo) */
#define bzero(a,b) memset(a,0,b)
#endif

#include <time.h>       /* for the 'newname' funtion */

#ifdef PLD_gnusvga
#undef _POSIX_SOURCE    /* Turn off POSIX to get around "problem" with <keys.h> header.
			   Ugly, but it works *sigh* */
#include <keys.h>
#endif

#include <strings.h>


/* Prototypes:  Since GNU CC, we can rest in the safety of ANSI prototyping. */

void plD_line_vga		(PLStream *, short, short, short, short);
void plD_polyline_vga		(PLStream *, short *, short *, PLINT);
void plD_state_vga		(PLStream *, PLINT);

#ifdef HAVE_FREETYPE

static void init_freetype_lv1 (PLStream *pls);
static void init_freetype_lv2 (PLStream *pls);
static void plD_pixel_vga (PLStream *, short, short);

extern void plD_FreeType_init(PLStream *pls);
extern void plD_render_freetype_text (PLStream *pls, EscText *args);
extern void plD_FreeType_Destroy(PLStream *pls);
extern void pl_set_extended_cmap0(PLStream *pls, int ncol0_width, int ncol0_org);

#endif

#ifdef PLD_gnusvga

static void     plpause         (PLStream *);
static void     svga_text       (PLStream *);
static void     svga_graph      (PLStream *);
static void     WaitForPage     (PLStream *pls);
static void     EventHandler    (PLStream *pls, GrMouseEvent *event);
static void     TranslateEvent  (PLStream *pls, GrMouseEvent *event, PLGraphicsIn *gin);

void plD_init_vga		(PLStream *);
void plD_eop_vga		(PLStream *);
void plD_eop_jpeg		(PLStream *);
void plD_bop_vga		(PLStream *);
void plD_tidy_vga		(PLStream *);
void plD_esc_vga		(PLStream *, PLINT, void *);

void plD_dispatch_init_vga( PLDispatchTable *pdt )
{
    pdt->pl_MenuStr  = "SVGA Screen (GRX20)";
    pdt->pl_DevName  = "vga";
    pdt->pl_type     = plDevType_Interactive;
    pdt->pl_seq      = 13;
    pdt->pl_init     = (plD_init_fp)     plD_init_vga;
    pdt->pl_line     = (plD_line_fp)     plD_line_vga;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_vga;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_vga;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_vga;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_vga;
    pdt->pl_state    = (plD_state_fp)    plD_state_vga;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_vga;
}

#endif

static void     fill_polygon    (PLStream *pls);
static void     setcmap         (PLStream *pls);
static void     plD_init_gnu_grx_dev(PLStream *pls);
static void     XorMod          (PLStream *pls, PLINT *mod);

#if defined(GRX_DO_TIFF) || defined(GRX_DO_BMP) || defined(GRX_DO_JPEG) || defined(PLD_tiff) || defined(PLD_jpg) || defined(PLD_bmp)
#if GRX_VERSION_API >= 0x0229
char            *newname        (char *ext);
#endif
#endif

/* gmf; should probably query this on start up... Maybe later. */

static exit_eventloop = 0;

#define CLEAN 0
#define DIRTY 1

static page_state;

/*
 *  The "device structure" for the gnusvga driver holding all sorts of stuff
 *  that once would have been "global".
 */

typedef struct {

#ifdef PLD_gnusvga

	PLGraphicsIn gin;                       /* Graphics input structure     */
	GrMouseEvent mevent;                    /* mouse event handler          */
	GrLineOption gnusvgaline;               /* Line Options                 */
	GrContext *switch_screen;               /* A copy of the GRX screen in case of switching */
	GrContext *double_buffer;               /* Screen pointer for double buffering  */
	GrContext *visual_screen;               /* Screen pointer for visual screen  */
	GrContext *top_line;                    /* Screen pointer for top line  */

#ifdef HAVE_FREETYPE
        FT_Data   FT;
#endif

/*
 *  Originally I didn't realise it was possible to "XOR" a line, so I
 *  blitted the cross hair in. Then by accident (I read the instructions,
 *  for something else I wanted to do, and stumbled across it !),
 *  I found out how to XOR a line, so got rid of the blitting. I have kept
 *  it in case it's quicker. It can be put back in a jiff by defining
 *  "BLIT_CROSSHAIR".
 */

#ifdef BLIT_CROSSHAIR
	GrContext *Xhair_X;                    /* Screen pointer for XhairX line  */
	GrContext *Xhair_Y;                    /* Screen pointer for XhairY line  */
#endif

#endif
	GrVideoDriver *Old_Driver_Vector;       /* Pointer for old driver      */

	PLINT vgax;
	PLINT vgay;

	int scale;                              /* scaling factor to "blow up" to */
	                                        /* the "virtual" page in removing hidden lines*/

	int colour;                             /* Current Colour               */
	int totcol;                             /* Total number of colours      */
	int fg;                                 /* Pointer (in colour index) to a "safe" foreground colour */

	int last_x;                             /* Used to "wipe out" the X-Hair */
	int last_y;                             /* Used to "wipe out" the X-Hair */
	int locate_mode;                        /* Set while in locate mode     */
	int draw_mode;                          /* used by XOR mode                 */

	char dont_copy_screen;                  /* set to non-zero before existing */
	char Xhair_on;                          /* set if the cross hair is on and displayed */
	char toggle_xhair;                      /* set if the cross hair is being "toggled" off for fixing top of screen */

	     } gnu_grx_Dev;

#ifdef PLD_gnusvga

/* More Prototypes (dependent on  "gnu_grx_Dev") */

static void     init_double_buffer      ( gnu_grx_Dev *dev );
static void     CreateXhair             ( gnu_grx_Dev *dev, GrMouseEvent *event );
static void     DestroyXhair            ( gnu_grx_Dev *dev );
static void     SaveTopOfScreen         ( gnu_grx_Dev *dev );
static void     RestoreTopOfScreen      ( gnu_grx_Dev *dev );

#endif


/*--------------------------------------------------------------------------*\
 * plD_init_gnu_grx_dev()
 *
\*--------------------------------------------------------------------------*/

static void
plD_init_gnu_grx_dev(PLStream *pls)
{
    gnu_grx_Dev *dev;

/* Allocate and initialize device-specific data */

    if (pls->dev != NULL)
	plwarn("plD_init_gnu_grx_dev: \"gnu_grx_Dev\" device pointer is already set");

    pls->dev = calloc(1, (size_t) sizeof(gnu_grx_Dev));
    if (pls->dev == NULL)
	plexit("plD_init_gnu_grx_dev: Out of memory.");

    bzero(pls->dev,sizeof(gnu_grx_Dev)); /* I'm lazy - quick way of setting everything to 0 */

    dev = (gnu_grx_Dev *) pls->dev;

    if ( (pls->dev_compression<=0)||(pls->dev_compression>99) )
       pls->dev_compression=90;

    if (pls->width<=0) pls->width=1;

    dev->colour=1;
    dev->totcol=16;
    dev->vgax = 1023;
    dev->vgay = 767;

}

#ifdef PLD_gnusvga

/*----------------------------------------------------------------------*\
 * init_double_buffer()
 *
 * Initialize the second buffer for double buffering
\*----------------------------------------------------------------------*/

static void init_double_buffer ( gnu_grx_Dev *dev )
{
  if ( (dev->double_buffer==NULL)&&(dev->visual_screen==NULL) )
     {
      dev->visual_screen=GrSaveContext(NULL);
      dev->double_buffer=GrCreateContext(GrScreenX(), GrScreenY(),NULL,NULL);
      GrSetContext(dev->double_buffer);
      GrClearContext(0);
     }
  else
     {
      plexit("An undetermined error happened allocating the resources for double buffering.");
     }

}

/*----------------------------------------------------------------------*\
 * plD_init_vga()
 *
 * Initialize device.
\*----------------------------------------------------------------------*/

void
plD_init_vga(PLStream *pls)
{
    gnu_grx_Dev *dev;
#ifdef HAVE_FREETYPE
    FT_Data *FT;
    int freetype=0;
    int smooth_text=0;
    char *a;
    DrvOpt vga_options[] = {{"text", DRV_INT, &freetype, "Turn FreeType for text on (1) or off (0)"},
                              {"smooth", DRV_INT, &smooth_text, "Turn text smoothing on (1) or off (0)"},
			      {NULL, DRV_INT, NULL, NULL}};

/*
 *  Next, we parse the driver options to set some driver specific stuff.
 *  Before passing it though, we check for any environmental variables which
 *  might be set for the default behaviour of these "features" of the
 *  drivers. Naturally, the command line equivalent overrides it, hence why
 *  we check the enviro vars first.
 */

    a=getenv("PLPLOT_GNUSVGA_TEXT");
    if (a!=NULL)
       {
        freetype=atol(a);
       }

    a=getenv("PLPLOT_GNUSVGA_SMOOTH");
    if (a!=NULL)
       {
        smooth_text=atol(a);
       }

    plParseDrvOpts(vga_options);

#endif


    pls->termin = 1;            /* is an interactive terminal */
    pls->icol0 = 1;
    pls->bytecnt = 0;
    pls->page = 0;
    pls->graphx = TEXT_MODE;
    pls->dev_fill0 = 1;
    pls->dev_xor = 1;           /* device support xor mode */

    if (!pls->colorset)
	pls->color = 1;

#ifdef HAVE_FREETYPE
    if (freetype)
       {
        pls->dev_text = 1; /* want to draw text */
        init_freetype_lv1(pls);     /* first level initialisation of freertype. Must be done before plD_init_gnu_grx_dev(pls) */
        FT=(FT_Data *)pls->FT;
        FT->smooth_text=smooth_text;
       }
#endif

    if (pls->dev==NULL)
       plD_init_gnu_grx_dev(pls);

    dev=(gnu_grx_Dev *)pls->dev;


/* Set up device parameters */

    svga_graph(pls);            /* Can't get current device info unless in
				   graphics mode. */

    dev->vgax = GrSizeX() - 1;  /* should I use -1 or not??? */
    dev->vgay = GrSizeY() - 1;

#ifdef use_experimental_hidden_line_hack

    if (dev->vgax>dev->vgay)    /* Work out the scaling factor for the  */
       {                        /* "virtual" (oversized) page           */
       dev->scale=PIXELS_X/dev->vgax;
       }
    else
       {
       dev->scale=PIXELS_Y/dev->vgay;
       }
#else

   dev->scale=1;

#endif

    if (pls->xdpi==0)
       {
	plP_setpxl(2.5*dev->scale, 2.5*dev->scale);   /* My best guess.  Seems to work okay. */
       }
    else
       {
	pls->ydpi=pls->xdpi;        /* Set X and Y dpi's to the same value */
	plP_setpxl(dev->scale*pls->xdpi/25.4,dev->scale*pls->ydpi/25.4); /* Convert DPI to pixels/mm */
       }

    plP_setphy(0, dev->scale*dev->vgax, 0, dev->scale*dev->vgay);

    dev->gnusvgaline.lno_width=pls->width;
    dev->gnusvgaline.lno_pattlen=0;

/* Check and set up for Double buffering */

    if (pls->db!=0)
       init_double_buffer(dev);

#ifdef HAVE_FREETYPE

    if (pls->dev_text)
       {
        init_freetype_lv2(pls);      /* second level initialisation of freetype. Must be done AFTER plD_init_gnu_grx_dev(pls) */
       }

#endif

}

#endif

/*----------------------------------------------------------------------*\
 * plD_line_vga()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*----------------------------------------------------------------------*/

void
plD_line_vga(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
    int x1 = x1a/dev->scale, y1 = y1a/dev->scale, x2 = x2a/dev->scale, y2 = y2a/dev->scale;

    y1 = dev->vgay - y1;
    y2 = dev->vgay - y2;

   dev->gnusvgaline.lno_color=dev->colour|dev->draw_mode; /* dev->draw_mode for "XOR" */
   GrCustomLine(x1,y1,x2,y2,&dev->gnusvgaline);

    page_state = DIRTY;
}

/*----------------------------------------------------------------------*\
 * plD_polyline_vga()
 *
 * Draw a polyline in the current color.
\*----------------------------------------------------------------------*/

void
plD_polyline_vga(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i = 0; i < npts - 1; i++)
	plD_line_vga(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}


/*----------------------------------------------------------------------*\
 * fill_polygon()
 *
 * Fill polygon described in points pls->dev_x[] and pls->dev_y[].
\*----------------------------------------------------------------------*/

static void
fill_polygon(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    int i;
    int points[PL_MAXPOLY][2];

    if (pls->dev_npts < 1)
	return;

/* Select the fill pattern */

/* Specify boundary */

    for (i = 0; i < pls->dev_npts; i++) {
	points[i][0] = pls->dev_x[i]/dev->scale;
	points[i][1] = dev->vgay - (pls->dev_y[i]/dev->scale);
    }
    GrFilledPolygon(pls->dev_npts, points, dev->colour);

}


/*----------------------------------------------------------------------*\
 * setcmap()
 *
 * Sets up color palette.
\*----------------------------------------------------------------------*/

static void
setcmap(PLStream *pls)
{
    int i, ncol1, ncol0, last_col;
    PLColor cmap1col;
    PLFLT tmp_colour_pos;
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

#ifdef HAVE_FREETYPE
/*
 *  Set up the "extended" cmap0 palette with the entries for anti-aliasing
 *  the text.
 */

    FT_Data *FT=(FT_Data *)pls->FT;

if ((pls->dev_text==1)&&(FT->smooth_text==1))
   {
    FT->ncol0_org=pls->ncol0;                                   /* save a copy of the original size of ncol0 */
    FT->ncol0_xtra=GrNumColors()-(pls->ncol1+pls->ncol0);       /* work out how many free slots we have */
    FT->ncol0_width=FT->ncol0_xtra/(pls->ncol0-1);              /* find out how many different shades of anti-aliasing we can do */
    if (FT->ncol0_width>64) FT->ncol0_width=64;                 /* set a maximum number of shades */
    plscmap0n(FT->ncol0_org+(FT->ncol0_width*pls->ncol0));      /* redefine the size of cmap0 */
    pl_set_extended_cmap0(pls, FT->ncol0_width, FT->ncol0_org); /* call the function to add the extra cmap0 entries and calculate stuff */
   }

#endif


    if (isTrueColour())
       {
        GrSetRGBcolorMode();
#ifdef PLD_gnusvga
    if (pls->termin==1)
       GrMouseSetColors(GrAllocColor(255,255, 255),GrAllocColor(0,0,0));
#endif
       dev->fg=GrAllocColor(255,255, 255);

       }
    else
       {

        GrResetColors();

        if (GrNumColors()<pls->ncol0)
           ncol0=GrNumColors();
        else
           ncol0=pls->ncol0;

/* Initialize cmap 0 colors */

    for (i = 0; i < ncol0; i++)
	{
	GrSetColor(i, pls->cmap0[i].r, pls->cmap0[i].g, pls->cmap0[i].b);
	}

#ifdef PLD_gnusvga
    if (pls->termin==1)
       GrMouseSetColors(15,0);
#endif
   dev->fg=15;

/* Initialize any remaining slots for cmap1 */

    ncol1 = GrNumFreeColors();

    if (pls->ncol1<ncol1) ncol1=pls->ncol1;

    for (i = 0; i < ncol1; i++)
	{

	 if (ncol1<pls->ncol1)       /* Check the dynamic range of colours */
	    {

	     /*
	      * Ok, now if we have less colour slots available than are being
	      * defined by pls->ncol1, then we still want to use the full
	      * dynamic range of cmap1 as best we can, so what we do is work
	      * out an approximation to the index in the full dynamic range
	      * in cases when pls->ncol1 exceeds the number of free colours.
	      */

	     tmp_colour_pos= i>0 ? pls->ncol1*((PLFLT)i/ncol1) : 0;
	     plcol_interp(pls, &cmap1col, (int) tmp_colour_pos, pls->ncol1);

	    }
	else
	    {
	     plcol_interp(pls, &cmap1col, i, ncol1);
	    }

             GrSetColor(i + pls->ncol0, cmap1col.r, cmap1col.g, cmap1col.b);

    }

}

}

/*--------------------------------------------------------------------------*\
 * XorMod()
 *
 * Enter xor mode ( mod != 0) or leave it ( mode = 0)
\*--------------------------------------------------------------------------*/

static void
XorMod(PLStream *pls, PLINT *mod)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (*mod == 0)
      dev->draw_mode=GrWRITE;
    else
      dev->draw_mode=GrXOR;
}


/*----------------------------------------------------------------------*\
 * plD_state_vga()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*----------------------------------------------------------------------*/

void
plD_state_vga(PLStream *pls, PLINT op)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
PLFLT tmp_colour_pos;

    switch (op) {

    case PLSTATE_WIDTH:
	dev->gnusvgaline.lno_width=pls->width;
	break;

    case PLSTATE_COLOR0:
	if ( (pls->icol0 == PL_RGB_COLOR)||     /*  Should never happen since PL_RGB_COLOR is depreciated, but here for backwards compatibility */
             (isTrueColour()) )  /*  We will do this if we are in "TrueColour" mode */
           {
	    if ( (dev->totcol < GrNumColors())||         /* See if there are slots left, if so we will allocate a new colour */
                 (isTrueColour()) )  /* In TrueColour mode we allocate each colour as we come to it */
	       {
                if (isTrueColour())
                    dev->colour=GrAllocColor(pls->curcolor.r,pls->curcolor.g, pls->curcolor.b);
                else
                    {
                     GrSetColor(++dev->totcol, pls->curcolor.r, pls->curcolor.g, pls->curcolor.b);
                     dev->colour = dev->totcol;
                    }
	       }

           }
         else  /* just a normal colour allocate, so don't worry about the above stuff, just grab the index */
           {
            dev->colour = pls->icol0;
           }
	break;

    case PLSTATE_COLOR1:
       if (!isTrueColour())
	{ int ncol1=GrNumColors() - pls->ncol0;
         if (ncol1<pls->ncol1)
           {
           tmp_colour_pos=ncol1*((PLFLT)pls->icol1/(pls->ncol1>0 ? pls->ncol1 : 1));
           dev->colour = pls->ncol0 + (int)tmp_colour_pos;
           }
        else
           {
           dev->colour = pls->ncol0 + pls->icol1;
           }
	}
        else    /* it is a truecolour image */
           {
             dev->colour = GrAllocColor(pls->curcolor.r, pls->curcolor.g, pls->curcolor.b);
           }
	break;

    case PLSTATE_CMAP0:
    case PLSTATE_CMAP1:
	if (pls->color)
	    setcmap(pls);
	break;
    }
}

#ifdef PLD_gnusvga

/*----------------------------------------------------------------------*\
 * plD_eop_vga()
 *
 * End of page.
\*----------------------------------------------------------------------*/

void
plD_eop_vga(PLStream *pls)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (page_state == DIRTY)
       {
	if ( (dev->double_buffer!=NULL)&&(pls->db!=0)&&(dev->visual_screen!=NULL) )
	   {
	    GrSetContext(dev->visual_screen);
	    GrBitBlt(NULL,0,0,dev->double_buffer,0,0,
		      GrScreenX(), GrScreenY(),GrWRITE);
	   }
	else if ( (pls->db!=0) && ((dev->double_buffer==NULL)||(dev->visual_screen==NULL)) )
	   {
	    plexit("Wacko error with double buffering I think");
	   }

	if ( ! pls->nopause )
	    WaitForPage(pls);
      }

    page_state = CLEAN;
}

/*----------------------------------------------------------------------*\
 * plD_bop_vga()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void
plD_bop_vga(PLStream *pls)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if ( dev->Xhair_on == 1 ) DestroyXhair ( dev );

    pls->page++;
    plD_eop_vga(pls);

    if ( (pls->db==1) && (dev->double_buffer!=NULL) )
       {
	GrSetContext(dev->double_buffer);
	GrClearContext(0);
       }
    else
	GrClearScreen(0);

if (((pls->cmap0[0].r!=0)||(pls->cmap0[0].g!=0)||
     (pls->cmap0[0].b!=0) )&&(isTrueColour()))
   {
     GrFilledBox(0,0, GrScreenX(), GrScreenY(),
                 GrAllocColor(pls->cmap0[0].r,pls->cmap0[0].g,
                              pls->cmap0[0].b));
   }

}

/*----------------------------------------------------------------------*\
 * plD_tidy_vga()
 *
 * Close graphics file or otherwise clean up.
 * Checks to see if memory was allocated to a "saved screen" and
 * destroys if necessary.
 * Since I use the same "svga_text()" function to rest the screen as I
 * do for swapping between text and graphics, I use a "dev->don't_copy_screen"
 * field to make sure the "svga_text()" doesn't copy the screen.
\*----------------------------------------------------------------------*/

void
plD_tidy_vga(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if ( dev->Xhair_on == 1 ) DestroyXhair ( dev );

    GrMouseUnInit();
    dev->dont_copy_screen=1;
    svga_text(pls);

    if (dev->switch_screen!=NULL)
       {
	GrDestroyContext(dev->switch_screen);
	dev->switch_screen=NULL;
       }

    if (dev->visual_screen!=NULL)
       {
	GrDestroyContext(dev->visual_screen);
	dev->visual_screen=NULL;
       }

    if (dev->double_buffer!=NULL)
       {
	GrDestroyContext(dev->double_buffer);
	dev->double_buffer=NULL;
       }

    if (pls->dev!=NULL)
       {
	free(pls->dev);
	pls->dev=NULL;
       }

#ifdef HAVE_FREETYPE
  plD_FreeType_Destroy(pls);
#endif

}



/*--------------------------------------------------------------------------*\
 * gnusvga_GrMouseGetEvent ()
 *
 * Wrapper function for mouse events.
 * Function basically sits there and does the cross-hair thing if necessary,
 * otherwise does nothing but hand over to GRX20's mouse function.
\*--------------------------------------------------------------------------*/

void gnusvga_GrMouseGetEvent (int flags,PLStream *pls, GrMouseEvent *event)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
int ret=0;

do {
    if (dev->Xhair_on==1)
       {

	GrMouseGetEvent(flags|GR_M_MOTION,event);

	if (event->flags & GR_M_MOTION)
	   {

#ifdef BLIT_CROSSHAIR

	    GrBitBlt(NULL,0,dev->last_y,dev->Xhair_X,0,0,GrScreenX(), 0,GrXOR);
	    GrBitBlt(NULL,dev->last_x,0,dev->Xhair_Y,0,0,0, GrScreenY(),GrXOR);

	    GrBitBlt(NULL,0,event->y,dev->Xhair_X,0,0,GrScreenX(), 0,GrXOR);
	    GrBitBlt(NULL,event->x,0,dev->Xhair_Y,0,0,0, GrScreenY(),GrXOR);

#else

	    GrHLine(0,GrScreenX(),dev->last_y,dev->fg|GrXOR);
	    GrVLine(dev->last_x,0,GrScreenY(),dev->fg|GrXOR);

	    GrHLine(0,GrScreenX(),event->y,dev->fg|GrXOR);
	    GrVLine(event->x,0,GrScreenY(),dev->fg|GrXOR);

#endif
	    dev->last_x=event->x;
	    dev->last_y=event->y;
	   }
	if (flags & event->flags)
	   {
	    ret=1;
	   }
	}
    else
       {
       GrMouseGetEvent(flags,event);
       ret=1;
       }
 } while (ret==0);
}


/*--------------------------------------------------------------------------*\
 * gnusvga_GetCursorCmd()
 *
 * Waits for a graphics input event and returns coordinates.
\*--------------------------------------------------------------------------*/

static void
gnusvga_GetCursorCmd(PLStream *pls, PLGraphicsIn *ptr)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

    plGinInit(gin);
    dev->locate_mode=1;
    while ( (gin->pX < 0) && (dev->locate_mode) ) {
	gnusvga_GrMouseGetEvent(GR_M_BUTTON_DOWN+GR_M_KEYPRESS,pls,&dev->mevent);
	TranslateEvent(pls, &dev->mevent, gin);
    }
    *ptr = *gin;
}

/*--------------------------------------------------------------------------*\
 * ConfigBufferingCmd()
 *
 * Based on X-windows driver
\*--------------------------------------------------------------------------*/

static void ConfigBufferingCmd( PLStream *pls, PLBufferingCB *ptr )
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

switch (ptr->cmd) {

    case PLESC_DOUBLEBUFFERING_ENABLE:
	    if (pls->db==0)
	       {
		pls->db = 1;
		init_double_buffer(dev);
	       }
	break;

    case PLESC_DOUBLEBUFFERING_DISABLE:
	/*
	 *   First of all make sure that any active double buffer is
	 *   copied across to the visual screen, then clear any memory.
	 */
	if ( (pls->db==1)  &&
	     (dev->double_buffer!=NULL) &&
	     (dev->visual_screen!=NULL)    )
	   {
	    GrSetContext(dev->visual_screen);
	    GrBitBlt(NULL,0,0,dev->double_buffer,0,0,
		      GrScreenX(), GrScreenY(),GrWRITE);
	    GrDestroyContext(dev->visual_screen);
	    dev->visual_screen=NULL;
	    GrDestroyContext(dev->double_buffer);
	    dev->double_buffer=NULL;
	    }
	pls->db = 0;
	break;

    case PLESC_DOUBLEBUFFERING_QUERY:
	ptr->result = pls->db;
	break;

    default:
	printf( "Unrecognized buffering request ignored.\n" );
	break;
    }
}


/*----------------------------------------------------------------------*\
 * plD_esc_vga()
 *
 * Escape function.
 *
 * Functions:
 *
 *      PLESC_FILL              Fill polygon
 *      PLESC_GETC              Get coordinates upon mouse click
 *      PLESC_DOUBLEBUFFERING   Activate double buffering
 *      PLESC_XORMOD            set/reset xor mode
\*----------------------------------------------------------------------*/

void
plD_esc_vga(PLStream *pls, PLINT op, void *ptr)
{


    switch (op) {
      case PLESC_TEXT:  /* Set to text mode */
	svga_text(pls);
	break;

      case PLESC_GRAPH: /* set to graphics mode */
	svga_graph(pls);
	break;

    case PLESC_GETC:
	gnusvga_GetCursorCmd(pls, (PLGraphicsIn *) ptr);
	break;

      case PLESC_FILL:  /* fill */
	fill_polygon(pls);
	break;

    case PLESC_DOUBLEBUFFERING:
	ConfigBufferingCmd(pls, (PLBufferingCB *) ptr );
	break;

    case PLESC_XORMOD:
	XorMod(pls, (PLINT *) ptr);
	break;

#ifdef HAVE_FREETYPE
    case PLESC_HAS_TEXT:
      plD_render_freetype_text(pls, (EscText *)ptr);
      break;
#endif

#ifdef GRX_DO_JPEG
    case PLESC_SET_COMPRESSION:
	 if ( ((int) ptr>0)&&((int) ptr<100) )
	    {
	     pls->dev_compression=(int) ptr;
	    }
	break;
#endif

    }
}


/*----------------------------------------------------------------------*\
 * svga_text()
 *
 * Switch to text mode.
 *
 * This code first saves a copy of the video memory, so if it is
 * necessary to do a redraw, then everything is there to be pulled up
 * quickly.
\*----------------------------------------------------------------------*/

static void
svga_text(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (pls->graphx == GRAPHICS_MODE)
       {
	if ( dev->Xhair_on == 1 ) DestroyXhair ( dev );

	if ( (dev->switch_screen==NULL)&&(dev->dont_copy_screen==0) )
	   {
	    dev->switch_screen=GrCreateContext(GrScreenX(), GrScreenY(),NULL,NULL);
	    GrBitBlt(dev->switch_screen,0,0,NULL,0,0,GrScreenX(), GrScreenY(),GrWRITE);
	   }

	GrSetMode(GR_default_text);
	pls->graphx = TEXT_MODE;
	page_state = CLEAN;
      }
}

/*----------------------------------------------------------------------*\
 * svga_graph()
 *
 * Switch to graphics mode.
 * Will restore a copy of the previous graphics screen if one has been
 * saved.
\*----------------------------------------------------------------------*/

static void
svga_graph(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (pls->graphx == TEXT_MODE) {
	    if ((pls->xlength!=0)&&(pls->ylength!=0))
	       {
	       GrSetMode(GR_width_height_graphics,pls->xlength,pls->ylength);
	       }
	    else
	       {
	       GrSetMode(GR_default_graphics);
	       }
	    dev->totcol = 16;
	    setcmap(pls);
	if (dev->switch_screen!=NULL) /* Ok, seems like we saved a copy of the screen */
	   {
	    GrBitBlt(NULL,0,0,dev->switch_screen,0,0,GrScreenX(), GrScreenY(),GrWRITE);
	    GrDestroyContext(dev->switch_screen);
	    dev->switch_screen=NULL;
	   }
	pls->graphx = GRAPHICS_MODE;
	page_state = CLEAN;
    }
}

/*----------------------------------------------------------------------*\
 * plpause()
 *
 * Wait for a keystroke or mouse click.
 * Prompts user.
 * Saves screen's top line so prompt is not destructive
\*----------------------------------------------------------------------*/

static void
plpause(PLStream *pls)
{
GrMouseEvent event;
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    if (pls->nopause)
	return;

    SaveTopOfScreen(dev);

    GrTextXY(0, 0, "Pause->", dev->fg, 0);       /* Prompt for pausing */
    gnusvga_GrMouseGetEvent(GR_M_LEFT_DOWN+GR_M_KEYPRESS,pls,&event);       /* Wait for a mouse event */

    RestoreTopOfScreen(dev);

}


/*----------------------------------------------------------------------*\
 * WaitForPage()
 *
 * This routine waits for the user to advance the plot, while handling
 * all other events.
\*----------------------------------------------------------------------*/

static void
WaitForPage(PLStream *pls)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    while ( ! exit_eventloop )
	  {
	   gnusvga_GrMouseGetEvent(GR_M_LEFT_DOWN+GR_M_KEYPRESS,pls,&dev->mevent);
	   EventHandler(pls, &dev->mevent);
	 }
    exit_eventloop = FALSE;
}

/*----------------------------------------------------------------------*\
 * do_location()
 *
 * Prints the World Co-ordinate in the top of the display and then wait
 * for a keypress or mouse button before continuing.
 * Function constantly updates the coordinates (damn it's a nice function)
\*----------------------------------------------------------------------*/

void do_location(PLStream *pls)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
    PLGraphicsIn *gin = &(dev->gin);
    char buf[80];
    GrMouseEvent event;

    bzero(buf,80);

/* Call user locate mode handler if provided */

    if (pls->LocateEH != NULL)
	(*pls->LocateEH) (gin, pls->LocateEH_data, &dev->locate_mode);

/* Use default procedure */

    else {

	  SaveTopOfScreen(dev);

	  for (;;) {

		if (plTranslateCursor(gin))
		     {
		       sprintf(buf,"Value at cursor is : X = %f, Y = %f   ", gin->wX, gin->wY);
		       GrTextXY(0, 0, buf, dev->fg, 0);
		     }
		else
		     {
		       GrTextXY(0, 0, "Cannot translate                                   ", dev->fg, 0);
		     }

		 gnusvga_GrMouseGetEvent(GR_M_MOTION+GR_M_LEFT_DOWN+GR_M_KEYPRESS,pls,&event);       /* Wait for a mouse event */

		 if ((event.flags & GR_M_MOTION)!=1)    /* leave on anything *but* a movement */
		     break;

		 gin->pX = event.x;
		 gin->pY = event.y;
		 gin->dX = (PLFLT) event.x / dev->vgax;
		 gin->dY = 1.0 - (PLFLT) event.y / dev->vgay;

		   } /* End loop */

	   RestoreTopOfScreen( dev );

	  }

}



/*----------------------------------------------------------------------*\
 * EventHandler()
 *
 * Event handler routine.
 * Reacts to keyboard or mouse input.
\*----------------------------------------------------------------------*/

static void
EventHandler(PLStream *pls, GrMouseEvent *event)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

    TranslateEvent(pls, event, &dev->gin);

#ifdef DEBUG
    {
      char buf[200];
      sprintf(buf, "flags 0x%x, buttons 0x%x, key 0x%x                ",
	      event->flags, event->buttons, event->key);
      GrTextXY(0, 0, buf, 15, 0);
      sprintf(buf, "Input char 0x%x, Keycode 0x%x, string: %s               ",
	      event->key, dev->gin.keysym, dev->gin.string);
      GrTextXY(0, 50, buf, 15, 0);
      sprintf(buf, "button %d, pX %d, pY %d, state 0x%x                     ",
	      dev->gin.button, dev->gin.pX, dev->gin.pY, dev->gin.state);
      GrTextXY(0, 100, buf, 15, 0);
    }
#endif

/* Call user event handler */
/* Since this is called first, the user can disable all plplot internal
   event handling by setting key.code to 0 and key.string to '\0' */

    if (dev->gin.button) {
	if (pls->ButtonEH != NULL)
	    (*pls->ButtonEH) (&dev->gin, pls->ButtonEH_data, &exit_eventloop);
    }
    else {
	if (pls->KeyEH != NULL)
	    (*pls->KeyEH) (&dev->gin, pls->KeyEH_data, &exit_eventloop);
    }

/* Handle internal events */


/* Advance to next page (i.e. terminate event loop) on a <eol> */

    if (dev->gin.button == 1)
	exit_eventloop = TRUE;

switch(dev->gin.keysym)
      {

    case PLK_Linefeed:
    case PLK_Return:
	exit_eventloop = TRUE;
	break;

    case 'l':
	dev->gin.pX = event->x;
	dev->gin.pY = event->y;
	dev->gin.dX = (PLFLT) event->x / dev->vgax;
	dev->gin.dY = 1.0 - (PLFLT) event->y / dev->vgay;
	do_location(pls);
	break;

    case 'x':
	plGinInit(&dev->gin);
	if (dev->Xhair_on==0)
	   {
	    CreateXhair( dev, event );
	   }
	else
	   {
	    DestroyXhair( dev );
	   }
	break;

#if GRX_VERSION_API >= 0x0229
#ifdef GRX_DO_TIFF
       case PLK_F10:  /* F10 */
       SaveContextToTiff(NULL, newname("tif"), 0,"Created by GNUSVGA");
       break;
#endif

#ifdef GRX_DO_JPEG
       case PLK_F11:  /* F11 */
#if GRX_VERSION_API >= 0x0244
       GrSaveContextToJpeg(NULL, newname("jpg"), pls->dev_compression);
#else
       SaveContextToJpeg(NULL, newname("jpg"), pls->dev_compression);
#endif

       break;
#endif

#ifdef GRX_DO_BMP
       case PLK_F12:   /* F12 */
       GrSaveBmpImage(newname("bmp"), NULL, 0, 0, GrScreenX(), GrScreenY());
       break;
#endif
#endif
      }


/* Terminate on a 'Q' (not 'q', since it's too easy to hit by mistake) */

    if (dev->gin.string[0] == 'Q') {
	pls->nopause = TRUE;
	plexit("");
    }
}



/*--------------------------------------------------------------------------*\
 * TranslateEvent()
 *
 * Fills in the PLGraphicsIn from a MouseEvent.  All keys are not yet
 * translated correctly.
\*--------------------------------------------------------------------------*/

static void
TranslateEvent(PLStream *pls, GrMouseEvent *event, PLGraphicsIn *gin)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
    gin->string[0] = '\0';

/* Translate DJGPP GrMouseEvent into PlPLot event */

    if (event->flags & GR_M_KEYPRESS)
       {
	gin->keysym = event->key;
	if ((isprint(event->key))&&(event->key<255))
	   {
	    gin->string[0] = event->key;
	    gin->string[1] = '\0';
	   }
	else
	   {
	    switch (event->key) {

	    case K_F1: case K_F2: case K_F3:
	    case K_F4: case K_F5: case K_F6:
	    case K_F7: case K_F8: case K_F9:
	    case K_F10:
	      gin->keysym = event->key  + (PLK_F1- K_F1);
	      break;

	    case K_F11: /* F11 */
		 gin->keysym=PLK_F11;
		 break;

	    case K_F12: /* F12 */
		 gin->keysym=PLK_F12;
		 break;

	    case K_Home:
	    case K_EHome:
	      gin->keysym=PLK_Home;
	      break;

	    case K_Up: case K_EUp:
	      gin->keysym = PLK_Up;
	      break;

	    case K_Right: case K_ERight:
	      gin->keysym=PLK_Right;
	      break;

	    case K_Down: case K_EDown:
	      gin->keysym = PLK_Down;
	      break;

	    case K_Left: case K_ELeft:
	      gin->keysym = PLK_Left;
	      break;

	    case K_PageDown: case K_EPageDown:
	      gin->keysym = PLK_Prior;
	      break;

	    case K_PageUp: case K_EPageUp:
	      gin->keysym = PLK_Next;
	      break;

	    case K_Insert: case K_EInsert:
	      gin->keysym = PLK_Insert;
	      break;
	    default: gin->keysym = event->key;
	      break;

	    }
	}
	gin->button = 0;
	gin->state = event->kbstat;
    }

    else if (event->flags & GR_M_BUTTON_DOWN)
       {
	 switch (event->flags)
		{
		case GR_M_LEFT_DOWN:
		     gin->button = 1;
		     break;
		case GR_M_MIDDLE_DOWN:
		     gin->button = 2;
		     break;
		case GR_M_RIGHT_DOWN:
		     gin->button = 3;
		     break;
	      }
      gin->keysym = 0x20;
      gin->pX = event->x;
      gin->pY = event->y;
      gin->dX = (PLFLT) event->x / dev->vgax;
      gin->dY = 1.0 - (PLFLT) event->y / dev->vgay;
      gin->state = event->kbstat;
    }

}

/*----------------------------------------------------------------------*\
 * CreateXhair()
 *
 * Creates the cross hairs.
 * Cross hairs can be implimented as "blits" or lines.
\*----------------------------------------------------------------------*/

static void CreateXhair ( gnu_grx_Dev *dev, GrMouseEvent *event )
{
#ifdef BLIT_CROSSHAIR

GrContext *swap_context=NULL;

 /*
  *  create the contexts holding the two cross hairs
  */

 dev->Xhair_X=GrCreateContext(GrScreenX(),1,NULL,NULL);
 dev->Xhair_Y=GrCreateContext(1, GrScreenY(),NULL,NULL);

 swap_context=GrSaveContext(NULL);    /* save a temp copy of the current context */
 GrSetContext(dev->Xhair_X);          /* Set the context to the "X" line */
 GrClearContext(dev->fg);             /* Clear it out with forground colour */
 GrSetContext(dev->Xhair_Y);
 GrClearContext(dev->fg);
 GrSetContext(swap_context);          /* Set the context back */

#endif

 if (dev->toggle_xhair==0)
    {
#ifdef BLIT_CROSSHAIR

     GrBitBlt(NULL,0,event->y,dev->Xhair_X,0,0,GrScreenX(), 0,GrXOR);
     GrBitBlt(NULL,event->x,0,dev->Xhair_Y,0,0,0, GrScreenY(),GrXOR);

#else

    GrHLine(0,GrScreenX(),event->y,dev->fg|GrXOR);
    GrVLine(event->x,0,GrScreenY(),dev->fg|GrXOR);

#endif

     dev->last_x=event->x;
     dev->last_y=event->y;
    }
 else
    {
#ifdef BLIT_CROSSHAIR

     GrBitBlt(NULL,0,dev->last_y,dev->Xhair_X,0,0,GrScreenX(), 0,GrXOR);
     GrBitBlt(NULL,dev->last_x,0,dev->Xhair_Y,0,0,0, GrScreenY(),GrXOR);

#else

    GrHLine(0,GrScreenX(),dev->last_y,dev->fg|GrXOR);
    GrVLine(dev->last_x,0,GrScreenY(),dev->fg|GrXOR);

#endif

    }



 dev->Xhair_on=1;

}

/*----------------------------------------------------------------------*\
 * DestroyXhair()
 *
 * Creates the cross hairs.
\*----------------------------------------------------------------------*/

static void DestroyXhair ( gnu_grx_Dev *dev )
{


#ifdef BLIT_CROSSHAIR

  GrBitBlt(NULL,0,dev->last_y,dev->Xhair_X,0,0,GrScreenX(), 0,GrXOR);
  GrBitBlt(NULL,dev->last_x,0,dev->Xhair_Y,0,0,0, GrScreenY(),GrXOR);

  if (dev->Xhair_X!=NULL)
    {
    GrDestroyContext(dev->Xhair_X);
    dev->Xhair_X=NULL;
    }
  if (dev->Xhair_Y!=NULL)
    {
    GrDestroyContext(dev->Xhair_Y);
    dev->Xhair_Y=NULL;
    }

#else

  GrHLine(0,GrScreenX(),dev->last_y,dev->fg|GrXOR);
  GrVLine(dev->last_x,0,GrScreenY(),dev->fg|GrXOR);

#endif

 if (dev->toggle_xhair==0)
    {
     dev->last_x=0;
     dev->last_y=0;
   }


 dev->Xhair_on=0;

}

/*----------------------------------------------------------------------*\
 * SaveTopOfScreen()
 *
 * Function saves the top "line" of the screen to a buffer (using
 * blitting) so writing can be done to it and later restored.
\*----------------------------------------------------------------------*/

static void SaveTopOfScreen ( gnu_grx_Dev *dev )

{
	  /* Turn off Cross Hairs if they were turned on (nasty if not)
	   * since it causes problems with the top of the line
	   */

	  if (dev->Xhair_on==1)
	     {
	     dev->toggle_xhair=1;
	     DestroyXhair(dev);
	     }

	  /*  Save the top bit of the screen so we can overwrite
	   *  without too many problems
	   */

	  dev->top_line=GrCreateContext(GrScreenX(), 16,NULL,NULL);
	  GrBitBlt(dev->top_line,0,0,NULL,0,0,GrScreenX(), 16,GrWRITE);

	  /*
	   * Turn back on cross hair now if necessary
	   */

	  if (dev->toggle_xhair==1)
	     {
	     CreateXhair(dev, NULL);
	     dev->toggle_xhair=0;
	     }

}

/*----------------------------------------------------------------------*\
 * RestoreTopOfScreen()
 *
 * Function restores the saved top "line" of the screen to a buffer
 * (using blitting) so damage caused by writing can be undone.
\*----------------------------------------------------------------------*/

static void RestoreTopOfScreen ( gnu_grx_Dev *dev )

{
	    if (dev->top_line!=NULL)    /* Ok, seems like we saved a copy of the top line */
	       {

		if (dev->Xhair_on==1)   /* Turn off cross hair while fixing top */
		   {
		    dev->toggle_xhair=1;
		    DestroyXhair(dev);
		   }

		GrBitBlt(NULL,0,0,dev->top_line,0,0,GrScreenX(), 16,GrWRITE); /* So we will restore anything the top line overwrote */
		GrDestroyContext(dev->top_line);        /* remove copy of original top line */
		dev->top_line=NULL;

		/*
		 * Turn back on cross hair now if necessary
		 */
	       if (dev->toggle_xhair==1)
		  {
		   CreateXhair(dev, NULL);
		   dev->toggle_xhair=0;
		  }

	       }
}

#endif


#if defined(GRX_DO_TIFF) || defined(GRX_DO_BMP) || defined(GRX_DO_JPEG) || defined(PLD_tiff) || defined(PLD_jpg) || defined(PLD_bmp)

const char gnu_alphabet[]="0123456789abcdefghijklmnopqrstuvwxyz";

/*
 *    newname ()
 *
 *    Function returns a new name
 *    Works from the current time, year etc..
 *    Joins them all together and makes a string which as long as the system
 *    clock increases, should remain unique each time. Can be called up to
 *    36 times a second, after that the program delays for 1/10 of a sec
 *    and tries again for a unique file name. Function written for
 *    DOS, but should be portable.
 *    Function is Y2K+ compatible, but won't work with dates before
 *    2000. Good until 2036, then it will start reallocating names.
 *    Only a problem if you plan to use the same computer, program,
 *    directory, and files for 36 years :)
 */

char *newname ( char *ext)
{

static char name[13];
static char last_name[13];
time_t x;
struct tm *t;
static int i;
static int old_time;

bzero(name,13);

x=time(NULL);
t = gmtime(&x);
while ((old_time==x)&&(i==35))
   {
    delay(100);
    x=time(NULL);
    t = gmtime(&x);
   }

name[0]=gnu_alphabet[(t->tm_year-100)%36];
name[1]=gnu_alphabet[t->tm_mon+10];
name[2]=gnu_alphabet[t->tm_mday+4];
name[3]=gnu_alphabet[t->tm_hour];
name[4]=(t->tm_min>30 ? gnu_alphabet[t->tm_min-29] : gnu_alphabet[t->tm_min+1]);
name[5]=(t->tm_min>30 ? (t->tm_sec>30 ? 'a' : 'b') : (t->tm_sec>30 ? 'c' : 'd'));
name[6]=(t->tm_sec>30 ? gnu_alphabet[t->tm_sec-29] : gnu_alphabet[t->tm_sec+1]);

if(strncmp(last_name,name,7)==0)
  {
   ++i;
  }
else
  {
   i=0;
  }
name[7]=gnu_alphabet[i];
name[8]='.';

if (ext!=NULL) strncpy(&name[9],ext,3);
strcpy(last_name,name);
old_time=x;
return(name);
}


/*----------------------------------------------------------------------*\
 * gnusvga_get_a_file_name()
 *
 * Checks to see if there is a file name already, if there isn't then
 * it asks the user for one, allocates memory for the name, and makes
 * one.
\*----------------------------------------------------------------------*/

void gnusvga_get_a_file_name ( PLStream *pls )
{
int i=0;
   if (pls->BaseName==NULL)  /* Check to see if a File name has been defined yet */
     {

      /*
       *  Allocate memory for the file name.
       *  Should I be doing this like this ?
       *  I *think* this will be cleaned up later on by plplot, so i will do it anyway
       */

      if ((pls->BaseName=malloc(80))==NULL)
	 plexit("Could not allocate some memory");
      if ((pls->FileName=malloc(80))==NULL)
	 plexit("Could not allocate some memory");

      do { /* Continue to ask for the file name until we get an answer we like */
	  fprintf(stderr,"Enter file name (include \"0\" or \"00\" if defining a family, or use \"auto\" to \ndo automatic naming): ");
	  fgets(pls->BaseName, 79, stdin);
	  if (pls->BaseName!=NULL)
	     {
	      while (pls->BaseName[i]!=0)   /* strip off the trailing CR/LF or LF sequence, since we really don't want that stuffing things up */
		    {
		     if ((pls->BaseName[i]==10)||(pls->BaseName[i]==13))
			pls->BaseName[i]=0;
		     ++i;
		     if (i>79) break;
		    }

	      strncpy(pls->FileName,pls->BaseName,79); /* Copy BaseName to FileName */
	      if (strchr(pls->BaseName,'0')!=NULL) /* Test to see if we might have Family support */
		 pls->family=1;
	     }
	  } while (pls->BaseName==NULL);
     }
}


/*----------------------------------------------------------------------*\
 * gnusvga_expand_BaseName()
 *
 * Function takes the BaseName and sees if there is a family option set.
 * If there is, then it makes sure the file name format is 'legal' for
 * expanding the 'family' out, and then it expands the name. Otherwise,
 * it prompts the user for another file name.
 * Basically the algorithm replaces the last occurrence of "0" with
 * an alpha-numeric character. That makes for a total of 36
 * possibilities with a single "0", ie "image0.tif". When there are
 * two 0's, ie "image00.tif", then there are 1296 possible file names,
 * which should really be enough for anyone I hope.
\*----------------------------------------------------------------------*/

void gnusvga_expand_BaseName( PLStream *pls )
{
char *zero=NULL;
char yn[60];

if (pls->page>1)
   {
    if (pls->family==0) /* See if families aren't specified */
       {
	if (strchr(pls->BaseName,'0')!=NULL) /* if they gave a name supporting families, then check if they anted them */
	   {
	    do {
		fprintf(stderr,"Do you want to enable family support (y/n) : ");
		fgets(yn, sizeof(yn), stdin);
	       } while ((yn[0]=='y')&&(yn[0]=='Y')&&(yn[0]=='n')&&(yn[0]=='N'));
	    if ((yn[0]=='y')||(yn[0]=='Y'))
	       {
	       pls->family=1;
	       }
	   }
       }

    if (pls->family==0) /* See if families aren't specified */
       {
	free(pls->BaseName);
	pls->BaseName=NULL;
	free(pls->FileName);
	gnusvga_get_a_file_name ( pls );
       }
    else
       {
	strcpy(pls->FileName,pls->BaseName);
	zero=(char *)strrchr(pls->FileName,'0');
	if (zero==NULL)
	   plabort("Incorrect format for family name given (must have a \"0\" or \"00\" in the name)");
	else
	{
	if (zero[-1]=='0')
	   {
	    zero[-1]=gnu_alphabet[(pls->page-1)/36];
	   }
	else if ((pls->page-1)>35)
	   plabort("Number of files exceeded (next time try \"00\" in filename instead of \"0\")");

	zero[0]=gnu_alphabet[(pls->page-1)%36];
	}
     }

   }
}
#endif

#ifdef PLD_tiff

void plD_init_tiff(PLStream *pls);
void plD_tidy_tiff(PLStream *pls);
void plD_bop_tiff(PLStream *pls);
void plD_esc_tiff(PLStream *pls, PLINT op, void *ptr);
void plD_eop_tiff(PLStream *pls);

void plD_dispatch_init_tiff( PLDispatchTable *pdt )
{
    pdt->pl_MenuStr  = "TIFF File (TIFFLIB / GRX20)";
    pdt->pl_DevName  = "tiff";
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 14;
    pdt->pl_init     = (plD_init_fp)     plD_init_tiff;
    pdt->pl_line     = (plD_line_fp)     plD_line_vga;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_vga;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_tiff;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_tiff;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_tiff;
    pdt->pl_state    = (plD_state_fp)    plD_state_vga;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_tiff;
}

/*----------------------------------------------------------------------*\
 * plD_init_tiff()
 *
 * Initialize device.
\*----------------------------------------------------------------------*/

void plD_init_tiff(PLStream *pls)
{
    gnu_grx_Dev *dev=NULL;
#ifdef HAVE_FREETYPE
    FT_Data *FT;
    int freetype=0;
    int smooth_text=0;
    char *a;
    DrvOpt tiff_options[] = {{"text", DRV_INT, &freetype, "Turn FreeType for text on (1) or off (0)"},
                              {"smooth", DRV_INT, &smooth_text, "Turn text smoothing on (1) or off (0)"},
			      {NULL, DRV_INT, NULL, NULL}};

/*
 *  Next, we parse the driver options to set some driver specific stuff.
 *  Before passing it though, we check for any environmental variables which
 *  might be set for the default behaviour of these "features" of the
 *  drivers. Naturally, the command line equivalent overrides it, hence why
 *  we check the enviro vars first.
 */

    a=getenv("PLPLOT_TIFF_TEXT");
    if (a!=NULL)
       {
        freetype=atol(a);
       }

    a=getenv("PLPLOT_TIFF_SMOOTH");
    if (a!=NULL)
       {
        smooth_text=atol(a);
       }

    plParseDrvOpts(tiff_options);

#endif

    pls->termin = 0;            /* is an interactive terminal */
    pls->icol0 = 1;
    pls->bytecnt = 0;
    pls->page = 0;
    pls->graphx = TEXT_MODE;
    pls->dev_fill0 = 1;

    if (!pls->colorset)
	pls->color = 1;

#ifdef HAVE_FREETYPE
    if (freetype)
       {
        pls->dev_text = 1; /* want to draw text */
        init_freetype_lv1(pls);     /* first level initialisation of freertype. Must be done before plD_init_gnu_grx_dev(pls) */
        FT=(FT_Data *)pls->FT;
        FT->smooth_text=smooth_text;
       }
#endif

    if (pls->dev==NULL)
       plD_init_gnu_grx_dev(pls);

      dev=(gnu_grx_Dev *)pls->dev;
      dev->Old_Driver_Vector=GrDriverInfo->vdriver;
      GrSetDriver ("memory");

      if (pls->xlength <= 0 || pls->ylength <=0)
      {
/* use default width, height of 800x600 if not specifed by -geometry option
 * or plspage */
	 plspage(0., 0., 800, 600, 0, 0);
      }

	if ( (pls->ncol1 < 3) && (pls->ncol0 < 3) )
	   {
	    GrSetMode (GR_width_height_color_graphics, pls->xlength, pls->ylength, 2);
	   }
	else if ( (pls->ncol1 > 256) || (pls->ncol0 > 256) )
	   {
	    GrSetMode (GR_width_height_graphics, pls->xlength, pls->ylength);
	   }
	else
	   {
	    GrSetMode (GR_width_height_color_graphics, pls->xlength, pls->ylength, 256);
	   }

	setcmap(pls);
	dev->totcol = 16;               /* Reset RGB map so we don't run out of
				   indicies */
	pls->graphx = GRAPHICS_MODE;
	page_state = CLEAN;


    dev->vgax = GrSizeX() - 1;  /* should I use -1 or not??? */
    dev->vgay = GrSizeY() - 1;

#ifdef use_experimental_hidden_line_hack

    if (dev->vgax>dev->vgay)    /* Work out the scaling factor for the  */
       {                        /* "virtual" (oversized) page           */
       dev->scale=PIXELS_X/dev->vgax;
       }
    else
       {
       dev->scale=PIXELS_Y/dev->vgay;
       }
#else

   dev->scale=1;

#endif

    if (pls->xdpi==0)
       {
/* This corresponds to a typical monitor resolution of 4 pixels/mm. */
	plspage(4.*25.4, 4.*25.4, 0, 0, 0, 0);
       }
    else
       {
	pls->ydpi=pls->xdpi;        /* Set X and Y dpi's to the same value */
       }

    plP_setpxl(dev->scale*pls->xdpi/25.4,dev->scale*pls->ydpi/25.4); /* Convert DPI to pixels/mm */
    plP_setphy(0, dev->scale*dev->vgax, 0, dev->scale*dev->vgay);

dev->gnusvgaline.lno_width=pls->width;
dev->gnusvgaline.lno_pattlen=0;

#ifdef HAVE_FREETYPE

    if (freetype)
       {
        init_freetype_lv2(pls);      /* second level initialisation of freetype. Must be done AFTER plD_init_gnu_grx_dev(pls) */
       }

#endif

}



/*----------------------------------------------------------------------*\
 * plD_esc_tiff()
 *
 * Escape function.
\*----------------------------------------------------------------------*/

void plD_esc_tiff(PLStream *pls, PLINT op, void *ptr)
{
    switch (op) {

      case PLESC_FILL:  /* fill */
	fill_polygon(pls);
	break;

#ifdef HAVE_FREETYPE
    case PLESC_HAS_TEXT:
      plD_render_freetype_text(pls, (EscText *)ptr);
      break;
#endif

    }
}




/*----------------------------------------------------------------------*\
 * plD_eop_tiff()
 *
 * End of page.
\*----------------------------------------------------------------------*/

void plD_eop_tiff(PLStream *pls)
{

    if (page_state == DIRTY)
       {

	gnusvga_get_a_file_name(pls);

	if ( (strncasecmp(pls->FileName,"auto",4)==0) )
	   {
	    SaveContextToTiff(NULL, newname("tif"), 0,"Created by GNUSVGA");
	   }
       else
	   {
	    gnusvga_expand_BaseName(pls);
	    SaveContextToTiff(NULL,pls->FileName, 0,"Created by GNUSVGA");
	   }
      }

    page_state = CLEAN;
}

/*----------------------------------------------------------------------*\
 * plD_bop_tiff()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void plD_bop_tiff(PLStream *pls)
{
    pls->page++;
    plD_eop_tiff(pls);
    GrClearScreen(0);

    if (((pls->cmap0[0].r!=0)||(pls->cmap0[0].g!=0)||
        (pls->cmap0[0].b!=0) )&&(isTrueColour()))
      {
        GrFilledBox(0,0, GrScreenX(), GrScreenY(),
                    GrAllocColor(pls->cmap0[0].r,pls->cmap0[0].g,
                                 pls->cmap0[0].b));
      }

}

/*----------------------------------------------------------------------*\
 * plD_tidy_tiff()
 *
 * Close graphics file or otherwise clean up.
\*----------------------------------------------------------------------*/

void plD_tidy_tiff(PLStream *pls)
{
    gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

      if (dev->Old_Driver_Vector)
	 {
	  GrSetDriver (dev->Old_Driver_Vector->name);
	  dev->Old_Driver_Vector=NULL;
	 }

   free (pls->dev);
   pls->dev=NULL;

#ifdef HAVE_FREETYPE
  plD_FreeType_Destroy(pls);
#endif

}
#endif

#ifdef PLD_jpg

void plD_init_jpg(PLStream *pls);
void plD_tidy_jpg(PLStream *pls);
void plD_bop_jpg(PLStream *pls);
void plD_esc_jpg(PLStream *pls, PLINT op, void *ptr);
void plD_eop_jpg(PLStream *pls);

void plD_dispatch_init_jpg( PLDispatchTable *pdt )
{
    pdt->pl_MenuStr  = "JPEG File (Independent JPEG Group based on GRX20)";
    pdt->pl_DevName  = "jpg";
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 15;
    pdt->pl_init     = (plD_init_fp)     plD_init_jpg;
    pdt->pl_line     = (plD_line_fp)     plD_line_vga;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_vga;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_jpg;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_jpg;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_jpg;
    pdt->pl_state    = (plD_state_fp)    plD_state_vga;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_jpg;
}


/*----------------------------------------------------------------------*\
 * plD_init_jpg()
 *
 * Initialize device.
\*----------------------------------------------------------------------*/

void plD_init_jpg(PLStream *pls)
{
    gnu_grx_Dev *dev=NULL;
#ifdef HAVE_FREETYPE
    FT_Data *FT;
    int freetype=0;
    int smooth_text=0;
    char *a;
    DrvOpt jpg_options[] = {{"text", DRV_INT, &freetype, "Turn FreeType for text on (1) or off (0)"},
                              {"smooth", DRV_INT, &smooth_text, "Turn text smoothing on (1) or off (0)"},
			      {NULL, DRV_INT, NULL, NULL}};

/*
 *  Next, we parse the driver options to set some driver specific stuff.
 *  Before passing it though, we check for any environmental variables which
 *  might be set for the default behaviour of these "features" of the
 *  drivers. Naturally, the command line equivalent overrides it, hence why
 *  we check the enviro vars first.
 */

    a=getenv("PLPLOT_JPG_TEXT");
    if (a!=NULL)
       {
        freetype=atol(a);
       }

    a=getenv("PLPLOT_JPG_SMOOTH");
    if (a!=NULL)
       {
        smooth_text=atol(a);
       }

    plParseDrvOpts(jpg_options);

#endif


    pls->termin = 0;            /* is an interactive terminal */
    pls->icol0 = 1;
    pls->bytecnt = 0;
    pls->page = 0;
    pls->graphx = TEXT_MODE;

    pls->dev_fill0 = 1;

    if (!pls->colorset)
	pls->color = 1;

#ifdef HAVE_FREETYPE
    if (freetype)
       {
        pls->dev_text = 1; /* want to draw text */
        init_freetype_lv1(pls);     /* first level initialisation of freertype. Must be done before plD_init_gnu_grx_dev(pls) */
        FT=(FT_Data *)pls->FT;
        FT->smooth_text=smooth_text;
       }
#endif

    if (pls->dev==NULL)
       plD_init_gnu_grx_dev(pls);

      dev=(gnu_grx_Dev *)pls->dev;

/* Set up device parameters */
      dev->Old_Driver_Vector=GrDriverInfo->vdriver;
      GrSetDriver ("memory");

      if (pls->xlength <= 0 || pls->ylength <=0)
      {
/* use default width, height of 800x600 if not specifed by -geometry option
 * or plspage */
	 plspage(0., 0., 800, 600, 0, 0);
      }


	GrSetMode (GR_width_height_color_graphics, pls->xlength, pls->ylength, 256);

	setcmap(pls);
	dev->totcol = 16;               /* Reset RGB map so we don't run out of
				   indicies */
	pls->graphx = GRAPHICS_MODE;
	page_state = CLEAN;


    dev->vgax = GrSizeX() - 1;  /* should I use -1 or not??? */
    dev->vgay = GrSizeY() - 1;

#ifdef use_experimental_hidden_line_hack

    if (dev->vgax>dev->vgay)    /* Work out the scaling factor for the  */
       {                        /* "virtual" (oversized) page           */
       dev->scale=PIXELS_X/dev->vgax;
       }
    else
       {
       dev->scale=PIXELS_Y/dev->vgay;
       }
#else

   dev->scale=1;

#endif


    if (pls->xdpi==0)
       {
/* This corresponds to a typical monitor resolution of 4 pixels/mm. */
	plspage(4.*25.4, 4.*25.4, 0, 0, 0, 0);
       }
    else
       {
	pls->ydpi=pls->xdpi;        /* Set X and Y dpi's to the same value */
       }

    plP_setpxl(dev->scale*pls->xdpi/25.4,dev->scale*pls->ydpi/25.4); /* Convert DPI to pixels/mm */
    plP_setphy(0, dev->scale*dev->vgax, 0, dev->scale*dev->vgay);

    dev->gnusvgaline.lno_width=pls->width;
    dev->gnusvgaline.lno_pattlen=0;

#ifdef HAVE_FREETYPE

    if (freetype)
       {
        init_freetype_lv2(pls);      /* second level initialisation of freetype. Must be done AFTER plD_init_gnu_grx_dev(pls) */
       }

#endif


}



/*----------------------------------------------------------------------*\
 * plD_esc_jpg()
 *
 * Escape function.
\*----------------------------------------------------------------------*/

void plD_esc_jpg(PLStream *pls, PLINT op, void *ptr)
{
    switch (op) {

    case PLESC_FILL:
	fill_polygon(pls);
	break;

    case PLESC_SET_COMPRESSION:
	 if ( ((int) ptr>0)&&((int) ptr<100) )
	    {
	     pls->dev_compression=(int) ptr;
	    }
	break;

#ifdef HAVE_FREETYPE
    case PLESC_HAS_TEXT:
      plD_render_freetype_text(pls, (EscText *)ptr);
      break;
#endif

    }
}


/*----------------------------------------------------------------------*\
 * plD_eop_jpg()
 *
 * End of page.
\*----------------------------------------------------------------------*/

void plD_eop_jpg(PLStream *pls)
{

    if (page_state == DIRTY)
       {

	gnusvga_get_a_file_name(pls);

	if ( (strncasecmp(pls->FileName,"auto",4)==0) )
	   {
#if GRX_VERSION_API >= 0x0244
	    GrSaveContextToJpeg(NULL, newname("jpg"), pls->dev_compression);
#else
	    SaveContextToJpeg(NULL, newname("jpg"), pls->dev_compression);
#endif
	   }
       else
	   {
	    gnusvga_expand_BaseName(pls);
#if GRX_VERSION_API >= 0x0244
	    GrSaveContextToJpeg(NULL,pls->FileName, pls->dev_compression);
#else
	    SaveContextToJpeg(NULL,pls->FileName, pls->dev_compression);
#endif
	   }
      }

    page_state = CLEAN;
}

/*----------------------------------------------------------------------*\
 * plD_bop_jpg()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void plD_bop_jpg(PLStream *pls)
{
    pls->page++;
    plD_eop_jpg(pls);
    GrClearScreen(0);

    if (((pls->cmap0[0].r!=0)||(pls->cmap0[0].g!=0)||
        (pls->cmap0[0].b!=0) )&&(isTrueColour()))
      {
        GrFilledBox(0,0, GrScreenX(), GrScreenY(),
                    GrAllocColor(pls->cmap0[0].r,pls->cmap0[0].g,
                                 pls->cmap0[0].b));
      }

}

/*----------------------------------------------------------------------*\
 * plD_tidy_jpg()
 *
 * Close graphics file or otherwise clean up.
\*----------------------------------------------------------------------*/

void plD_tidy_jpg(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

      if (dev->Old_Driver_Vector)
	 {
	  GrSetDriver (dev->Old_Driver_Vector->name);
	  dev->Old_Driver_Vector=NULL;
	 }
free(pls->dev);
pls->dev=NULL;

#ifdef HAVE_FREETYPE
  plD_FreeType_Destroy(pls);
#endif

}
#endif

#ifdef PLD_bmp

void plD_init_bmp(PLStream *pls);
void plD_tidy_bmp(PLStream *pls);
void plD_bop_bmp(PLStream *pls);
void plD_esc_bmp(PLStream *pls, PLINT op, void *ptr);
void plD_eop_bmp(PLStream *pls);

void plD_dispatch_init_bmp( PLDispatchTable *pdt )
{
    pdt->pl_MenuStr  = "Windows Bitmap File (GRX20)";
    pdt->pl_DevName  = "bmp";
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 16;
    pdt->pl_init     = (plD_init_fp)     plD_init_bmp;
    pdt->pl_line     = (plD_line_fp)     plD_line_vga;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_vga;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_bmp;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_bmp;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_bmp;
    pdt->pl_state    = (plD_state_fp)    plD_state_vga;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_bmp;
}



/*----------------------------------------------------------------------*\
 * plD_init_bmp()
 *
 * Initialize device.
\*----------------------------------------------------------------------*/

void plD_init_bmp(PLStream *pls)
{
    gnu_grx_Dev *dev=NULL;
#ifdef HAVE_FREETYPE
    FT_Data *FT;
    int freetype=0;
    int smooth_text=0;
    char *a;
    DrvOpt bmp_options[] = {{"text", DRV_INT, &freetype, "Turn FreeType for text on (1) or off (0)"},
                              {"smooth", DRV_INT, &smooth_text, "Turn text smoothing on (1) or off (0)"},
			      {NULL, DRV_INT, NULL, NULL}};

/*
 *  Next, we parse the driver options to set some driver specific stuff.
 *  Before passing it though, we check for any environmental variables which
 *  might be set for the default behaviour of these "features" of the
 *  drivers. Naturally, the command line equivalent overrides it, hence why
 *  we check the enviro vars first.
 */

    a=getenv("PLPLOT_BMP_TEXT");
    if (a!=NULL)
       {
        freetype=atol(a);
       }

    a=getenv("PLPLOT_BMP_SMOOTH");
    if (a!=NULL)
       {
        smooth_text=atol(a);
       }

    plParseDrvOpts(bmp_options);

#endif

    pls->termin = 0;            /* is an interactive terminal */
    pls->icol0 = 1;
    pls->bytecnt = 0;
    pls->page = 0;
    pls->graphx = TEXT_MODE;

    pls->dev_fill0 = 1;

    if (!pls->colorset)
	pls->color = 1;

#ifdef HAVE_FREETYPE
    if (freetype)
       {
        pls->dev_text = 1; /* want to draw text */
        init_freetype_lv1(pls);     /* first level initialisation of freertype. Must be done before plD_init_gnu_grx_dev(pls) */
        FT=(FT_Data *)pls->FT;
        FT->smooth_text=smooth_text;
       }
#endif

    if (pls->dev==NULL)
       plD_init_gnu_grx_dev(pls);

      dev=(gnu_grx_Dev *)pls->dev;

/* Set up device parameters */
      dev->Old_Driver_Vector=GrDriverInfo->vdriver;
      GrSetDriver ("memory");

      if (pls->xlength <= 0 || pls->ylength <=0)
      {
/* use default width, height of 800x600 if not specifed by -geometry option
 * or plspage */
	 plspage(0., 0., 800, 600, 0, 0);
      }

	if ( (pls->ncol1 < 3) && (pls->ncol0 < 3) )
	   {
	    GrSetMode (GR_width_height_color_graphics, pls->xlength, pls->ylength, 2);
	   }
	else if ( (pls->ncol1 > 256) || (pls->ncol0 > 256) )
	   {
	    GrSetMode (GR_width_height_graphics, pls->xlength, pls->ylength);
	   }
	else
	   {
	    GrSetMode (GR_width_height_color_graphics, pls->xlength, pls->ylength, 256);
	   }

	setcmap(pls);
	dev->totcol = 16;               /* Reset RGB map so we don't run out of
				   indicies */
	pls->graphx = GRAPHICS_MODE;
	page_state = CLEAN;


    dev->vgax = GrSizeX() - 1;  /* should I use -1 or not??? */
    dev->vgay = GrSizeY() - 1;

#ifdef use_experimental_hidden_line_hack

    if (dev->vgax>dev->vgay)    /* Work out the scaling factor for the  */
       {                        /* "virtual" (oversized) page           */
       dev->scale=PIXELS_X/dev->vgax;
       }
    else
       {
       dev->scale=PIXELS_Y/dev->vgay;
       }
#else

   dev->scale=1;

#endif


    if (pls->xdpi==0)
       {
/* This corresponds to a typical monitor resolution of 4 pixels/mm. */
	plspage(4.*25.4, 4.*25.4, 0, 0, 0, 0);
       }
    else
       {
	pls->ydpi=pls->xdpi;        /* Set X and Y dpi's to the same value */
       }

    plP_setpxl(dev->scale*pls->xdpi/25.4,dev->scale*pls->ydpi/25.4); /* Convert DPI to pixels/mm */
    plP_setphy(0, dev->scale*dev->vgax, 0, dev->scale*dev->vgay);

    dev->gnusvgaline.lno_width=pls->width;
    dev->gnusvgaline.lno_pattlen=0;

#ifdef HAVE_FREETYPE

    if (freetype)
       {
        init_freetype_lv2(pls);      /* second level initialisation of freetype. Must be done AFTER plD_init_gnu_grx_dev(pls) */
       }

#endif


}



/*----------------------------------------------------------------------*\
 * plD_esc_bmp()
 *
 * Escape function.
\*----------------------------------------------------------------------*/

void plD_esc_bmp(PLStream *pls, PLINT op, void *ptr)
{
    switch (op) {

      case PLESC_FILL:  /* fill */
	fill_polygon(pls);
	break;
    }
}


/*----------------------------------------------------------------------*\
 * plD_eop_bmp()
 *
 * End of page.
\*----------------------------------------------------------------------*/

void plD_eop_bmp(PLStream *pls)
{

    if (page_state == DIRTY)
       {

	gnusvga_get_a_file_name(pls);

	if ( (strncasecmp(pls->FileName,"auto",4)==0) )
	   {
	    GrSaveBmpImage(newname("bmp"), NULL, 0, 0, GrScreenX(), GrScreenY());
	   }
       else
	   {
	    gnusvga_expand_BaseName(pls);
	    GrSaveBmpImage(pls->FileName, NULL, 0, 0, GrScreenX(), GrScreenY());
	   }
      }


    page_state = CLEAN;
}

/*----------------------------------------------------------------------*\
 * plD_bop_bmp()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void plD_bop_bmp(PLStream *pls)
{
    pls->page++;
    plD_eop_bmp(pls);
    GrClearScreen(0);

    if (((pls->cmap0[0].r!=0)||(pls->cmap0[0].g!=0)||
        (pls->cmap0[0].b!=0) )&&(isTrueColour()))
      {
        GrFilledBox(0,0, GrScreenX(), GrScreenY(),
                    GrAllocColor(pls->cmap0[0].r,pls->cmap0[0].g,
                                 pls->cmap0[0].b));
      }

}

/*----------------------------------------------------------------------*\
 * plD_tidy_bmp()
 *
 * Close graphics file or otherwise clean up.
\*----------------------------------------------------------------------*/

void plD_tidy_bmp(PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

      if (dev->Old_Driver_Vector)
	 {
	  GrSetDriver (dev->Old_Driver_Vector->name);
	  dev->Old_Driver_Vector=NULL;
	 }

#ifdef HAVE_FREETYPE
  plD_FreeType_Destroy(pls);
#endif

free(pls->dev);
pls->dev=NULL;
}
#endif


#ifdef HAVE_FREETYPE

/*----------------------------------------------------------------------*\
 *  void plD_pixel_vga (PLStream *pls, short x, short y)
 *
 *  callback function, of type "plD_pixel_fp", which specifies how a single
 *  pixel is set in the current colour.
\*----------------------------------------------------------------------*/

void plD_pixel_vga (PLStream *pls, short x, short y)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;

 GrPlot(x,y,dev->colour);
}

/*----------------------------------------------------------------------*\
 *  void init_freetype_lv1 (PLStream *pls)
 *
 *  "level 1" initialisation of the freetype library.
 *  "Level 1" initialisation calls plD_FreeType_init(pls) which allocates
 *  memory to the pls->FT structure, then sets up the pixel callback
 *  function.
\*----------------------------------------------------------------------*/

static void init_freetype_lv1 (PLStream *pls)
{
FT_Data *FT;

plD_FreeType_init(pls);

FT=(FT_Data *)pls->FT;
FT->pixel= (plD_pixel_fp)plD_pixel_vga;

}

/*----------------------------------------------------------------------*\
 *  void init_freetype_lv2 (PLStream *pls)
 *
 *  "Level 2" initialisation of the freetype library.
 *  "Level 2" fills in a few setting that aren't public until after the
 *  graphics sub-syetm has been initialised.
\*----------------------------------------------------------------------*/

static void init_freetype_lv2 (PLStream *pls)
{
gnu_grx_Dev *dev=(gnu_grx_Dev *)pls->dev;
FT_Data *FT=(FT_Data *)pls->FT;

FT->scale=dev->scale;
FT->ymax=dev->vgay;
FT->invert_y=1;

}


#endif

#else
int
pldummy_gnusvga()
{
    return 0;
}

#endif                          /* GNUSVGA */

