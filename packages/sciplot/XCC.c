/* $Id: XCC.c 404 1999-09-28 15:56:30Z bhdenni $ */
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

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "XCCP.h"

#define NUMBER(a) ((int)(sizeof(a)/sizeof(a[0])))

#if NeedFunctionProtoTypes
static int _pixelSort(const void *_arg1, const void *_arg2)
#else
static int _pixelSort(_arg1, _arg2)
void *_arg1;
void *_arg2;
#endif /* NeedFunctionProtoTypes */
{
	return ( ((XColor *)_arg1)->pixel - ((XColor *)_arg2)->pixel);
}

#if NeedFunctionProtoTypes
static void _queryColors(XCC _xcc)
#else
static void _queryColors(_xcc)
XCC _xcc;
#endif
{
	int i;

	_xcc->CMAP = (XColor *)Xmalloc(sizeof(XColor) * _xcc->numColors);
	if (_xcc->CMAP)
	{
		for(i=0;i<_xcc->numColors;i++)
			_xcc->CMAP[i].pixel = _xcc->CLUT ? _xcc->CLUT[i] : _xcc->stdCmap.base_pixel + i;

		if (!XQueryColors(_xcc->dpy, _xcc->colormap, _xcc->CMAP, _xcc->numColors))
		{
			XFree(_xcc->CMAP);
			_xcc->CMAP = NULL;
		}
		else
			qsort(_xcc->CMAP, _xcc->numColors, sizeof(XColor), _pixelSort);
	}
}

/*
** Find a standard colormap from a property, and make sure the visual matches
** the one we are using!
*/

#if NeedFunctionProtoTypes
static int _findGoodCube(XCC _xcc, Atom _atom, XStandardColormap *_matchedCube)
#else
static int _findGoodCube(_xcc, _atom, _matchedCube)
XCC _xcc;
Atom _atom;
XStandardColormap *_matchedCube;
#endif /* NeedFunctionProtoTypes */
{
	XStandardColormap *cubes, *match;
	int status;
	int count;
	int i;

	if (!_atom)
		return 0;

	cubes = (XStandardColormap *)NULL;
	status = XGetRGBColormaps(_xcc->dpy, RootWindow(_xcc->dpy, DefaultScreen(_xcc->dpy)),
				&cubes, &count, _atom);

	match = NULL;
	if (status)
	{
		status = 0;
		for(i=0;(match == NULL) && (i<count);i++)
		{
			if (cubes[i].visualid == _xcc->visualInfo->visualid)
			{
				match = cubes+i;
				status = 1;
			}
		}
	}

	if (match)
		*_matchedCube = *match;

	if (cubes)
		XFree((char *)cubes);

	return status;
}

/*
** Find a standard cmap if it exists.
*/

Atom ColorAtomList[] =
{
	XA_RGB_DEFAULT_MAP,
	XA_RGB_BEST_MAP,
	XA_RGB_GRAY_MAP,
};
Atom GrayAtomList[] =
{
	XA_RGB_GRAY_MAP,
	XA_RGB_DEFAULT_MAP,
};

