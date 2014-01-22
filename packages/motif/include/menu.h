#ifdef HAVE_CONFIG_H
#include "motif_config.h"
#endif

extern "C" {
  Widget CreateMenu(Widget parent, const char* name);
  Widget CreateMenuBar(Widget parent, const char* name);
  Widget CreateMenuChoice(Widget parent, const char *name, 
			  XtCallbackProc callback, XtPointer client_data);
  Widget CreatePushbutton(Widget parent, const char* name, 
			  XtCallbackProc callback, XtPointer client_data); 
}
