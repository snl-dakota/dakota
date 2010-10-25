/* $Id: plfreetype.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 * Copyright (C) 2002, 2004  Andrew Roach
 * Copyright (C) 2002  Maurice LeBrun
 * Copyright (C) 2002, 2004, 2005  Alan W. Irwin
 * Copyright (C) 2003, 2004  Joao Cardoso
 * Copyright (C) 2003, 2004, 2005  Rafael Laboissiere
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
 *
 *
 *                  Support routines for freetype font engine
 *
 *  This file contains a series of support routines for drivers interested
 *  in using freetype rendered fonts instead of plplot plotter fonts.
 *  Freetype supports a gerth of font formats including TrueType, OpenType,
 *  Adobe Type1, Type42 etc... the list seems almost endless. Any bitmap
 *  driver should be able to use any of these freetype fonts from plplot if
 *  these routines are properly initialised.
 *
 *  Freetype support is not intended to be a "feature" of the common API,
 *  but is  implemented as a driver-specific optional extra invoked via the
 *  -drvopt command line toggle. It is intended to be used in the context of
 *  "PLESC_HAS_TEXT" for any bitmap drivers without native font support.
 *  Implementing freetype in this manner minimise changes to the overall
 *  API. Because of this approach, there is not a "wealth" of font options
 *  available to the programmer. You can not do anything you can't do for a
 *  normal freetype plotter font like boldface. You can do most of the
 *  things that you can do with a plotter font however, like greek
 *  characters superscripting, and selecting one of the four "pre-defined"
 *  plplot font types. At present underlining and overlining are not
 *  supported.
 *
 *  To give the user some level of control over the fonts that are used,
 *  environmental variables can be set to over-ride the definitions used by
 *  the five default plplot fonts.
 *
 *  The exact syntax for evoking freetype fonts is dependant on each
 *  driver, but for the GD and GNUSVGA drivers I have followed the syntax of
 *  the PS driver and use the command-line switch of "-drvopt text" to
 *  activate the feature, and suggest other programmers do the same for
 *  commonality.
 *
 *  Both anti-aliased and monochrome font rendering is supported by these
 *  routines. How these are evoked depends on the programmer, but with the
 *  GD and GNUSVGA driver families I have used the command-line switch
 *  "-drvopt smooth" to activate the feature; but, considering you also need
 *  to turn freetype on, it would probably really be more like "-drvopt
 *  text,smooth".
 *
 */

#include <unistd.h>

#include "plDevs.h"
#include "plplotP.h"
#include "drivers.h"
#ifdef HAVE_FREETYPE
#include "plfreetype.h"
#include "plfci-truetype.h"

#define FT_Data _FT_Data_

/* Font lookup table that is constructed in plD_FreeType_init*/
FCI_to_FontName_Table FontLookup[N_TrueTypeLookup];
/*              TOP LEVEL DEFINES       */

/*  Freetype lets you set the text size absolutely. It also takes into
 *  account the DPI when doing so. So does plplot. Why, then, is it that the
 *  size of the text drawn by plplot is bigger than the text drawn by
 *  freetype when given IDENTICAL parameters ? Perhaps I am missing
 *  something somewhere, but to fix this up we use TEXT_SCALING_FACTOR to
 *  set a scaling factor to try and square things up a bit.
 */

#define TEXT_SCALING_FACTOR .7

/* default size of temporary text buffer */
/* If we wanted to be fancy we could add sizing, but this should be big enough */

#define NTEXT_ALLOC 1024

/* Default size of the text cache used with buffering */

#define FT_TEXT_CACHESZ 65536

/*--------------------------------------------------------------------------*\
 *  Some debugging macros
\*--------------------------------------------------------------------------*/

#define Verbose(...) do {if (pls->verbose){fprintf(stderr,__VA_ARGS__);}}while(0)
#define Debug(...) do {if (pls->debug){fprintf(stderr,__VA_ARGS__);}}while(0)


/*              FUNCTION PROTOTYPES    */

/*  Public prototypes, generally available to the API  */

void plD_FreeType_init(PLStream *pls);
void plD_render_freetype_text (PLStream *pls, EscText *args);
void plD_FreeType_Destroy(PLStream *pls);
void pl_set_extended_cmap0(PLStream *pls, int ncol0_width, int ncol0_org);
void pl_RemakeFreeType_text_from_buffer (PLStream *pls);
void plD_render_freetype_sym (PLStream *pls, EscText *args);

/*  Private prototypes for use in this file only */

static void FT_PlotChar(PLStream *pls,FT_Data *FT, FT_GlyphSlot  slot, int x, int y, short colour );
static void FT_SetFace( PLStream *pls, PLUNICODE fci );
static PLFLT CalculateIncrement( int bg, int fg, int levels);
static void pl_save_FreeType_text_to_buffer (PLStream *pls, EscText *args);
static FT_ULong hershey_to_unicode (char in);
static void FT_WriteStrW(PLStream *pls,const PLUNICODE  *text, short len, int x, int y);
static void FT_StrX_YW(PLStream *pls,const PLUNICODE *text, short len, int *xx, int *yy);

/*----------------------------------------------------------------------*\
 * FT_StrX_YW()
 *
 * Returns the dimensions of the text box. It does this by fully parsing
 * the supplied text through the rendering engine. It does everything
 * but draw the text. This seems, to me, the easiest and most accurate
 * way of determining the text's dimensions. If/when caching is added,
 * the CPU hit for this "double processing" will be minimal.
\*----------------------------------------------------------------------*/