#if NeedFunctionProtoTypes
static int _lookForStdCmap(XCC _xcc, Atom _atom)
#else
static int _lookForStdCmap(_xcc, _atom)
XCC _xcc;
Atom _atom;
#endif /* NeedFunctionProtoTypes */
{
	int status;
	int i;

	status = 0;

	if ((status = _findGoodCube(_xcc, _atom, &_xcc->stdCmap)) == 0)
		switch(_xcc->visualInfo->class)
		{
			case TrueColor: /* HMMM? */
			case StaticColor:
			case PseudoColor:
			case DirectColor:

				for(i=0;i<NUMBER(ColorAtomList);i++)
					if ((status = _findGoodCube(_xcc, ColorAtomList[i], &_xcc->stdCmap)) != 0)
						break;

				break;

			case StaticGray:
			case GrayScale:
		
				for(i=0;i<NUMBER(GrayAtomList);i++)
					if ((status = _findGoodCube(_xcc, GrayAtomList[i], &_xcc->stdCmap)) != 0)
						break;
				break;
		}

	if (!status)
		return 0;

/*
** This is a hack to force standard colormaps that don't set green/blue max
** to work correctly.  For instance RGB_DEFAULT_GRAY has these set if xstdcmap
** is used, but not if xscm was. Plus this also makes RGB_RED_MAP (etc) work.
*/

	_xcc->colormap = _xcc->stdCmap.colormap;

	if ( (!_xcc->stdCmap.green_max + !_xcc->stdCmap.blue_max + !_xcc->stdCmap.red_max) > 1)
	{
		_xcc->mode = MODE_MY_GRAY;
		if (_xcc->stdCmap.green_max)
		{
			_xcc->stdCmap.red_max = _xcc->stdCmap.green_max;
			_xcc->stdCmap.red_mult = _xcc->stdCmap.green_mult;
		}
		else
		if (_xcc->stdCmap.blue_max)
		{
			_xcc->stdCmap.red_max = _xcc->stdCmap.blue_max;
			_xcc->stdCmap.red_mult = _xcc->stdCmap.blue_mult;
		}
		_xcc->stdCmap.green_max = _xcc->stdCmap.blue_max = 0;
		_xcc->stdCmap.green_mult = _xcc->stdCmap.blue_mult = 0;
	}
	else
		_xcc->mode = MODE_STDCMAP;

	_xcc->numColors = (_xcc->stdCmap.red_max+1) *
					 (_xcc->stdCmap.green_max+1) *
					 (_xcc->stdCmap.blue_max+1);

	_queryColors(_xcc);

	return status;
}

/*
** If we die, we go to the world of B+W
*/

#if NeedFunctionProtoTypes
static void _initBW(XCC _xcc)
#else
static void _initBW(_xcc)
XCC _xcc;
#endif /* NeedFuntionProtoTypes */
{
	XColor color;

	_xcc->mode = MODE_BW;

	color.red = color.blue = color.green = 0;
	if (!XAllocColor(_xcc->dpy, _xcc->colormap, &color))
		_xcc->blackPixel = 0;
	else
		_xcc->blackPixel = color.pixel;

	color.red = color.blue = color.green = 0xFFFF;
	if (!XAllocColor(_xcc->dpy, _xcc->colormap, &color))
		_xcc->whitePixel = _xcc->blackPixel ? 0 : 1;
	else
		_xcc->whitePixel = color.pixel;

	_xcc->numColors = 2;
}

/*
** Make our life easier and ramp our grays. Note
** that each lookup is /= 2 of the previous.
*/

#if NeedFunctionProtoTypes
static void _initGray(XCC _xcc)
#else
static void _initGray(_xcc)
XCC _xcc;
#endif /* NeedFunctionProtoTypes */
{
	XColor *clrs, *cstart;
	register int i;
	double dinc;

	_xcc->numColors = _xcc->visual->map_entries;

	_xcc->CLUT = (unsigned long *)Xmalloc(sizeof(unsigned long) * _xcc->numColors);
	cstart = (XColor *)Xmalloc(sizeof(XColor) * _xcc->numColors);

retryGray:

	dinc = 65535.0/(_xcc->numColors-1);

	clrs = cstart;
	for(i=0;i<_xcc->numColors;i++)
	{
		clrs->red = clrs->blue = clrs->green = dinc * i;
		if (!XAllocColor(_xcc->dpy, _xcc->colormap, clrs))
		{
			XFreeColors(_xcc->dpy, _xcc->colormap, _xcc->CLUT, i, 0);

			_xcc->numColors /= 2;

			if (_xcc->numColors > 1)
				goto retryGray;
			else
			{
				XFree((char *)_xcc->CLUT);
				_xcc->CLUT = NULL;
				_initBW(_xcc);
				XFree((char *)cstart);
				return;
			}
		}
		_xcc->CLUT[i] = clrs++->pixel;
	}

	XFree((char *)cstart);

	_xcc->stdCmap.colormap = _xcc->colormap;
	_xcc->stdCmap.base_pixel = 0;
	_xcc->stdCmap.red_max = _xcc->numColors-1;
	_xcc->stdCmap.green_max = 0;
	_xcc->stdCmap.blue_max = 0;
	_xcc->stdCmap.red_mult = 1;
	_xcc->stdCmap.green_mult = _xcc->stdCmap.blue_mult = 0;

	_queryColors(_xcc);

	_xcc->mode = MODE_MY_GRAY;
}

