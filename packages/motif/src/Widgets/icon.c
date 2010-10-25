#ifndef NO_XPM
/*
Version 1.2	Added cvtIconDestructor
Version 1.1	Fixed valuemask, now works for XPM versions 3.0-3.4
Version 1.0	Dec '93

The converter from String to Icon knows about a few preloaded icons:
FATAL, ERROR, WARNING, QUESTION, INFO, emptysquare, filledsquare,
emptydiamond, filleddiamond and none. Other names are assumed to be
names of files in XPM format.

An Icon is a combination of a Pixmap and a mask. The mask is
constructed from the pixels that are marked `none' (i.e.,
`transparent') in the XPM data. The other pixels end up in the icon's
pixmap. The actual colors depend on many factors, but the XPM format
offers a way of defining `dynamic' colors, i.e., colors that are
chosen at run time instead of by the icon's designer. The designer can
assign symbolic names to colors, such as `foreground' or `shadow'. The
application can then replace the symbolic names with actual colors
when the icon is loaded.

The type converter tries to automate this process. When an icon is
loaded, the function looks for symbolic color names that match
resources of the widget into which the icon isloaded. E.g., if the
icon has a symbolic color `mainColor' and the widget has resource of
the same name, the value of the resource will be used as the actual
color for the icon.

In this way, icons can be created that fit the widget, whatever the
colors of that widget.

Good symbolic names to use are `background' (defined in every widget),
`foreground' (defined e.g., in |XfwfLabel|), `topShadowColor' and
`bottomShadowColor' (defined in |XfwfFrame|).

The implementation is as follows: the pixmap for the icon actually has
to be created twice; once to get the list of symbolic colors and again
with the replacement colors. When the XPM data is not preloaded, it is
read from a file. The data is converted to an icon, converted back,
and then again converted to an icon with new colors.

The converter is passed one extra argument: the widget that the icon
is loaded into. It must be in |args[0].addr|.

The table of colors to override is set to a fixed size of 20. This
should be enough for most applications: how many widgets have more
than 20 color resources?

*/

#include <stdio.h>				/* For debugging only */
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>				/* For debugging only */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "Converters.h"
#include "XPM/FATAL.pm"
#include "XPM/ERROR.pm"
#include "XPM/WARNING.pm"
#include "XPM/QUESTION.pm"
#include "XPM/INFO.pm"
#include "XPM/square0.pm"
#include "XPM/square1.pm"
#include "XPM/diamond0.pm"
#include "XPM/diamond1.pm"

#define MAXCOLORSYM  20



/* Here are the quarks for the built-in icons names. */

static XrmQuark filleddiamondq, emptydiamondq, filledsquareq, emptysquareq,
    QUESTIONq, FATALq, ERRORq, WARNINGq, INFOq, noneq;

static Boolean initialized = False;



/* This function initializes the quarks. */

static void init_icon_quarks()
{
    filleddiamondq = XrmPermStringToQuark("filleddiamond");
    emptydiamondq = XrmPermStringToQuark("emptydiamond");
    filledsquareq = XrmPermStringToQuark("filledsquare");
    emptysquareq = XrmPermStringToQuark("emptysquare");
    QUESTIONq = XrmPermStringToQuark("QUESTION");
    FATALq = XrmPermStringToQuark("FATAL");
    ERRORq = XrmPermStringToQuark("ERROR");
    WARNINGq = XrmPermStringToQuark("WARNING");
    INFOq = XrmPermStringToQuark("INFO");
    noneq = XrmPermStringToQuark("none");
    initialized = True;
}


/* The function |file_to_icon| is called by the type converter
|cvtStringToIcon|.  It reads a file in XPM format and prints error
messages in case of failure.  Of the XPM attributes, only the color
symbols are retained. */

