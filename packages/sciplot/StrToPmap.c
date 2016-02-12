#ifndef lint
static char rcsid[] = "$ StrToPmap.c <thoth>, what a hack $";
#endif /* lint */

/*

   This resource converter assumes the default visual.

   You can change this by setting the integer variable
   Xfwf_StringToPixmap_AssumeDefaultVisual to 0.  

   This will activate code that exercises a bug in the AsciiText Widget.

   It will likely exercise similar bugs in any widget that uses a
   GetValuesHook to let GetValues requests access members of
   subwidgets because these subwidgets aren't created when the
   converter is called to process app-defaults.

   Widgets with different visuals are rare enough to justify the
   default behavior of assuming the default visual.

*/

#include	<X11/IntrinsicP.h>
#include	<X11/StringDefs.h>
#include	<X11/Xmu/CharSet.h>
#include	<X11/CoreP.h>
#include	<X11/Shell.h>	/* XtNvisual */
#include <sys/param.h>		/* get MAXPATHLEN if possible */
#include <stdlib.h>		/* for malloc definitions */
#include <stdio.h>
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

#include "Converters.h"

#ifdef HAVE_XPM

#include <X11/xpm.h>

#if 0
#define PIXMAPFILEPATHCACHE (cache->string_to_pixmap.pixmapFilePath)
#else
static char	** PIXMAPFILEPATHCACHE = NULL;
#endif


/*
 * split_path_string - split a colon-separated list into its constituent
 * parts; to release, free list[0] and list.
 */
static char **split_path_string (src)
    register char *src;
{
    int nelems = 1;
    register char *dst;
    char **elemlist, **elem;

    /* count the number of elements */
    for (dst = src; *dst; dst++) if (*dst == ':') nelems++;

    /* get memory for everything */
    dst = (char *) malloc (dst - src + 1);
    if (!dst) return NULL;
    elemlist = (char **) calloc ((nelems + 1), sizeof (char *));
    if (!elemlist) {
	free (dst);
	return NULL;
    }

    /* copy to new list and walk up nulling colons and setting list pointers */
    strcpy (dst, src);
    for (elem = elemlist, src = dst; *src; src++) {
	if (*src == ':') {
	    *elem++ = dst;
	    *src = '\0';
	    dst = src + 1;
	}
    }
    *elem = dst;

    return elemlist;
}

#if 0
void _XmuStringToPixmapInitCache (c)
    register XmuCvtCache *c;
{
    c->string_to_pixmap.pixmapFilePath = NULL;
}

void _XmuStringToPixmapFreeCache (c)
    register XmuCvtCache *c;
{
    if (c->string_to_pixmap.pixmapFilePath) {
	if (c->string_to_pixmap.pixmapFilePath[0]) 
	  free (c->string_to_pixmap.pixmapFilePath[0]);
	free ((char *) (c->string_to_pixmap.pixmapFilePath));
    }
}
#endif

/*
 * XmuLocatePixmapFile - read a pixmap file using the normal defaults
 */