/*
** These are the color visuals that require a CLUT
*/

#if NeedFunctionProtoTypes
static void _initColor(XCC _xcc)
#else
static void _initColor(_xcc)
XCC _xcc;
#endif /* NeedFunctionProtoTypes */
{
	XColor *clrs, *cstart;
	register int red, green, blue;
	double dinc;
	int cubeval;
	int count;

	cubeval = 1;
	while((cubeval*cubeval*cubeval) < _xcc->visual->map_entries)
		cubeval++;
	cubeval--;
	_xcc->numColors = cubeval * cubeval * cubeval;

	_xcc->CLUT = (unsigned long *)Xmalloc(sizeof(unsigned long) * _xcc->numColors);
	cstart = (XColor *)Xmalloc(sizeof(XColor) * _xcc->numColors);

retryColor:

	_xcc->stdCmap.red_max = cubeval - 1;
	_xcc->stdCmap.green_max = cubeval - 1;
	_xcc->stdCmap.blue_max = cubeval - 1;
	_xcc->stdCmap.red_mult = cubeval * cubeval;
	_xcc->stdCmap.green_mult = cubeval;
	_xcc->stdCmap.blue_mult = 1;
	_xcc->stdCmap.base_pixel = 0;

	clrs = cstart;
	count = 0;
	for (red = 0; red <= _xcc->stdCmap.red_max; red++) 
	{
		for (green = 0; green <= _xcc->stdCmap.green_max; green++) 
		{
			for(blue = 0; blue <= _xcc->stdCmap.blue_max; blue++)
			{
				if (_xcc->stdCmap.red_max)
					clrs->red = 0xFFFF * red / _xcc->stdCmap.red_max;
				else
					clrs->red = 0;
				if (_xcc->stdCmap.green_max)
					clrs->green = 0xFFFF * green / _xcc->stdCmap.green_max;
				else
					clrs->green = 0;
				if (_xcc->stdCmap.blue_max)
					clrs->blue = 0xFFFF * blue / _xcc->stdCmap.blue_max;
				else
					clrs->blue = 0;

				if (!XAllocColor(_xcc->dpy, _xcc->colormap, clrs))
				{
					XFreeColors(_xcc->dpy, _xcc->colormap, _xcc->CLUT, count, 0);

					cubeval--;

					if (cubeval > 1)
						goto retryColor;
					else
					{
						XFree((char *)_xcc->CLUT);
						_xcc->CLUT = NULL;
						_initBW(_xcc);
						XFree((char *)cstart);
						return;
					}

				}
				_xcc->CLUT[count++] = clrs++->pixel;
			}
		}
	}

	_xcc->numColors = count;

	_queryColors(_xcc);

	XFree((char *)cstart);

	_xcc->mode = MODE_STDCMAP;
}

/*
** Get our shifts and masks
*/

#if NeedFunctionProtoTypes
static void _initTrueColor(XCC _xcc)
#else
static void _initTrueColor(_xcc)
XCC _xcc;
#endif /* NeedFunctionProtoTypes */
{
	register unsigned long rmask, gmask, bmask;

	_xcc->mode = MODE_TRUE;

	rmask = _xcc->masks.red = _xcc->visualInfo->red_mask;
	_xcc->shifts.red = 0;
	_xcc->bits.red = 0;
	while (!(rmask & 1))
	{
		rmask >>= 1;
		_xcc->shifts.red++;
	}
	while((rmask & 1))
	{
		rmask >>= 1;
		_xcc->bits.red++;
	}

	gmask = _xcc->masks.green = _xcc->visualInfo->green_mask;
	_xcc->shifts.green = 0;
	_xcc->bits.green = 0;
	while (!(gmask & 1))
	{
		gmask >>= 1;
		_xcc->shifts.green++;
	}
	while(gmask & 1)
	{
		gmask >>= 1;
		_xcc->bits.green++;
	}

	bmask = _xcc->masks.blue = _xcc->visualInfo->blue_mask;
	_xcc->shifts.blue = 0;
	_xcc->bits.blue = 0;
	while (!(bmask & 1))
	{
		bmask >>= 1;
		_xcc->shifts.blue++;
	}
	while(bmask & 1)
	{
		bmask >>= 1;
		_xcc->bits.blue++;
	}

	_xcc->numColors = ((_xcc->visualInfo->red_mask) | 
						(_xcc->visualInfo->green_mask) | 
						(_xcc->visualInfo->blue_mask)) + 1;
}