void
FT_StrX_YW(PLStream *pls, const PLUNICODE *text, short len, int *xx, int *yy)
{
    FT_Data *FT=(FT_Data *)pls->FT;
    short i=0;
    FT_Vector  akerning;
    int x=0,y=0;
    char esc;

    plgesc(&esc);

/*
 * Things seems to work better with this line than without it;
 * I guess because there is no vertical kerning or advancement for most
 * non-transformed fonts, so we need to define *something* for the y height,
 * and this is the best thing I could think of.
 */

    y -= FT->face->size->metrics.height;

/* walk through the text character by character */
    for (i=0;i<len;i++) {
	if ((text[i]==esc)&&(text[i-1]!=esc)) {
	    if (text[i+1]==esc) continue;

	    switch(text[i+1]) {

	    case 'u': /* super script */
	    case 'd': /* subscript */
	    case 'U':
	    case 'D':
                i++;
                break;
	    }

        } else if (text[i] & PL_FCI_MARK) {
	   /* FCI in text stream; change font accordingly. */
	   FT_SetFace(pls , text[i]);
        } else {

	/* see if we have kerning for the particular character pair */
	    if ((i>0)&&FT_HAS_KERNING(FT->face)) {
		FT_Get_Kerning( FT->face,
				text[i-1],
				text[i],
				ft_kerning_default,
				&akerning );
		x+= (akerning.x >> 6);        /* add (or subtract) the kerning */
	    }

     /*
      * Next we load the char. This also draws the char, transforms it, and
      * converts it to a bitmap. At present this is a bit wasteful, but
      * if/when I add cache support, then this data won't go to waste.
      * Since there is no sense in going to the trouble of doing anti-aliasing
      * calculations since we aren't REALLY plotting anything, we will render
      * this as monochrome since it is probably marginally quicker. If/when
      * cache support is added, naturally this will have to change.
      */

	    FT_Load_Char( FT->face, text[i], FT_LOAD_MONOCHROME+FT_LOAD_RENDER);

     /*
      * Add in the "advancement" needed to position the cursor for the next
      * character. Unless the text is transformed, "y" will always be zero.
      * Y is negative because freetype does things upside down
      */

	    x += (FT->face->glyph->advance.x);
	    y -= (FT->face->glyph->advance.y);
	}
    }

/*
 * Convert from unit of 1/64 of a pixel to pixels, and do it real fast with
 * a bitwise shift (mind you, any decent compiler SHOULD optimise /64 this way
 * anyway...)
 */

/* (RL, on 2005-01-23) Removed the shift bellow to avoid truncation errors
 * later.
    *yy=y>> 6;
    *xx=x>> 6;
 */
    *yy = y;
    *xx = x;

}

/*----------------------------------------------------------------------*\
 * FT_WriteStrW()
 *
 * Writes a string of FT text at the current cursor location.
 * most of the code here is identical to "FT_StrX_Y" and I will probably
 * collapse the two into some more efficient code eventually.
\*----------------------------------------------------------------------*/

void
FT_WriteStrW(PLStream *pls, const PLUNICODE *text, short len, int x, int y)
{
    FT_Data *FT=(FT_Data *)pls->FT;
    short i=0,last_char=-1;
    FT_Vector  akerning, adjust;
    char esc;

    plgesc(&esc);


/*
 *  Adjust for the descender - make sure the font is nice and centred
 *  vertically. Freetype assumes we have a base-line, but plplot thinks of
 *  centre-lines, so that's why we have to do this. Since this is one of our
 *  own adjustments, rather than a freetype one, we have to run it through
 *  the transform matrix manually.
 *
 *  For some odd reason, this works best if we triple the
 *  descender's height and then adjust the height later on...
 *  Don't ask me why, 'cause I don't know. But it does seem to work.
 *
 *  I really wish I knew *why* it worked better though...
 *
 *   y-=FT->face->descender >> 6;
 */

#ifdef DODGIE_DECENDER_HACK
    adjust.y= (FT->face->descender >> 6)*3;
#else
    adjust.y= (FT->face->descender >> 6);
#endif

/* (RL) adjust.y is zeroed below,, making the code above (around
 * DODGIE_DECENDER_HACK) completely useless.  This is necessary for
 * getting the vertical alignment of text right, which is coped with
 * in function plD_render_freetype_text now.
 */

    adjust.x=0;
    adjust.y=0;
    FT_Vector_Transform( &adjust, &FT->matrix);
    x+=adjust.x;
    y-=adjust.y;

/* (RL, on 2005-01-25) The computation of cumulated glyph width within
 * the text is done now with full precision, using 26.6 Freetype
 * arithmetics.  We should then shift the x and y variables by 6 bits,
 * as below.  Inside the character for loop, all operations regarding
 * x and y will be done in 26.6 mode and  these variables will be
 * converted to integers when passed to FT_PlotChar.  Notrice that we
 * are using ROUND and float division instead of ">> 6" now.  This
 * minimizes truncation errors.
*/

    x <<= 6;
    y <<= 6;

/* walk through the text character by character */

    for (i=0; i<len; i++) {
	if ((text[i]==esc)&&(text[i-1]!=esc)) {
	    if (text[i+1]==esc) continue;

	    switch(text[i+1]) {

/*
 *  We run the OFFSET for the super-script and sub-script through the
 *  transformation matrix so we can calculate nice and easy the required
 *  offset no matter what's happened rotation wise. Everything else, like
 *  kerning and advancing from character to character is transformed
 *  automatically by freetype, but since the superscript/subscript is a
 *  feature of plplot, and not freetype, we have to make allowances.
 */

	    case 'u': /* super script */
	    case 'U': /* super script */
                adjust.y = FT->face->size->metrics.height / 2;
                adjust.x=0;
                FT_Vector_Transform( &adjust, &FT->matrix);
                x+=adjust.x;
                y-=adjust.y;
                i++;
                break;

	    case 'd': /* subscript */
	    case 'D': /* subscript */
                adjust.y = -FT->face->size->metrics.height / 2;
                adjust.x=0;
                FT_Vector_Transform( &adjust, &FT->matrix);
                x+=adjust.x;
                y-=adjust.y;
                i++;
                break;
	    }

        } else if (text[i] & PL_FCI_MARK) {
	   /* FCI in text stream; change font accordingly. */
	   FT_SetFace(pls , text[i]);
	   FT=(FT_Data *)pls->FT;
	   FT_Set_Transform( FT->face, &FT->matrix, &FT->pos );
        } else {
	/* see if we have kerning for the particular character pair */
	    if ((last_char!=-1)&&(i>0)&&FT_HAS_KERNING(FT->face)) {
		FT_Get_Kerning( FT->face,
				text[last_char],
				text[i],
				ft_kerning_default, &akerning );
		x += akerning.x;        /* add (or subtract) the kerning */
                y -= akerning.y;        /* Do I need this in case of rotation ? */

            }


       FT_Load_Char( FT->face, text[i], (FT->smooth_text==0) ? FT_LOAD_MONOCHROME+FT_LOAD_RENDER : FT_LOAD_RENDER|FT_LOAD_FORCE_AUTOHINT);
       FT_PlotChar(pls,FT, FT->face->glyph,
                   ROUND (x / 64.0), ROUND (y / 64.0), 2 ); /* render the text */

	    x += FT->face->glyph->advance.x;
            y -= FT->face->glyph->advance.y;

	    last_char=i;
	}
    } /* end for */
}

