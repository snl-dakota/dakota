#ifdef HAVE_CONFIG_H
#include "motif_config.h"
#endif

extern "C" {
  Widget CreateMenu(Widget parent, char* name);
  Widget CreateMenuBar(Widget parent, char* name);
  Widget CreateMenuChoice(Widget parent, char *name, XtCallbackProc callback,
                          XtPointer client_data);
  Widget CreatePushbutton(Widget parent, char* name, XtCallbackProc callback,
                          XtPointer client_data); 
}