/*
** Cheat here! Make the direct color visual work like
** a true color! USE the CLUT!!!
*/

#if NeedFunctionProtoTypes
static void _initDirectColor(XCC _xcc)
#else
static void _initDirectColor(_xcc)
XCC _xcc;
#endif /* NeedFunctionProtoTypes */
{
	int n, count;
	XColor *clrs, *cstart;
	unsigned long rval, bval, gval;
	unsigned long *rtable;
	unsigned long *gtable;
	unsigned long *btable;
	double dinc;

	_initTrueColor(_xcc); /* for shift stuff */

	rval = _xcc->visualInfo->red_mask >> _xcc->shifts.red;
	gval = _xcc->visualInfo->green_mask >> _xcc->shifts.green;
	bval = _xcc->visualInfo->blue_mask >> _xcc->shifts.blue;

	rtable = (unsigned long *)Xmalloc(sizeof(unsigned long) * (rval+1));
	gtable = (unsigned long *)Xmalloc(sizeof(unsigned long) * (gval+1)); 
	btable = (unsigned long *)Xmalloc(sizeof(unsigned long) * (bval+1));

	_xcc->maxEntry = (rval > gval) ? rval : gval;
	_xcc->maxEntry = (_xcc->maxEntry > bval) ? _xcc->maxEntry : bval;

	cstart = (XColor *)Xmalloc(sizeof(XColor) * (_xcc->maxEntry+1));
	_xcc->CLUT = (unsigned long *)Xmalloc(sizeof(unsigned long) * (_xcc->maxEntry+1));

retrydirect:

	for(n=0;n<=rval;n++)
		rtable[n] = rval ? 65535.0/(double)rval * n : 0;
	for(n=0;n<=gval;n++)
		gtable[n] = gval ? 65535.0/(double)gval * n : 0;
	for(n=0;n<=bval;n++)
		btable[n] = bval ? 65535.0/bval * n : 0;

	_xcc->maxEntry = (rval > gval) ? rval : gval;
	_xcc->maxEntry = (_xcc->maxEntry > bval) ? _xcc->maxEntry : bval;

	count = 0;
	clrs = cstart;
	_xcc->numColors = (bval + 1) * (gval + 1) * (rval + 1);
	for(n=0;n<=_xcc->maxEntry;n++)
	{
		dinc = (double)n/(double)_xcc->maxEntry;
		clrs->red = rtable[(int)(dinc * rval)];
		clrs->green = gtable[(int)(dinc * gval)];
		clrs->blue = btable[(int)(dinc * bval)];
		if (XAllocColor(_xcc->dpy, _xcc->colormap, clrs))
		{
			_xcc->CLUT[count++] = clrs->pixel;
			clrs++;
		}
		else
		{
			XFreeColors(_xcc->dpy, _xcc->colormap, _xcc->CLUT, count, 0);

			bval >>= 1;
			gval >>= 1;
			rval >>= 1;

			_xcc->masks.red = (_xcc->masks.red >> 1) & _xcc->visualInfo->red_mask;
			_xcc->masks.green = (_xcc->masks.green >> 1) & _xcc->visualInfo->green_mask;
			_xcc->masks.blue = (_xcc->masks.green >> 1) & _xcc->visualInfo->blue_mask;
			
			_xcc->shifts.red++;
			_xcc->shifts.green++;
			_xcc->shifts.blue++;

			_xcc->bits.red--;
			_xcc->bits.green--;
			_xcc->bits.blue--;

			 _xcc->numColors = (bval + 1) * (gval + 1) * (rval + 1);

			if (_xcc->numColors > 1)
				goto retrydirect;
			else
			{
				XFree((char *)_xcc->CLUT);
				_xcc->CLUT = NULL;
				_initBW(_xcc);
				break;
			}
		}
	}

	XFree(rtable);
	XFree(gtable);
	XFree(btable);
	XFree(cstart);
}