/*----------------------------------------------------------------------*\
 * FT_PlotChar()
 *
 * Plots an individual character. I know some of this stuff, like colour
 * could be parsed from plstream, but it was just quicker this way.
\*----------------------------------------------------------------------*/

void
FT_PlotChar(PLStream *pls, FT_Data *FT, FT_GlyphSlot slot,
	    int x, int y, short colour )
{
    unsigned char bittest;
    short i,k,j;
    int n=slot->bitmap.pitch;

    if ((slot->bitmap.pixel_mode==ft_pixel_mode_mono)||(pls->icol0==0)) {
	x+=slot->bitmap_left;
	y-=slot->bitmap_top;

	for(i=0;i<slot->bitmap.rows;i++) {
	    for (k=0;k<n;k++) {
		bittest=128;
		for (j=0;j<8;j++) {
		    if ((bittest&(unsigned char)slot->bitmap.buffer[(i*n)+k])==bittest)
			FT->pixel(pls, x+(k*8)+j, y+i);
		    bittest>>=1;
		}
            }
        }
    }

/* this is the anti-aliased stuff */
/* At present only one anti-aliased mode is supported, using a
   fixed background and palette. That might change eventually */

    else {
	x+=slot->bitmap_left;
	y-=slot->bitmap_top;

	for(i=0;i<slot->bitmap.rows;i++) {
	    for (k=0;k<slot->bitmap.width;k++) {
		FT->shade=(slot->bitmap.buffer[(i*slot->bitmap.width)+k]);
		if (FT->shade>0) {
		    FT->col_idx=FT->ncol0_width-((FT->ncol0_width*FT->shade)/255);
		    FT->last_icol0=pls->icol0;
		    plcol0(pls->icol0+(FT->col_idx*(FT->ncol0_org-1)));
		    FT->pixel(pls,x+k,y+i);
		    plcol0(FT->last_icol0);
                }
	    }
        }
    }
}

/*----------------------------------------------------------------------*\
 * plD_FreeType_init()
 *
 * Allocates memory to Freetype structure
 * Initialises the freetype library.
 * Initialises freetype structure
\*----------------------------------------------------------------------*/

