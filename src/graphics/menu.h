/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//#ifdef HAVE_CONFIG_H
//#include "motif_config.h"
//#endif

extern "C" {
  Widget CreateMenu(Widget parent, const char* name);
  Widget CreateMenuBar(Widget parent, const char* name);
  Widget CreateMenuChoice(Widget parent, const char *name, 
			  XtCallbackProc callback, XtPointer client_data);
  Widget CreatePushbutton(Widget parent, const char* name, 
			  XtCallbackProc callback, XtPointer client_data); 
}
