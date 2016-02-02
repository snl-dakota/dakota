/* $Id: XCCP.h 404 1999-09-28 15:56:30Z bhdenni $ */
/*
 * Copyright 1994,1995 John L. Cwikla
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of John L. Cwikla or
 * Wolfram Research, Inc not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.  John L. Cwikla and Wolfram Research, Inc make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * John L. Cwikla and Wolfram Research, Inc disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall John L. Cwikla or
 * Wolfram Research, Inc be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
 * Author:
 *  John L. Cwikla
 *  X Programmer
 *  Wolfram Research Inc.
 *
 *  cwikla@wri.com
*/

#ifndef _XCCP_h
#define _XCCP_h

#include "XCC.h"

#define MODE_UNDEFINED 0 /* UNKNOWN */
#define MODE_BW 1			/* Default B+W */
#define MODE_STDCMAP 2		/* Has a stdcmap */
#define MODE_TRUE 3			/* Is a truecolor/direcolor visual */
#define MODE_MY_GRAY 4	/* my grayramp */

struct _XColorContext
{
	Display *dpy;
	Visual *visual;
	Colormap colormap;
	XVisualInfo *visualInfo;

	int numColors;

	char mode;
	char needToFreeColormap;
	Atom stdCmapAtom;

	XStandardColormap stdCmap;
	unsigned long *CLUT;
	XColor *CMAP;

	struct 
	{
		int red;
		int green;
		int blue;
	} shifts;
	struct
	{
		unsigned long red;
		unsigned long green;
		unsigned long blue;
	} masks;
	struct
	{
		int red;
		int green;	
		int blue;
	} bits;
	unsigned long maxEntry;

	unsigned long blackPixel, whitePixel;
};


#endif /* _XCCP_h */
