/*
To choose a color that is somewhat darker or lighter than another
color, the function |choose_color| queries the RGB values of a pixel
and multiplies them with a factor. If all goes well, the function
returns |True|. If the chosen color ends up being the same as the
original, the color gray75 is returned instead.
*/

#include <stdio.h>
#include <assert.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <math.h>
#include "Converters.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

Boolean choose_color(self, factor, base, result)
    Widget self;
    double factor;
    Pixel base;
    Pixel *result;
{
    Colormap colormap;
    XColor color, dummy;

    if (XtIsRealized(self))
	colormap = self->core.colormap;
    else
	colormap = DefaultColormapOfScreen(XtScreen(self));
    color.pixel = base;

    XQueryColor(XtDisplay(self), colormap, &color);
    color.red = min(65535, factor * color.red);
    color.green = min(65535, factor * color.green);
    color.blue = min(65535, factor * color.blue);
    if (! XAllocColor(XtDisplay(self), colormap, &color))
	return False;
    if (base == color.pixel
	&& ! XAllocNamedColor(XtDisplay(self), colormap, "gray75",
			      &color, &dummy))
	return False;

    *result = color.pixel;
    return True;

}


void choose_xcc_color(self, xcc, factor, base, result)
    Widget self;
    XCC xcc;
    double factor;
    Pixel base;
    Pixel *result;
{
    Colormap colormap;
    XColor color;
    double invfactor = 1.0/(double) factor;
    double maxval = 65535.0, h = 1.0/maxval;
    int v;

    /* Find RGB values of `base' */
#if 0
    if (XtIsRealized(self))
	colormap = self->core.colormap;
    else
	colormap = DefaultColormapOfScreen(XtScreen(self));
#else
    colormap = XCCGetColormap(xcc);
#endif
    color.pixel = base;
    XQueryColor(XtDisplay(self), colormap, &color);

    /* Find the XCC color corresponding to base */
    base = XCCGetPixel(xcc, color.red, color.green, color.blue);

    /* Use factor as a gamma correction */
    v = maxval * pow((double) color.red/maxval, invfactor) + 0.5;
    color.red = v > 65535 ? 65535 : v;
    v = maxval * pow((double) color.green/maxval, invfactor) + 0.5;
    color.green = v > 65535 ? 65535 : v;
    v = maxval * pow((double) color.blue/maxval, invfactor) + 0.5;
    color.blue = v > 65535 ? 65535 : v;

    *result = XCCGetPixel(xcc, color.red, color.green, color.blue);

    /* If the color is unintentionally the same, take a gray color */
    if (factor != 1.0 && base == *result) {
	if (factor > 1.0)
	    (void) XParseColor(XtDisplay(self), colormap, "gray75", &color);
	else
	    (void) XParseColor(XtDisplay(self), colormap, "gray25", &color);
	*result = XCCGetPixel(xcc, color.red, color.green, color.blue);
    }
}
