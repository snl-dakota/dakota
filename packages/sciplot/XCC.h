/* $Id: XCC.h 404 1999-09-28 15:56:30Z bhdenni $ */
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

#ifndef _XCC_h
#define _XCC_h

typedef struct _XColorContext *XCC;



#if NeedFunctionProtoTypes

extern XCC XCCCreate(Display *_dpy, Visual *_visual, int _usePrivateColormap, 
						int _useStdCmaps, Atom _stdColormapAtom, Colormap *_colormap);
extern XCC XCCMonoCreate(Display *_dpy, Visual *_visual, Colormap _colormap);
extern void XCCFree(XCC _xcc);

extern unsigned long XCCGetPixel(XCC _xcc, unsigned int _red, unsigned int _green, unsigned int _blue);
extern int XCCGetNumColors(XCC _xcc);

extern Colormap XCCGetColormap(XCC _xcc);
extern Visual *XCCGetVisual(XCC _xcc);
extern XVisualInfo *XCCGetVisualInfo(XCC _xcc);
extern Display *XCCGetDisplay(XCC _xcc);
extern int XCCQueryColor(XCC _xcc, XColor *_color);
extern int XCCQueryColors(XCC _xcc, XColor *_colors, int _numColors);



#else

extern XCC XCCCreate();
extern XCC XCCMonoCreate();
extern void XCCFree();
extern unsigned long XCCGetPixel();
extern int XCCGetNumColors();
extern Colormap XCCGetColormap();
extern Visual *XCCGetVisual();
extern XVisualInfo *XCCGetVisualInfo();
extern Display *XCCGetDisplay();
extern int XCCQueryColor();
extern int XCCQueryColors();

#endif /* NeedFunctionProtoTypes */


#endif /* _XCC_H */