int XfwfLocatePixmapFile (screen, colormap, visual, name,
			  rval, mask,
			  name_ret, widthp, heightp,
			  pixels, npixels)
     Screen	*screen;
     Colormap	colormap;
     Visual	*visual;
     char *name;
     Pixmap	*rval, *mask;	/* RETURN */
     char **name_ret;		/* RETURN */
     int *widthp, *heightp;	/* RETURN */
     Pixel	**pixels;	/* RETURN */
     unsigned	*npixels;
{
  Display *dpy = DisplayOfScreen (screen);
  Window root = RootWindowOfScreen (screen);
  Bool try_plain_name = True;
/*  XmuCvtCache *cache = _XmuCCLookupDisplay (dpy); */
  char **file_paths;
  char filename[MAXPATHLEN];
  int i;
  int	status;
  char **split_path_string();


  /*
   * look in cache for pixmap path
   */
  /* if (cache)*/ {
    if (!PIXMAPFILEPATHCACHE) {
      XrmName xrm_name[2];
      XrmClass xrm_class[2];
      XrmRepresentation rep_type;
      XrmValue value;

      xrm_name[0] = XrmStringToName ("pixmapFilePath");
      xrm_name[1] = 0;
      xrm_class[0] = XrmStringToClass ("PixmapFilePath");
      xrm_class[1] = 0;
      if (!XrmGetDatabase(dpy)) {
	(void) XGetDefault (dpy, "", "");
      }
      if (XrmQGetResource (XrmGetDatabase(dpy), xrm_name, xrm_class, 
			   &rep_type, &value) &&
	  rep_type == XrmStringToQuark(XtRString)) {
	PIXMAPFILEPATHCACHE = 
	  split_path_string (value.addr);
      }
    }
    file_paths = PIXMAPFILEPATHCACHE;
  }


  /*
   * Search order:
   *    1.  name if it begins with / or ./
   *    2.  "each prefix in file_paths"/name
   *    3.  PIXMAPDIR/name
   *    4.  name if didn't begin with / or .
   */

#ifndef PIXMAPDIR
#define PIXMAPDIR "/usr/include/X11/pixmaps"
#endif

  for (i = 1; i <= 4; i++) {
    XpmAttributes	xpmattr;
    char *fn = filename;

    switch (i) {
    case 1:
      if (!(name[0] == '/'
	    || name[0] == '.' && ( name[1] == '/'
				  || (name[1]=='.' && name[2]=='/')
				  )))
	continue;
      fn = name;
      try_plain_name = False;
      break;
    case 2:
      if (file_paths && *file_paths) {
	sprintf (filename, "%s/%s", *file_paths, name);
	file_paths++;		/* next path */
	i--;			/* retry case 2 */
	break;
      }
      continue;
    case 3:
      sprintf (filename, "%s/%s", PIXMAPDIR, name);
      break;
    case 4:
      if (!try_plain_name) continue;
      fn = name;
      break;
    }

    xpmattr.visual = visual;
    xpmattr.colormap = colormap;
    xpmattr.valuemask = XpmVisual|XpmColormap| (pixels?XpmReturnPixels:0);
    status = XpmReadFileToPixmap(dpy, root, fn, rval, mask, &xpmattr);
    if (status == XpmSuccess) {
      if (name_ret) *name_ret = XtNewString(fn);
      if (widthp) *widthp = xpmattr.width;
      if (heightp) *heightp = xpmattr.height;
      if (pixels) *pixels = xpmattr.pixels;
      if (npixels) *npixels = xpmattr.npixels;
      return 1;
    } else if (status != XpmOpenFailed) {
      break;
    }
  }
  return 0;
}
#endif

#ifdef HAVE_XPM

typedef struct XfwfCvtStrToPmap_recyclerec {
  Pixmap	pmap;
  Pixel		*pixels;
  unsigned	npixels;
  struct XfwfCvtStrToPmap_recyclerec	*next;
} recyclerec;

static recyclerec *head=0;

#endif

XtConvertArgRec  Xfwf_StringToPixmap_Args[] = {
  { XtWidgetBaseOffset, (XtPointer) XtOffset(Widget, core.screen),
      sizeof(Screen*) },
  { XtWidgetBaseOffset, (XtPointer) XtOffset(Widget, core.colormap),
      sizeof(Colormap) },
  { XtWidgetBaseOffset, (XtPointer) XtOffset(Widget, core.self),
      sizeof(Widget) },
};

int Xfwf_Num_StringToPixmap_Args = XtNumber(Xfwf_StringToPixmap_Args);

int Xfwf_StringToPixmap_AssumeDefaultVisual = 1;

/* write linked list to store Pixel arrays for freeing with destructor */