void plD_FreeType_init(PLStream *pls)
{
    FT_Data *FT;
    char *a;
/* font paths and file names can be long so leave generous (1024) room */
    char font_dir[1024];
    /* N.B. must be in exactly same order as TrueTypeLookup */
    const char *env_font_names[N_TrueTypeLookup] = {
         "PLPLOT_FREETYPE_SANS_FONT",
         "PLPLOT_FREETYPE_SERIF_FONT",
         "PLPLOT_FREETYPE_MONO_FONT",
         "PLPLOT_FREETYPE_SCRIPT_FONT",
         "PLPLOT_FREETYPE_SYMBOL_FONT",
         "PLPLOT_FREETYPE_SANS_ITALIC_FONT",
         "PLPLOT_FREETYPE_SERIF_ITALIC_FONT",
         "PLPLOT_FREETYPE_MONO_ITALIC_FONT",
         "PLPLOT_FREETYPE_SCRIPT_ITALIC_FONT",
         "PLPLOT_FREETYPE_SYMBOL_ITALIC_FONT",
         "PLPLOT_FREETYPE_SANS_OBLIQUE_FONT",
         "PLPLOT_FREETYPE_SERIF_OBLIQUE_FONT",
         "PLPLOT_FREETYPE_MONO_OBLIQUE_FONT",
         "PLPLOT_FREETYPE_SCRIPT_OBLIQUE_FONT",
         "PLPLOT_FREETYPE_SYMBOL_OBLIQUE_FONT",
         "PLPLOT_FREETYPE_SANS_BOLD_FONT",
         "PLPLOT_FREETYPE_SERIF_BOLD_FONT",
         "PLPLOT_FREETYPE_MONO_BOLD_FONT",
         "PLPLOT_FREETYPE_SCRIPT_BOLD_FONT",
         "PLPLOT_FREETYPE_SYMBOL_BOLD_FONT",
         "PLPLOT_FREETYPE_SANS_BOLD_ITALIC_FONT",
         "PLPLOT_FREETYPE_SERIF_BOLD_ITALIC_FONT",
         "PLPLOT_FREETYPE_MONO_BOLD_ITALIC_FONT",
         "PLPLOT_FREETYPE_SCRIPT_BOLD_ITALIC_FONT",
         "PLPLOT_FREETYPE_SYMBOL_BOLD_ITALIC_FONT",
         "PLPLOT_FREETYPE_SANS_BOLD_OBLIQUE_FONT",
         "PLPLOT_FREETYPE_SERIF_BOLD_OBLIQUE_FONT",
         "PLPLOT_FREETYPE_MONO_BOLD_OBLIQUE_FONT",
         "PLPLOT_FREETYPE_SCRIPT_BOLD_OBLIQUE_FONT",
         "PLPLOT_FREETYPE_SYMBOL_BOLD_OBLIQUE_FONT"
    };
    short i;

#if defined(MSDOS) || defined(WIN32)
    static char *default_font_names[]={"arial.ttf","times.ttf","timesi.ttf","arial.ttf",
				       "symbol.ttf"};
#else
    const char *default_unix_font_dir=PL_FREETYPE_FONT_DIR;
#endif

    if (pls->FT) {
	plwarn("Freetype seems already to have been initialised!");
	return;
    }

    if ((pls->FT=calloc(1, (size_t)sizeof(FT_Data)))==NULL)
	plexit("Could not allocate memory for Freetype");

    FT=(FT_Data *)pls->FT;

    if ((FT->textbuf=calloc(NTEXT_ALLOC, 1))==NULL)
	plexit("Could not allocate memory for Freetype text buffer");

    if (pls->plbuf_write==1)
    {
      if((FT->text_cache=calloc(1, (size_t) FT_TEXT_CACHESZ))==NULL)
         plexit("Could not allocate memory for Freetype text cache");
      FT->mem_allocated=FT_TEXT_CACHESZ;
    }

    if ( FT_Init_FreeType( &FT->library ) )
	plexit("Could not initialise Freetype library");

    /* set to an impossible value for an FCI */
    FT->fci=PL_FCI_IMPOSSIBLE;

#if defined(MSDOS) || defined(WIN32)

/*
 * Work out if we have Win95+ or Win3.?... sort of.
 * Actually, this just tries to find the place where the fonts live by looking
 * for arial, which should be on all windows machines.
 * At present, it only looks in two places, on one drive. I might change this
 * soon.
 */

    if (access("c:/windows/fonts/arial.ttf", F_OK)==0)
	strcpy(font_dir,"c:/windows/fonts/");

    else if (access("c:/windows/system/arial.ttf", F_OK)==0)
	strcpy(font_dir,"c:/windows/system/");

    else
	plwarn("Could not find font path; I sure hope you have defined fonts manually !");

#else

/*
 *  For Unix systems, we will set the font path up a little differently in
 *  that the configured PL_FREETYPE_FONT_DIR has been set as the default path,
 *  but the user can override this by setting the environmental variable
 *  "PLPLOT_FREETYPE_FONT_DIR" to something else.
 *  NOTE WELL - the trailing slash must be added for now !
 */

    if ((a = getenv("PLPLOT_FREETYPE_FONT_DIR")) != NULL)
        strcpy(font_dir,a);
    else
        strcpy(font_dir,default_unix_font_dir);

#endif

/*
 * The driver looks for N_TrueTypeLookup  environmental variables
 * where the path and name of these fonts can be OPTIONALLY set,
 * overriding the configured default values.
 */

    for (i=0; i<N_TrueTypeLookup; i++) {
	if ((a = getenv(env_font_names[i])) != NULL) {

/*
 *  Work out if we have been given an absolute path to a font name, or just
 *  a font name sans-path. To do this we will look for a directory separator
 *  character, which means some system specific junk. DJGPP is all wise, and
 *  understands both Unix and DOS conventions. DOS only knows DOS, and
 *  I assume everything else knows Unix-speak. (Why Bill, didn't you just
 *  pay the extra 15c and get a REAL separator???)
 */

#ifdef MSDOS
	    if (a[1]==':') /* check for MS-DOS absolute path */
#else
	    if ((a[0]=='/')||(a[0]=='~')) /* check for unix abs path */
#endif
		strcpy(FT->font_name[i],a);

	    else {
		strcpy(FT->font_name[i],font_dir);
		strcat(FT->font_name[i],a);
	    }

	} else {
	    strcpy(FT->font_name[i],font_dir);
	    strcat(FT->font_name[i],TrueTypeLookup[i].pfont);
	}

	if (access(FT->font_name[i], F_OK)!=0) {
	    char msgbuf[1024];
	    sprintf(msgbuf,
		    "plD_FreeType_init: Could not find the freetype compatible font:\n %s",
		    FT->font_name[i]);
	    plwarn(msgbuf);
	}
        FontLookup[i].fci = TrueTypeLookup[i].fci;
        FontLookup[i].pfont = FT->font_name[i];
    }
/*
 * Next, we check to see if -drvopt has been used on the command line to
 * over-ride any settings
 */

}


/*----------------------------------------------------------------------*\
 * FT_SetFace( PLStream *pls, PLUNICODE fci )
 *
 * Sets up the font face and size
\*----------------------------------------------------------------------*/