#if NeedFunctionProtoTypes
XCC XCCMonoCreate(Display *_dpy, Visual *_visual, Colormap _colormap)
#else
XCC XCCMonoCreate(_dpy, _visual, _colormap)
Display *_dpy;
Visual *_visual;
Colormap _colormap;
#endif /* NeedFunctionProtoTypes */
{
	XCC xcc;
	XVisualInfo visInfo;
	int n;

	xcc = (XCC)Xmalloc(sizeof(struct _XColorContext));
   
	if (xcc == NULL)
		return NULL;

	xcc->dpy = _dpy;
	xcc->visual = _visual;
	xcc->colormap = _colormap;
	xcc->CLUT = NULL;
	xcc->CMAP = NULL;
	xcc->mode = MODE_UNDEFINED;
	xcc->needToFreeColormap = 0;

	visInfo.visualid = XVisualIDFromVisual(_visual);
	xcc->visualInfo = XGetVisualInfo(_dpy, VisualIDMask, &visInfo, &n);

	_initBW(xcc);

	return xcc;
}


#if NeedFunctionProtoTypes
XCC XCCCreate(Display *_dpy, Visual *_visual, int _usePrivateColormap, int _useStdCmaps, 
							Atom _stdCmapAtom, Colormap *_colormap)
#else
XCC XCCCreate(_dpy, _visual, _usePrivateColormap, _useStdCmaps, _stdCmapAtom, _colormap)
Display *_dpy;
Visual *_visual;
int _usePrivateColormap;
int _useStdCmaps;
Atom _stdCmapAtom;
Colormap *_colormap;
#endif /* NeedFunctionProtoTypes */
{
	XCC xcc;
	int n;
	XVisualInfo visInfo;
	int status;
	int retryCount;

	xcc = (XCC)Xmalloc(sizeof(struct _XColorContext));

	if (xcc == NULL)
		return NULL;

	xcc->dpy = _dpy;
	xcc->visual = _visual;
	xcc->colormap = DefaultColormap(_dpy, DefaultScreen(_dpy));
	xcc->CLUT = NULL;
	xcc->CMAP = NULL;
	xcc->mode = MODE_UNDEFINED;
	xcc->needToFreeColormap = 0;

	visInfo.visualid = XVisualIDFromVisual(_visual);
	xcc->visualInfo = XGetVisualInfo(_dpy, VisualIDMask, &visInfo, &n);

	status = 0;
	if (!_usePrivateColormap && _useStdCmaps)
		status = _lookForStdCmap(xcc, _stdCmapAtom);

	retryCount = 0;
	while(retryCount < 2)
	{
		if (!status)
		{
			if (_usePrivateColormap || (xcc->visual != DefaultVisual(_dpy, DefaultScreen(_dpy))))
			{
				xcc->colormap = XCreateColormap(_dpy, RootWindow(_dpy, DefaultScreen(_dpy)), 
								xcc->visual, AllocNone);
				xcc->needToFreeColormap = (xcc->colormap != DefaultColormap(_dpy, DefaultScreen(_dpy)));
			
			}
			switch(_visual->class)
			{
				case StaticGray:
				case GrayScale:
					if (xcc->visual->map_entries == 2)
						_initBW(xcc);
					else
						_initGray(xcc);
					break;

				case TrueColor: /* shifts */
					_initTrueColor(xcc);
					break;
			
				case DirectColor: /* shifts & fake CLUT */
					_initDirectColor(xcc);
					break;

				case StaticColor:
				case PseudoColor:
					_initColor(xcc);
					break;
			}
		}
		else
			xcc->colormap = xcc->stdCmap.colormap;

		if ( (xcc->mode == MODE_BW) && (xcc->visualInfo->depth > 1))
		{
			_usePrivateColormap = 1;
			retryCount++;
		}
		else
			break;
	}

	*_colormap = xcc->colormap;

	return xcc;
}

/*
** This doesn't currently free black/white. Hmm...
*/