Boolean XfwfCvtStringToPixmap(dpy, args, num_args, fromVal, toVal,
			      converter_data)
   Display   *dpy;
   XrmValue  *args;
   Cardinal  *num_args;
   XrmValue  *fromVal;
   XrmValue  *toVal;
   XtPointer *converter_data;
{
  Screen	*screen;
  Colormap	colormap;
  Widget	temp;
  Visual	*visual;
  Arg	arg;
  char	*name;

   if (*num_args != Xfwf_Num_StringToPixmap_Args)
       XtAppErrorMsg(XtDisplayToApplicationContext(dpy),
          "cvtStringToPixmap", "wrongParameters", "XfwfToolkitError",
          "String to Pixmap conversion needs screen, colormap, and widget_self arguments",
          (String *)NULL, (Cardinal *)NULL);

  screen = *((Screen* *)args[0].addr);

  colormap = *((Colormap *)args[1].addr);

  if (Xfwf_StringToPixmap_AssumeDefaultVisual) {
    visual = screen->root_visual;
  } else {
    visual = CopyFromParent;
    temp = *((Widget *)args[2].addr);
    XtSetArg(arg, XtNvisual, &visual);
    while (temp && visual == CopyFromParent) {
      XtGetValues(temp, &arg, 1);
      temp = XtParent(temp);
    }
    if (!temp)
      visual = screen->root_visual;
  }

  name = (char*) fromVal->addr;

  if (0==XmuCompareISOLatin1(name, "None")) {
    done_bob(Pixmap, None);
    return 1;
  } else if (0==XmuCompareISOLatin1(name, "ParentRelative")) {
    done_bob(Pixmap, ParentRelative);
    return 1;
  } else if (0==XmuCompareISOLatin1(name, "CopyFromParent")) {
    done_bob(Pixmap, CopyFromParent);
    return 1;
  } else if (0==XmuCompareISOLatin1(name, "XtUnspecifiedPixmap")) {
    done_bob(Pixmap, XtUnspecifiedPixmap);
    return 1;
  } else {
#ifdef HAVE_XPM
    Pixmap	pmap;
    Pixel	*pixels;
    unsigned	npixels;
    recyclerec	*temp;
    if ( ! XfwfLocatePixmapFile(screen, colormap, visual,
				name, &pmap, (Pixmap*)0,
				(char**)0, (int*)0, (int*)0,
				&pixels, &npixels)) {
#endif
      XtStringConversionWarning(name, XtRPixmap);
      return 0;
#ifdef HAVE_XPM
    }

    temp = (recyclerec*)malloc(sizeof(*temp));
    temp->next = head;
    temp->pmap = pmap;
    temp->pixels = pixels;
    temp->npixels = npixels;
    head = temp;

    done_bob(Pixmap, pmap);
    return 1;
#endif
  }
}

void XfwfStringToPixmapDestructor(app, to, converter_data, args, num_args)
      XtAppContext app;
      XrmValue *to;
      XtPointer converter_data;
      XrmValue *args;
      Cardinal *num_args;
{
#ifdef HAVE_XPM
  Screen	*screen = *((Screen* *)args[0].addr);
  Colormap	colormap = *((Colormap *)args[1].addr);
  Display	*dpy = DisplayOfScreen(screen);
  Pixmap	pmap = *(Pixmap*)to->addr;
  recyclerec	**scan;

  for (scan = &head; *scan; scan = &(*scan)->next) {
    if ((*scan)->pmap == pmap)
      break;
  }

  if (!*scan) {
#if 1
    /* ah, it was one of the special cases */
#else
    XtAppWarningMsg
      (app, "Improper Free", "cvtStringToPixmap", "XfwfLibraryError",
       "String to Pixmap destructor called to free a pixmap that wasn't allocated",
       (String*)0, (Cardinal*)0 );
#endif
    return;
  }

  XFreePixmap(dpy, pmap);
  XFreeColors(dpy, colormap, (*scan)->pixels, (*scan)->npixels, 0L);
  free((*scan)->pixels);

  {
    recyclerec	*temp = *scan;
    *scan = temp->next;
    free(temp);
  }

#endif

}

void XfwfInstallStringToPixmapConverter()
{
  XtSetTypeConverter
    (XtRString, XtRPixmap, XfwfCvtStringToPixmap,
     Xfwf_StringToPixmap_Args, Xfwf_Num_StringToPixmap_Args,
     XtCacheByDisplay|XtCacheRefCount, XfwfStringToPixmapDestructor);
}