void FT_SetFace( PLStream *pls, PLUNICODE fci)
{
   FT_Data *FT=(FT_Data *)pls->FT;
   double font_size = pls->chrht * 72/25.4; /* font_size in points, chrht is in mm */
   FT->chrht=pls->chrht;
   if (fci != FT->fci) {
      char *font_name = plP_FCI2FontName(fci, FontLookup, N_TrueTypeLookup);
      if (font_name == NULL) {
	 if (FT->fci == PL_FCI_IMPOSSIBLE)
	   plexit("FT_SetFace: Bad FCI and no previous valid font to fall back on");
	 else
	   plwarn("FT_SetFace: Bad FCI.  Falling back to previous font.");
      } else {
	 FT->fci=fci;

	 if (FT->face!=NULL) {
	    FT_Done_Face(FT->face);
	    FT->face=NULL;
	 }

	 if (FT->face==NULL) {
	    if (FT_New_Face( FT->library,font_name, 0,&FT->face))
	      plexit("FT_SetFace: Error loading a font in freetype");
	 }
      }
   }
   FT_Set_Char_Size(FT->face,0,
		    font_size * 64/TEXT_SCALING_FACTOR,pls->xdpi,
		    pls->ydpi );
}

/*----------------------------------------------------------------------*\
 * plD_render_freetype_text()
 *
 * Transforms the font
 * calculates real-world bitmap coordinates from plplot ones
 * renders text using freetype
\*----------------------------------------------------------------------*/

void plD_render_freetype_text (PLStream *pls, EscText *args)
{
    FT_Data *FT=(FT_Data *)pls->FT;
    int x,y;
    int w=0,h=0;
    PLFLT *t = args->xform;
    FT_Matrix matrix;
    PLFLT angle=PI*pls->diorot/2;
    PLINT clxmin, clxmax, clymin, clymax;
    PLFLT Sin_A,Cos_A;
    FT_Vector adjust;
    PLUNICODE fci;
    FT_Fixed height;
    PLFLT height_factor;

/*
 *  First of all we will see if we are buffering the output. If we are,
 *  then we will take this opportunity to save the text to our local
 *  cached buffer here.
 */
    if ((pls->plbuf_write==1)&&(FT->redraw==0))
       pl_save_FreeType_text_to_buffer (pls, args);

if ((args->string!=NULL)||(args->unicode_array_len>0))
{

/*
 *   Work out if either the font size or the font face has changed.
 *   If either has, then we will reload the font face.
 */
    plgfci(&fci);
    if ((FT->fci!=fci)||(FT->chrht!=pls->chrht))
        FT_SetFace(pls,fci);


/*  this will help work out underlining and overlining*/

    Debug("%d %d %d %d;",FT->face->underline_position>>6,
	   FT->face->descender>>6,
	   FT->face->ascender>>6,
	   ((FT->face->underline_position*-1)+FT->face->ascender)>>6);



/*
 *  Now we work out how long the text is (for justification etc...) and how
 *  high the text is. This is done on UN-TRANSFORMED text, since we will
 *  apply our own transformations on it later, so it's necessary for us
 *  to to turn all transformations off first, before calling the function
 *  that calculates the text size.
 */

    FT->matrix.xx =0x10000;
    FT->matrix.xy =0x00000;
    FT->matrix.yx =0x00000;
    FT->matrix.yy =0x10000;

    FT_Vector_Transform( &FT->pos, &FT->matrix);
    FT_Set_Transform( FT->face, &FT->matrix, &FT->pos );

    FT_StrX_YW(pls,args->unicode_array,args->unicode_array_len,&w, &h);

/*
 *      Set up the transformation Matrix
 *
 * Fortunately this is almost identical to plplot's own transformation matrix;
 * you have NO idea how much effort that saves ! Some params are in a
 * different order, and Freetype wants integers whereas plplot likes floats,
 * but such differences are quite trivial.
 *
 * For some odd reason, this needs to be set a different way for DJGPP. Why ?
 * I wish I knew.
 */

/* (RL, on 2005-01-21) The height_factor variable is introduced below.
 * It is used here and farther below when computing the vertical
 * adjustment.  The rationale for its introduction is as follow: up to
 * now, the text produced with Hershey fonts was systematically taller
 * than the same text produced with TT fonts, and tha by a factor of
 * around 1.125 (I discovered this empirically).  This corresponds
 * roughly to the ratio between total height and the ascender of some
 * TT faces.  Hence the computation below.  Remember that descender is
 * always a negative quantity.
 */

    height_factor = (PLFLT) (FT->face->ascender - FT->face->descender)
                    / FT->face->ascender;
    height = (FT_Fixed) (0x10000 * height_factor);

#ifdef DJGPP
    FT->matrix.xx = height * t[0];
    FT->matrix.xy = height * t[2];
    FT->matrix.yx = height * t[1];
    FT->matrix.yy = height * t[3];
#else
    FT->matrix.xx = height * t[0];
    FT->matrix.xy = height * t[1];
    FT->matrix.yx = height * t[2];
    FT->matrix.yy = height * t[3];
#endif


/*                            Rotate the Font
 *
 *  If the page has been rotated using -ori, this is where we rotate the
 *  font to point in the right direction. To make things nice and easy, we
 *  will use freetypes matrix math stuff to do this for us.
 */

    Cos_A=cos(angle);
    Sin_A=sin(angle);

    matrix.xx =(FT_Fixed)0x10000*Cos_A;

#ifdef DJGPP
    matrix.xy =(FT_Fixed)0x10000*Sin_A*-1;
    matrix.yx =(FT_Fixed)0x10000*Sin_A;
#else
    matrix.xy =(FT_Fixed)0x10000*Sin_A;
    matrix.yx =(FT_Fixed)0x10000*Sin_A*-1;
#endif

    matrix.yy =(FT_Fixed)0x10000*Cos_A;

    FT_Matrix_Multiply(&matrix,&FT->matrix);


/*       Calculate a Vector from the matrix
 *
 * This is closely related to the "transform matrix".
 * The matrix is used for rendering the glyph, while the vector is used for
 * calculating offsets of the text box, so we need both. Why ? I dunno, but
 * we have to live with it, and it works...
 */


    FT_Vector_Transform( &FT->pos, &FT->matrix);


/*    Transform the font face
 *
 * This is where our matrix transformation is calculated for the font face.
 * This is only done once for each unique transformation since it is "sticky"
 * within the font. Font rendering is done later, using the supplied matrix,
 * but invisibly to us from here on. I don't believe the vector is used, but
 * it is asked for.
 */

    FT_Set_Transform( FT->face, &FT->matrix, &FT->pos );


/*                            Rotate the Page
 *
 *  If the page has been rotated using -ori, this is we recalculate the
 *  reference point for the text using plplot functions.
 */

  difilt(&args->x, &args->y, 1, &clxmin, &clxmax, &clymin, &clymax);


/*
 *   Convert into normal coordinates from virtual coordinates
 */

  x=args->x/FT->scale;

 if (FT->invert_y==1)
    y=FT->ymax-(args->y/FT->scale);
 else
    y=args->y/FT->scale;


 /*          Adjust for the justification and character height
  *
  *  Eeeksss... this wasn't a nice bit of code to work out, let me tell you.
  *  I could not work out an entirely satisfactory solution that made
  *  logical sense, so came up with an "illogical" one as well.
  *  The logical one works fine for text in the normal "portrait"
  *  orientation, and does so for reasons you might expect it to work; But
  *  for all other orientations, the text's base line is either a little
  *  high, or a little low. This is because of the way the base-line pos
  *  is calculated from the decender height. The "dodgie" way of calculating
  *  the position is to use the character height here, then adjust for the
  *  decender height by a three-fold factor later on. That approach seems to
  *  work a little better for rotated pages, but why it should be so, I
  *  don't understand. You can compile in or out which way you want it by
  *  defining "DODGIE_DECENDER_HACK".
  *
  *  note: the logic of the page rotation coming up next is that we pump in
  *  the justification factor and then use freetype to rotate and transform
  *  the values, which we then use to change the plotting location.
  */


#ifdef DODGIE_DECENDER_HACK
    adjust.y=h;
#else
    adjust.y=0;
#endif

/* (RL, on 2005-01-24) The code below uses floating point and division
 * operations instead of integer shift used before. This is slower but
 * gives accurate placement of text in plots.
 */

/* (RL, on 2005-01-21) The hack below is intended to align single
 * glyphs being generated via plpoin.  The way to detect this
 * situation is completely hackish, I must admit, by checking whether the
 * length of the Unicode array is equal 2 and whether the first
 * character is actually a font-changing command to font number 4 (for
 * symbols).  This is ugly because it depends on definitions set
 * elsewhere, but it works.
 *
 * The computation of the vertical and horizontal adjustments are
 * based on the bouding box of the glyph being loaded (since there is
 * only one glyph in the string in this case, we are okay here).
 */

    if ((args->unicode_array_len == 2)
      && (args->unicode_array[0] == 0x10000004))
    {
        adjust.x = args->just * ROUND (FT->face->glyph->metrics.width / 64.0);
        adjust.y = (FT_Pos) ROUND (FT->face->glyph->metrics.height / 128.0);
    }
    else
    {

/* (RL, on 2005-01-21) The vertical adjustment is set below, making
 * the DODGIE conditional moot.  I use the value of h as return by FT_StrX_YW,
 * which should correspond to the total height of the text being
 * drawn.  Freetype aligns text around the baseline, while PLplot
 * aligns to the center of the ascender portion.  We must then adjust
 * by half of the ascender and this is why there is a division by
 * height_factor below.
 */

      adjust.y = (FT_Pos)
          ROUND (FT->face->size->metrics.height / height_factor / 128.0);
      adjust.x = (FT_Pos) (args->just * ROUND (w / 64.0));

    }

    FT_Vector_Transform( &adjust, &FT->matrix);    /* was /&matrix); -  was I using the wrong matrix all this time ?*/

    x-=adjust.x;
    y+=adjust.y;

    FT_WriteStrW(pls,args->unicode_array,args->unicode_array_len,x,y); /* write it out */
}
else
{
plD_render_freetype_sym (pls, args);
}

}