static void file_to_icon(dpy, file, icon)
    Display *dpy;
    String file;
    Icon *icon;
{
    Cardinal one = 1;
    int status;
    char *filename;

#ifdef XpmReturnInfos
    icon->attributes.valuemask |= XpmReturnInfos;
#else
    icon->attributes.valuemask |= XpmSize;	/* XPM version 3.3 only */
#endif
    icon->mask = None;

        /*
         * Look for file and file.pm, if that fails, use
         * env. var. FWFPIXMAPS as a search pattern. If there
         * is no such env. var, use XFILESEARCHPATH.
         * (See XtResolvePathname)
         * Substitutions:
         * %N = file
         * %T = "pixmap"
         * %S = ".pm"
         */
    if (! (filename = XtResolvePathname
           (dpy, "pixmap", file, ".pm", "%N:%N%S", NULL, 0, NULL))
        && ! (filename = XtResolvePathname
              (dpy, "pixmap", file, ".pm", getenv("FWFPIXMAPS"),
               NULL, 0, NULL))) {
	XtAppWarningMsg
	    (XtDisplayToApplicationContext(dpy), "cvtStringToIcon",
	     "fileError", "XtToolkitError", "Pixmap file not found: \"%s\"",
	     (XtPointer) &file, &one);
	icon->pixmap = icon->mask = None;
        return;
    }        
    status = XpmReadFileToPixmap(dpy, DefaultRootWindow(dpy), filename,
				 &icon->pixmap, &icon->mask,
				 &icon->attributes);
    switch (status) {
    case XpmOpenFailed:
    case XpmFileInvalid:
    case XpmNoMemory:
	XtAppWarningMsg
	    (XtDisplayToApplicationContext(dpy), "cvtStringToIcon",
	     "fileError", "XtToolkitError", "Failed to create icon \"%s\"",
	     (XtPointer) &file, &one);
	icon->pixmap = icon->mask = None;
	break;
    case XpmColorError:
    case XpmColorFailed:
	XtAppWarningMsg
	    (XtDisplayToApplicationContext(dpy), "cvtStringToIcon",
	     "allocColor", "XtToolkitError",
	     "Could not get (all) colors for image \"%s\"",
	     (XtPointer) &file, &one);
	break;
    case XpmSuccess:
	; /* skip */
    }
    XtFree(filename);
}


/* The function |data_to_icon| is also called by the type converter
|cvtStringToIcon|.  It converts data in XPM format and prints error
messages in case of failure.  Of the XPM attributes, only the color
symbols are retained. */

static void data_to_icon(dpy, data, icon)
    Display *dpy;
    String *data;
    Icon *icon;
{
    int status;

#ifdef XpmReturnInfos
    icon->attributes.valuemask |= XpmReturnInfos;
#else
    icon->attributes.valuemask |= XpmSize;	/* XPM version 3.3 only */
#endif
    icon->mask = None;
    status = XpmCreatePixmapFromData(dpy, DefaultRootWindow(dpy), data,
				     &icon->pixmap, &icon->mask,
				     &icon->attributes);
    switch (status) {
    case XpmOpenFailed:
    case XpmFileInvalid:
    case XpmNoMemory:
	XtAppWarningMsg
	    (XtDisplayToApplicationContext(dpy), "cvtStringToIcon",
	     "fileError", "XtToolkitError", "Failed to create an icon",
	     NULL, NULL);
	icon->pixmap = icon->mask = None;
	break;
    case XpmColorError:
    case XpmColorFailed:
	XtAppWarningMsg
	    (XtDisplayToApplicationContext(dpy), "cvtStringToIcon",
	     "allocColor", "XtToolkitError",
	     "Could not get (all) colors for some icon", NULL, NULL);
	break;
    case XpmSuccess:
	; /* skip */
    }
}


/* The function |build_colortable| is also called from
|cvtStringToIcon|. It looks through all the resources for resources that
specify a color (|Pixel|). All such resources and their values are
entered in the table.

To get at the resource value, the |resource_offset| (an unsigned int)
must be added to the base address of the widget. The widget pointer is
first converted to an unsigned long, tehn the offset is added to it and
the result is converted back to a pointer, in this case a pointer to a
|Pixel|. */