#if NeedFunctionProtoTypes
void  XCCFree(XCC _xcc)
#else
void XCCFree(_xcc)
XCC _xcc;
#endif /* NeedFunctionProtoTypes */
{
	if (_xcc == NULL)
		return;

	if (_xcc->CLUT != NULL)
	{
		XFreeColors(_xcc->dpy, _xcc->colormap, _xcc->CLUT, _xcc->numColors, 0);
		XFree((char *)_xcc->CLUT);
	}

	if (_xcc->CMAP != NULL)
		XFree((char *)_xcc->CMAP);

	if (_xcc->needToFreeColormap)
		XFreeColormap(_xcc->dpy, _xcc->colormap);

	XFree(_xcc->visualInfo);
	XFree((char *)_xcc);
}

#if NeedFunctionProtoTypes
unsigned long XCCGetPixel(XCC _xcc, unsigned int _red, unsigned int _green, unsigned int _blue)
#else
unsigned long XCCGetPixel(_xcc, _red, _green, _blue)
XCC _xcc;
unsigned int _red;
unsigned int _green;
unsigned int _blue;
#endif /* NeedFunctionProtoTypes */
{
	unsigned long ired, igreen, iblue;
	XColor color;

	switch(_xcc->mode)
	{
		case MODE_BW:
		{
			double value;

			value = (double)_red/65535.0 * 0.3 + (double)_green/65535.0 * 0.59 + (double)_blue/65535.0 * 0.11;
			if (value > 0.5)
				return _xcc->whitePixel;
			else
				return _xcc->blackPixel;
		}
		case MODE_MY_GRAY:
				_red = _red * 0.3 + _green * 0.59 + _blue * 0.1;
				_green = 0;
				_blue = 0;
				/* fall thru */

		case MODE_STDCMAP:
			if ((ired = _red * (_xcc->stdCmap.red_max + 1) / 0xFFFF)
				> _xcc->stdCmap.red_max)
				ired = _xcc->stdCmap.red_max;
			ired *= _xcc->stdCmap.red_mult;

			if ((igreen = _green * (_xcc->stdCmap.green_max + 1) / 0xFFFF)
				> _xcc->stdCmap.green_max)
				igreen = _xcc->stdCmap.green_max;
			igreen *= _xcc->stdCmap.green_mult;

			if ((iblue = _blue * (_xcc->stdCmap.blue_max + 1) / 0xFFFF)
				> _xcc->stdCmap.blue_max)
				iblue = _xcc->stdCmap.blue_max;
			iblue *= _xcc->stdCmap.blue_mult;

			if (_xcc->CLUT != NULL)
				return _xcc->CLUT[_xcc->stdCmap.base_pixel + ired + igreen + iblue];
			else
				return _xcc->stdCmap.base_pixel + ired + igreen + iblue;

		case MODE_TRUE:
			if (_xcc->CLUT == NULL)
			{
				_red >>= 16 - _xcc->bits.red;
				_green >>= 16 - _xcc->bits.green;
				_blue >>= 16 - _xcc->bits.blue;

				ired = (_red << _xcc->shifts.red) & _xcc->masks.red;
				igreen = (_green << _xcc->shifts.green) & _xcc->masks.green;
				iblue = (_blue << _xcc->shifts.blue) & _xcc->masks.blue;
			}
			else
			{
				ired = _xcc->CLUT[(int)((_red * _xcc->maxEntry)/65535)] & _xcc->masks.red;
				igreen = _xcc->CLUT[(int)((_green * _xcc->maxEntry)/65535)] & _xcc->masks.green;
				iblue = _xcc->CLUT[(int)((_blue * _xcc->maxEntry)/65535)] & _xcc->masks.blue;
			}

			return ired | igreen | iblue;

		default:
			color.red = _red;
			color.green = _green;
			color.blue = _blue;

			if (!XAllocColor(_xcc->dpy, _xcc->colormap, &color))
				return 0;
			else
				return color.pixel;
	}
}

#if NeedFunctionProtoTypes
int XCCGetNumColors(XCC _xcc)
#else
int XCCGetNumColors(_xcc)
XCC _xcc;
#endif /* NeedFunctionProtoTypes */
{
	return _xcc->numColors;
}