/*----------------------------------------------------------------------*\
 * plD_FreeType_Destroy()
 *
 * Restores cmap0 if it had been modifed for anti-aliasing
 * closes the freetype library.
 * Deallocates memory to the Freetype structure
\*----------------------------------------------------------------------*/

void plD_FreeType_Destroy(PLStream *pls)
{
    FT_Data *FT=(FT_Data *)pls->FT;
    extern int FT_Done_Library( FT_Library  library );

    if (FT) {
	if (FT->smooth_text==1) plscmap0n(FT->ncol0_org);
   if (pls->plbuf_write==1)
    {
      if (FT->text_cache!=NULL)
         free(FT->text_cache);
    }

	FT_Done_Library(FT->library);
	free(FT->textbuf);
	free(pls->FT);
	pls->FT=NULL;
    }
}

/*----------------------------------------------------------------------*\
 * PLFLT CalculateIncrement( int bg, int fg, int levels)
 *
 * Takes the value of the foreground, and the background, and when
 * given the number of desired steps, calculates how much to incriment
 * a value to transition from fg to bg.
 * This function only does it for one colour channel at a time.
\*----------------------------------------------------------------------*/

static PLFLT CalculateIncrement( int bg, int fg, int levels)
{
    PLFLT ret=0;

    if (levels>1) {
	if (fg>bg)
	    ret=((fg+1)-bg)/levels;
	else if (fg<bg)
	    ret=(((fg-1)-bg)/levels);
    }
    return(ret);
}