static void build_colortable(self, table, size, n)
    Widget self;
    XpmColorSymbol *table;
    Cardinal size;
    Cardinal *n;
{
    Cardinal nres, i;
    XtResourceList res;

    /* fprintf(stderr, "build_colortable for %s\n", XtName(self)); */
    *n = 0;
    XtGetResourceList(XtClass(self), &res, &nres);
    for (i = 0; i < nres; i++)
	if (strcmp(res[i].resource_type, XtRPixel) == 0 && *n < size) {
	    table[*n].name = res[i].resource_name;
	    table[*n].value = NULL;
	    table[*n].pixel =
		* (Pixel*) ((unsigned long) self + res[i].resource_offset);
	    /*
	      fprintf(stderr, "  %s [%ld]\n", table[*n].name, table[*n].pixel);
	      */
	    (*n)++;
	}
}



/*
 * Actually, the XCC argument is not used, since we can't force the
 * XPM code to allocate colors through XCCGetPixel.
 * We only assert that the colormap in the XCC is the same as the
 * one passed explicitly.
 */
Boolean cvtStringToIcon(dpy, args, num_args, from, to, converter_data)
    Display *dpy;
    XrmValue *args;
    Cardinal *num_args;
    XrmValue *from;
    XrmValue *to;
    XtPointer *converter_data;
{
    static XpmColorSymbol table[MAXCOLORSYM];
    Cardinal n;
    String *data, s = (String) from->addr;
    Widget self = (Widget) args[0].addr;
    Colormap* cmap = (Colormap*) args[1].addr;
    XCC *xcc = (XCC*) args[2].addr;
    Icon *view;
    XrmQuark q;

    if (! initialized) init_icon_quarks();

    if (*num_args != 3)
	XtAppErrorMsg
	    (XtDisplayToApplicationContext(dpy),
	     "cvtStringToIcon", "wrongParameters", "XtToolkitError",
	     "String to Icon conversion needs three arguments: \
a widget, a colormap and an XCC", 
	     (String*) NULL, (Cardinal*) NULL);

    assert(dpy == XtDisplay(self));
    assert(args[1].size == sizeof(Colormap));
    assert(args[2].size == sizeof(XCC));
    assert(! *xcc || (*cmap == XCCGetColormap(*xcc)));

    view = (Icon*) XtMalloc(sizeof(*view));

    /*
     * Build a color translation table
     */
    build_colortable(self, table, XtNumber(table), &n);
    view->attributes.colorsymbols = table;
    view->attributes.numsymbols = n;
    view->attributes.colormap = *cmap;
    view->attributes.valuemask = XpmColorSymbols|XpmColormap;

    /*
     * Create the icon, from built-in data or from file
     */
    q = XrmStringToQuark(s);
    if (q == filleddiamondq) data_to_icon(dpy, diamond1, view);
    else if (q == emptydiamondq) data_to_icon(dpy, diamond0, view);
    else if (q == filledsquareq) data_to_icon(dpy, square1, view);
    else if (q == emptysquareq) data_to_icon(dpy, square0, view);
    else if (q == QUESTIONq) data_to_icon(dpy, QUESTION, view);
    else if (q == FATALq) data_to_icon(dpy, FATAL, view);
    else if (q == ERRORq) data_to_icon(dpy, ERROR, view);
    else if (q == WARNINGq) data_to_icon(dpy, WARNING, view);
    else if (q == INFOq) data_to_icon(dpy, INFO, view);
    else if (q == noneq) {XtFree((String)view); done_bert(Icon*, NULL);}
    else file_to_icon(dpy, s, view);

    done_bert(Icon*, view);
}


void cvtIconDestructor(app, to, converter_data, args, num_args)
    XtAppContext app;
    XrmValue *to;
    XtPointer converter_data;
    XrmValue *args;
    Cardinal *num_args;
{
    Icon* icon = *((Icon **) to->addr);
    Widget self = (Widget) args[0].addr;

    if (*num_args != 3)
	XtAppErrorMsg
	    (XtDisplayToApplicationContext(XtDisplay(self)),
	     "cvtIconDestructor", "wrongParameters", "XtToolkitError",
	     "Icon destructor needs three arguments: \
a widget, a colormap and an XCC", 
	     (String*) NULL, (Cardinal*) NULL);

#if 0
    fprintf(stderr, "cvtIconDesctructor: removing icon for widget %s\n",
	    XtName(self));
#endif

    XFreePixmap(XtDisplay(self), icon->pixmap);
    XFreePixmap(XtDisplay(self), icon->mask);
}

#endif /* NO_XPM */
