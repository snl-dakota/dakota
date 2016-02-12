#ifndef Xfwf_StrToPmap_h_
#define Xfwf_StrToPmap_h_

#include <X11/Intrinsic.h>

int XfwfLocatePixmapFile();

extern XtConvertArgRec  Xfwf_StringToPixmap_Args[];
extern int Xfwf_Num_StringToPixmap_Args;

extern int Xfwf_StringToPixmap_AssumeDefaultVisual;

Boolean XfwfCvtStringToPixmap();

void XfwfInstallStringToPixmapConverter();

#endif