/*----------------------------------------------------------------------*\
 * void pl_set_extended_cmap0(PLStream *pls, int ncol0_width, int ncol0_org)
 *
 *  ncol0_width - how many greyscale levels to accolate to each CMAP0 entry
 *  ncol0_org - the originl number of CMAP0 entries.
 *
 *  This function calcualtes and sets an extended CMAP0 entry for the
 *  driver. It is assumed that the caller has checked to make sure there is
 *  room for extending CMAP0 already.
 *
 *  NOTES
 *  We don't bother calculating an entry for CMAP[0], the background.
 *  It is assumed the caller has already expanded the size of CMAP[0]
\*----------------------------------------------------------------------*/

void pl_set_extended_cmap0(PLStream *pls, int ncol0_width, int ncol0_org)
{
    int i,j,k;
    int r,g,b;
    PLFLT r_inc,g_inc,b_inc;

    for (i=1;i<ncol0_org;i++) {

	r=pls->cmap0[i].r;
	g=pls->cmap0[i].g;
	b=pls->cmap0[i].b;

	r_inc=CalculateIncrement(pls->cmap0[0].r,r,ncol0_width);
	g_inc=CalculateIncrement(pls->cmap0[0].g,g,ncol0_width);
	b_inc=CalculateIncrement(pls->cmap0[0].b,b,ncol0_width);

	for (j=0,k=ncol0_org+i-1;j<ncol0_width;j++,k+=(ncol0_org-1)) {
	    r-=r_inc;
	    g-=g_inc;
	    b-=b_inc;
	    if ((r<0)||(g<0)||(b<0))
		plscol0 (k, 0, 0, 0);
	    else
		plscol0 (k, (r > 0xff ? 0xff : r), (g > 0xff ? 0xff : g), (b > 0xff ? 0xff : b));
        }
    }
}

/*--------------------------------------------------------------------------*\
 * void pl_save_FreeType_text_to_buffer (PLStream *pls, EscText *args)
 *
 * Function to save the text calls to freetype to a local cache which we
 * will later use to replay the commands, and therefore plot the text
 * (since the plReplot function doesn't cache text as "text" but rather as
 * vectors.) We need to save the EscText structure, naturally, as well as
 * the string pointed to in that structure, the colour of text from pls,
 * and also the size of text from pls.
\*--------------------------------------------------------------------------*/

static void pl_save_FreeType_text_to_buffer (PLStream *pls, EscText *args)
{
  FT_Data *FT=(FT_Data *)pls->FT;
  unsigned short len=0;
  unsigned short unicode_len=0,mem_used_by_unicode=0;
  unsigned short total_mem;
  int i;


  if (args->string!=NULL) len=strlen(args->string); /* If no string, then the length will be 0 */
  unicode_len=args->unicode_array_len;      /* Length of the unicode string */
  mem_used_by_unicode=sizeof(PLUNICODE)*unicode_len;


/*
 * Calcualte how much memory this piece of text will take up
 * We have to add in the text structure, text colour, the transform matrix,
 * the font size, and the text itself
 */

  total_mem=len+mem_used_by_unicode+sizeof(unsigned short)+sizeof(PLINT)+sizeof(EscText)+(4*sizeof(PLFLT))+1;

  i=FT->mem_pointer;

  /*
   *  Next we check to see if the cache has enough memory in it for the text
   *  it was just asked to draw - if it doesn't we will try to realloc some
   *  more memory to it. Assuming we get this extra memory, we will keep it
   *  until the program terminates.
   */

  while ((FT->mem_pointer+total_mem)>FT->mem_allocated)
    {
     if ((FT->text_cache=(realloc(FT->text_cache,(size_t) (FT_TEXT_CACHESZ+FT->mem_allocated))))==NULL)
        {
        	plexit("Could not allocate extra memory to Freetype text cache");
        }
     FT->mem_allocated+=FT_TEXT_CACHESZ;
    }

  /*
   *  Now we will start copying the stuff we want to keep to our cache
   *  This isn't the most efficient way of doing stuff in terms of speed,
   *  for example, if we really wanted this to be fast we would keep things
   *  on 32 bit boundaries, but it's good enough for our purposes.
   *  We basically have a pair system of a memcpy command to copy the stuff
   *  to the cache, then advance the pointer for the next write.
   */

  memcpy(&FT->text_cache[i],&len,sizeof(unsigned short));  /* Length of the text string we are printing */
  i+=sizeof(unsigned short);

  memcpy(&FT->text_cache[i],&pls->icol0,sizeof(PLINT));  /* Text colour */
  i+=sizeof(PLINT);

  memcpy(&FT->text_cache[i],&pls->chrht,sizeof(PLFLT));  /* Text size */
  i+=sizeof(PLFLT);

  memcpy(&FT->text_cache[i],&FT->scale,sizeof(PLFLT));  /* scale */
  i+=sizeof(PLFLT);

  memcpy(&FT->text_cache[i],args,sizeof(EscText));
  i+=sizeof(EscText);

  if (args->xform!=NULL)   /* Do not try to copy the matrix if it is blank, or BAD THINGS will happen */
    {
      memcpy(&FT->text_cache[i],args->xform,(4*sizeof(PLFLT)));
    }
  i+=(4*sizeof(PLFLT));

  if (args->string!=NULL)
    {
      memcpy(&FT->text_cache[i],args->string,len+1); /* Add the "len+1" at the end to get the terminating NULL */
    }
  else if (args->unicode_array_len>0)
    {
      memcpy(&FT->text_cache[i],args->unicode_array,mem_used_by_unicode); /* Add the "len+1" at the end to get the terminating NULL */
      i+=mem_used_by_unicode;
    }

  i+=(len+1);

  FT->mem_pointer=i; /* Advance the memory pointer */
  FT->num_strings++;

}

