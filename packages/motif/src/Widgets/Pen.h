#ifndef Xfwf_Pen_h_
#define Xfwf_Pen_h_

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XfwfPen {
  GC	gc;
  XGCValues	xgcv;
  unsigned long	mask;
  unsigned long	*colors;
  unsigned	ncolors;
  int	user_xgcv;
  int	user_gc;
} XfwfPen, *XfwfPenPtr;

/* When the pen is built by the String->Pen resource converter the
   following bits can be set in the mask: */
#define	XfwfPenGCBits	(GCForeground|GCBackground|GCFillStyle|\
			 GCTile|GCStipple)

#define XtRXfwfPenPtr	"XfwfPenPtr"

#ifndef XtCForegroundPen
#define XtCForegroundPen	"ForegroundPen"
#endif

#ifndef XtDefaultForegroundPen
#define XtDefaultForegroundPen	"solid:XtDefaultForeground"
#endif

#ifndef XtCBackgroundPen
#define XtCBackgroundPen	"BackgroundPen"
#endif

#ifndef XtDefaultBackgroundPen
#define XtDefaultBackgroundPen	"solid:XtDefaultBackground"
#endif

#ifndef XtNtopShadowPen
#define XtNtopShadowPen		"topShadowPen"
#endif

#ifndef XtCTopShadowPen
#define XtCTopShadowPen		"TopShadowPen"
#endif

#ifndef XtDefaultTopShadowPen
#define XtDefaultTopShadowPen	"solid:XtDefaultForeground"
#endif

#ifndef XtNbottomShadowPen
#define XtNbottomShadowPen	"bottomShadowPen"
#endif

#ifndef XtCBottomShadowPen
#define XtCBottomShadowPen	"BottomShadowPen"
#endif

#ifndef XtDefaultBottomShadowPen
#define XtDefaultBottomShadowPen	"mono:opaque-stipple:XtDefaultForeground,XtDefaultBackground,XtCheckerBitmap|solid:XtDefaultBackground"
#endif

/* performs a bcmp on the two structs */
int SameXfwfPen(/* XfwfPen*, XfwfPen* */);

/* if the gc field of the pen is 0, then it calls XtGetGC with the
   xgcv and mask to make one */
int ValidateXfwfPen(/* Widget, XfwfPen* */);

/* If the user_gc field is zero it calls XtReleaseGC on the gc.

   If user_xgcv is zero it releases each of the resources specified in
   (mask&XfwfPenGCBits).

   If colors is non-null it calls XFree colors with it and then free(3)s
   the colors pointer.
*/
int FreeXfwfPen(/* XfwfPen* */);

extern XtConvertArgRec	Xfwf_StringToPen_Args[];
extern int Xfwf_Num_StringToPen_Args;

void XfwfInstallStringToPenConverter();

#ifdef __cplusplus
}	/* extern C */
#endif

#endif /* Xfwf_Pen_h_ */
