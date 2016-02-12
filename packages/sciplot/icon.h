#ifndef _icon_h_
#define	_icon_h_

/* #ifdef HAVE_CONFIG_H
#include "motif_config.h"
#endif */ /* HAVE_CONFIG_H */

#ifdef HAVE_XPM

#include <X11/xpm.h>

Boolean cvtStringToIcon(
#if NeedFunctionPrototypes
    Display *dpy,
    XrmValue *args,
    Cardinal *num_args,
    XrmValue *from,
    XrmValue *to,
    XtPointer *converter_data
#endif
);

void cvtIconDestructor(
#if NeedFunctionPrototypes
    XtAppContext app,
    XrmValue *to,
    XtPointer converter_data,
    XrmValue *args,
    Cardinal *num_args
#endif
);

/* The |Icon| type is a convenient combination of a pixmap, a mask and
the pixmaps's attributes. Not all attributes are stored, only width
and height. */

typedef struct _Icon {
    Pixmap pixmap;
    Pixmap mask;
    XpmAttributes attributes;
} Icon;

#endif /* HAVE_XPM */

#endif