/*--------------------------------------------------------------------------*\
 *  void pl_RemakeFreeType_text_from_buffer (PLStream *pls)
 *
 *  The function replays the contents of our "local" plot text buffer to the
 *  freetype engine, therefore replaying the text drawing commands.
\*--------------------------------------------------------------------------*/

void pl_RemakeFreeType_text_from_buffer (PLStream *pls)
{
  FT_Data *FT=(FT_Data *)pls->FT;
  unsigned short len;
  PLINT colour, last_colour;
  PLFLT chrht, last_chrht,scale;
  int i,j;
  EscText text;

  if ((pls->plbuf_write==1)&&(pls->dev_text==1)&&(FT->num_strings>0))
    {

    /*
     *  Save the current colours and font size so we can restore stuff later. Should not
     *  be needed since this function should ONLY be getting called at the END of pages,
     *  when all plotting is done, and since the BOP functions should be setting these
     *  up anyway before the start of each page... BUT best done just in case.
     */

      last_colour=pls->icol0;
      last_chrht=pls->chrht;

      for (i=j=0;j<FT->num_strings;j++)
        {
          memcpy(&len,&FT->text_cache[i],sizeof(unsigned short));  /* Length of the text string we are printing */
          i+=sizeof(unsigned short);

          memcpy(&colour,&FT->text_cache[i],sizeof(PLINT));  /* Text writing colour */
          i+=sizeof(PLINT);
          pls->icol0=colour;

          memcpy(&chrht,&FT->text_cache[i],sizeof(PLFLT));  /* Text size */
          i+=sizeof(PLFLT);

          memcpy(&scale,&FT->text_cache[i],sizeof(PLFLT));  /* driver scale */
          i+=sizeof(PLFLT);

          memcpy(&text,&FT->text_cache[i],sizeof(EscText)); /* Structure which holds everything */
          i+=sizeof(EscText);

          text.xform=(PLFLT *)&FT->text_cache[i]; /* We just point back to our buffer :-) */
          i+=(4*sizeof(PLFLT));

          if (len>0)
            {
              text.string=&FT->text_cache[i];  /* Again, we just point to the copy in our buffer */
            }
          else if (text.unicode_array_len>0)
            {
              text.unicode_array= (PLUNICODE *) &FT->text_cache[i];
              i+=text.unicode_array_len*sizeof(PLUNICODE);
            }
          i+=(len+1);

          pls->chrht=chrht*scale/FT->scale;

        /*
         *   Since we are calling plD_render_freetype_text here, we need
         *   to set a special "redraw" flag to make sure we don't get
         *   recursion, since "plD_render_freetype_text" also ADDs text
         *   to the text buffer.
         */

          FT->redraw=1;
          plD_render_freetype_text(pls, &text);
          FT->redraw=0;
        }
      pls->icol0=last_colour;     /* Reset the colour */
      pls->chrht=last_chrht;      /* reset the font size */

    }
}

/*----------------------------------------------------------------------*\
 * plD_render_freetype_sym( PLStream *pls, EscText *args )
 *   PLStream *pls - pointer to plot stream
 *   EscText *args - pointer to standard "string" object.
 *
 *  This function is a simple rendering function which draws a single
 *  character at a time. The function is an alternative to the text
 *  functions which are considerably, and needlessly, more complicated
 *  than what we need here.
\*----------------------------------------------------------------------*/


void plD_render_freetype_sym (PLStream *pls, EscText *args)
{
    FT_Data *FT=(FT_Data *)pls->FT;
    int x,y;
    FT_Vector  adjust;
    PLUNICODE fci;

    x=args->x/FT->scale;

 if (FT->invert_y==1)
    y=FT->ymax-(args->y/FT->scale);
 else
    y=args->y/FT->scale;



/*
 *  Adjust for the descender - make sure the font is nice and centred
 *  vertically. Freetype assumes we have a base-line, but plplot thinks of
 *  centre-lines, so that's why we have to do this. Since this is one of our
 *  own adjustments, rather than a freetype one, we have to run it through
 *  the transform matrix manually.
 *
 *  For some odd reason, this works best if we triple the
 *  descender's height and then adjust the height later on...
 *  Don't ask me why, 'cause I don't know. But it does seem to work.
 *
 *  I really wish I knew *why* it worked better though...
 *
 *   y-=FT->face->descender >> 6;
 */

#ifdef DODGIE_DECENDER_HACK
    adjust.y= (FT->face->descender >> 6)*3;
#else
    adjust.y= (FT->face->descender >> 6);
#endif

    adjust.x=0;
    FT_Vector_Transform( &adjust, &FT->matrix);
    x+=adjust.x;
    y-=adjust.y;

    plgfci(&fci);
    FT_SetFace(pls,fci);

    FT=(FT_Data *)pls->FT;
    FT_Set_Transform( FT->face, &FT->matrix, &FT->pos );

    FT_Load_Char( FT->face, args->unicode_char, (FT->smooth_text==0) ? FT_LOAD_MONOCHROME+FT_LOAD_RENDER : FT_LOAD_RENDER|FT_LOAD_FORCE_AUTOHINT );

/*
 * Now we have to try and componsate for the fact that the freetype glyphs are left
 * justified, and plplot's glyphs are centred. To do this, we will just work out the
 * advancment, halve it, and take it away from the x position. This wont be 100%
 * accurate because "spacing" is factored into the right hand side of the glyph,
 * but it is as good a way as I can think of.
 */

    x -= (FT->face->glyph->advance.x >> 6)/2;
    FT_PlotChar(pls,FT, FT->face->glyph,  x, y, pls->icol0 ); /* render the text */

}




#else
int
plfreetype()
{
    return 0;
}

#endif
