/* $Id: plfreetype.h 3186 2006-02-15 18:17:33Z slbrow $
 *
 *    Header file for Support routines for freetype font engine
 *
 *    See plfreetype.c for more details

    Copyright (C) 2004  Andrew Roach

    This file is part of PLplot.

    PLplot is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as published
    by the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    PLplot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with PLplot; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


 */

#ifndef __PLFREETY_H__
#define __PLFREETY_H__

#ifdef HAVE_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

#define FT_Data _FT_Data_

typedef void (*plD_pixel_fp)     (PLStream *, PLINT, PLINT);

/*--------------------------------------------------------------------------*\
 * Define the FT_Data data structure.
 *
 * These are the "globalish" variables used by Freetype
 * They are kept here so they are moderately thread safe, and stream specific
\*--------------------------------------------------------------------------*/

typedef struct FT_Data {
    short               x;
    short               y;

    char *textbuf;		/* temporary string buffer */

/*
 *  If set to 1, scale won't do anything, but this is an "arbitrary" scale
 *  factor for the transformation between virtual and real coordinates. This
 *  is included to fix up the problem with the "hidden line removal bug" of
 *  the 3D plots, which is fixed by providing a super-scaled image. This
 *  should be a mirror, for example, of dev->scale in the PNG driver. If I
 *  was thinking 12 months ahead, I would have put that scale factor in
 *  "pls", not "dev", but at this late stage, we can just live with it
 *  now...
 */
    PLFLT		scale;

    unsigned char       greek;

/*
 *  Set "invert_y" to 1 if the y coordinates need to be inverted for
 *  plotting. Most bitmaps will need this.
 */
    unsigned char       invert_y;

/*
 *  ymax should be equal to, what it says - the maximum y coordinate of the
 *  bitmap. This is used in the process of calculating the inversion of the
 *  bitmap when invert_y is set to 1. If invert_y isn't set, this setting is
 *  ignored.
 */
    short               ymax;


    plD_pixel_fp        pixel;          /* pointer to a function which draws a single pixel */


    int want_smooth_text; /* flag to request text smoothing (won't */
                          /* necessarily get it though */
    int smooth_text;      /* Flag to indicate type of anti-aliasing used, if freetype text is active */


/*
 *  List of font names and paths corresponding to the "predefined" fonts of
 *  plplot. 1024 chars is presumably generous for each one's length, but at
 *  least we probably won't get in trouble this way.
 */

    /* 30 = five families * 3 styles * 2 weights = N_TrueTypeLookup */
    char                font_name[30][1024];

/*
 *  This is a mirror of pls->fci and is basically used for detecting when
 *  fonts have been changed .
 */

    PLUNICODE               fci;
    PLFLT chrht; /* Mirror of pls's copy so we can detect when the font SIZE has been changed */

    FT_Matrix           matrix;         /* used for rotating etc... the font. */
    FT_Vector           pos;            /* used for calculating offsets of text boxes/sizes */


/*
 *  The next few variables hold the original size of CMAP0, the number of
 *  extra slots added for anti-aliasing, and the "width" of the table used
 *  for anti-aliasing.
 */

    PLINT ncol0_org;            /* Original number of colours in CMAP0 */
    PLINT ncol0_xtra;           /* number of extra colours defined in CMAP0 for anti-aliasing */
    PLINT ncol0_width;          /* Number of greyscale levels for each of the original colours */
    PLINT last_icol0;           /* Last colour in cmap0, which should be one of the originals */

/*
 *  The next block of variables are used to store text within plplot's local
 *  text buffer. This buffer is used to overcome the unfortunate fact that
 *  the API does not cache text for plRemakePlot calls
 */

    char          *text_cache;         /* We have to maintain our own text buffer for each line */
                                       /* of text we are asked to draw because plplot's API doesn't */
    int           mem_allocated;
    int           mem_pointer;         /* Pointer of where we are up to in our buffer */
    int           num_strings;         /* Number of strings to be drawn by freetype */
    char          redraw;              /* Flag used to stop ugly recursion */


/*
 *  The rest of the variables should be considered very much PRIVATE, and
 *  more to the point, subject to change.
 *
 *  Don't rely on them existing in future versions of plplot's freetype
 *  support. If/when the Freetype cache manager is added to plplot, most, if
 *  not all, of these variables will move elsewhere.
 */

    FT_Library          library;        /* pointer to freetype library      */
    FT_Face             face;           /* pointer to a font face           */
    FT_GlyphSlot        slot;           /* pointer to a glyph slot          */
    FT_Glyph            image;          /* bitmap or outline image of font  */

    short               colour;         /* depreciated ?? must check code */

    PLINT shade, col_idx;		/* Used for antialiasing */

} FT_Data;


/*  Macros that look like functions  */

#define pl_FreeTypeBOP() \
   if ((pls->plbuf_write==1)&&(pls->dev_text==1)) \
      if (FT->text_cache!=NULL) \
          FT->num_strings=FT->mem_pointer=0; \

#endif

#endif /* __PLFREETY_H__ */
