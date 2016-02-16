/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <stdio.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>


Widget CreatePushbutton(Widget parent, char* name, XtCallbackProc callback,
                        XtPointer client_data)
{		
  Widget button;
  Arg args[20];
  int n = 0;
  button =XmCreatePushButton(parent, name, args, n);

  XtManageChild(button);

  XtAddCallback(button, XmNactivateCallback, callback, client_data);
  return(button);
}


Widget CreateMenu(Widget parent, char* name)
{
  Widget cascade, menuwidget;
  Arg args[20];
  int n = 0;
  menuwidget = XmCreatePulldownMenu(parent, name, args, n);
  n=0;
  XtSetArg(args[n], XmNsubMenuId, menuwidget); n++;
  cascade = XmCreateCascadeButton(parent, name, args, n); 

  XtManageChild(cascade);

  return menuwidget;
}


Widget CreateMenuBar(Widget parent, char* name)
{
  Widget menubar;
  Arg args[20];
  int n = 0;
  menubar = XmCreateMenuBar(parent, name, args, n);
  XtManageChild(menubar);
  return menubar;
}


Widget CreateMenuChoice(Widget parent, char *name, XtCallbackProc callback,
                        XtPointer client_data)
{ return CreatePushbutton(parent, name, callback, client_data); }
