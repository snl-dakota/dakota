/*
 * cvtStringToColor
 *
 * resource converter from String to Pixel (Color), using the XCC
 * colormap.
 *
 * The converter must have two arguments: the colormap and the XCC pointer.
 *
 * Author: Bert Bos <bert@let.rug.nl>
 * Version: 1.1
 * Date: 13 Sep 1995
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "Converters.h"
#include "XCC.h"
#include <assert.h>


/* cvtStringToColor -- convert a string to closest color with XCC */
Boolean cvtStringToColor(dpy, args, num_args, from, to, converter_data)
    Display *dpy;
    XrmValue *args;
    Cardinal *num_args;
    XrmValue *from;
    XrmValue *to;
    XtPointer *converter_data;
{
    String s = (String) from->addr;
    Colormap* cmap = (Colormap*) args[0].addr;
    XCC *xcc = (XCC*) args[1].addr;
    XColor color;
    Pixel result;

    if (*num_args != 2)
	XtAppErrorMsg
	    (XtDisplayToApplicationContext(dpy),
	     "cvtStringToColor", "wrongParameters", "XtToolkitError",
	     "String to Color conversion needs two arguments", 
	     (String*) NULL, (Cardinal*) NULL);
    assert(args[0].size == sizeof(Colormap));
    assert(args[1].size == sizeof(XCC));
    assert(! *xcc || (*cmap == XCCGetColormap(*xcc)));
    if (strcmp(s, XtDefaultForeground) == 0)
        color.red = color.green = color.blue = 0;
    else if (strcmp(s, XtDefaultBackground) == 0)
        color.red = color.green = color.blue = 65535;
    else if (! XParseColor(dpy, *cmap, s, &color)) {
	XtDisplayStringConversionWarning(dpy, XtRString, "Color");
        done_bert(Pixel, 0);
    }
    if (*xcc)
	result = XCCGetPixel(*xcc, color.red, color.green, color.blue);
    else if (XAllocColor(dpy, *cmap, &color))
	result = color.pixel;
    else
	XtDisplayStringConversionWarning(dpy, XtRString, "Color");
    done_bert(Pixel, result);
}