#if NeedFunctionProtoTypes
Colormap XCCGetColormap(XCC _xcc)
#else
Colormap XCCGetColormap(_xcc)
XCC _xcc;
#endif /* NeedFunctionProtoTypes */
{
	if (_xcc)
		return _xcc->colormap;
	else
		return (Colormap)0;
}

#if NeedFunctionProtoTypes
Visual *XCCGetVisual(XCC _xcc)
#else
Visual *XCCGetVisual(_xcc)
XCC _xcc;
#endif /* NeedFunctionProtoTypes */
{
	if (_xcc)
		return _xcc->visual;
	else
		return (Visual *)NULL;
}

#if NeedFunctionProtoTypes
XVisualInfo *XCCGetVisualInfo(XCC _xcc)
#else
XVisualInfo *XCCGetVisualInfo(_xcc)
XCC _xcc;
#endif /* NeedFunctionProtoTypes */
{
	if (_xcc)
		return _xcc->visualInfo;
	else
		return (XVisualInfo *)NULL;
}

#if NeedFunctionProtoTypes
int XCCQueryColors(XCC _xcc, XColor *_colors, int _numColors)
#else
int XCCQueryColors(_xcc, _colors, _numColors)
XCC _xcc;
XColor *_colors;
int _numColors;
#endif /* NeedFunctionProtoTypes */
{
	int i;
	XColor *tc, *curTC;

	switch(_xcc->mode)
	{
		case MODE_BW:
			for(i=0,tc=_colors;i<_numColors;i++,tc++)
			{
				if (tc->pixel == _xcc->whitePixel)
					tc->red = tc->green = tc->blue = 65535;
				else
					tc->red = tc->green = tc->blue = 0;
			}
			break;

		case MODE_TRUE:
			if (_xcc->CLUT == NULL)
			{
				for(i=0,tc=_colors;i<_numColors;i++,tc++)
				{
					tc->red = ((tc->pixel & _xcc->masks.red) * 65535)/_xcc->masks.red;
					tc->green = ((tc->pixel & _xcc->masks.green) * 65535)/_xcc->masks.green;
					tc->blue = ((tc->pixel & _xcc->masks.blue) * 65535)/_xcc->masks.blue;
				}
			}
			else
				return XQueryColors(_xcc->dpy, _xcc->colormap, _colors, _numColors);
			break;

		case MODE_STDCMAP:
		default:
			if (_xcc->CMAP == NULL)
				return XQueryColors(_xcc->dpy, _xcc->colormap, _colors, _numColors);
			else
			{
				register int first, last, half;
				unsigned long halfPixel;

				for(i=0,tc=_colors;i<_numColors;i++)
				{
					first = 0;
					last = _xcc->numColors-1;
					curTC = tc;

					while(first <= last)
					{
						half = (first+last)/2;
						halfPixel = _xcc->CMAP[half].pixel;
						if (tc->pixel == halfPixel)
						{
							tc->red = _xcc->CMAP[half].red;
							tc->green = _xcc->CMAP[half].green;
							tc->blue = _xcc->CMAP[half].blue;
							tc++;
							first = last+1; /* fake break */
						}
						else
						if (tc->pixel > halfPixel)
							first = half+1;
						else
							last = half-1;
					}
					/* if have to get one, might as well get them all! */
					if (tc == curTC)
						return XQueryColors(_xcc->dpy, _xcc->colormap, _colors, _numColors);
				}

				return 1;
			}
			break;
		}

	return 1;
}

#if NeedFunctionProtoTypes
int XCCQueryColor(XCC _xcc, XColor *_color)
#else
int XCCQueryColor(_xcc, _color)
XCC _xcc;
XColor *_color;
#endif /* NeedFunctionProtoTypes */
{
	return XCCQueryColors(_xcc, _color, 1);
}


#if NeedFunctionProtoTypes
Display *XCCGetDisplay(XCC _xcc)
#else
Display *XCCGetDisplay(_xcc)
XCC _xcc;
#endif /* NeedFunctionProtoTypes */
{
	if (_xcc)
		return _xcc->dpy;
	else
		return (Display *)NULL;
}


