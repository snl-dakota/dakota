/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "Graphics.hpp"

#include <iostream>
using namespace std;

#include <cstdio>
#include <cstdlib>
#include <cmath>

//#include <X11/Intrinsic.h>
//#include <X11/StringDefs.h>
//#include <X11/Core.h>
//#include <Xm/Xm.h>
//#include <Xm/Label.h>
//#include <Xm/TextF.h>
//#include <Xm/RowColumn.h>
//#include <Xm/Form.h>
//#include <Xm/PushB.h>
//#include <Xm/DialogS.h>
#include <Xm/MainW.h>
//#include <Xm/PanedW.h>
//#include <Xm/BulletinB.h>
//#include <Xm/CascadeBP.h>
#include "RowCol.h"
#include <pthread.h>
#include "menu.h"

//#define GRAPHICS_DEBUG


// initialize statics
XtAppContext Graphics2D::appCon;
int Graphics2D::threadExitVal(1);
// workaround for const correctness with string literals
//String Graphics2D::fallbackResources[] = {
const char* Graphics2D::fallbackResources[] = {
  "Graphics2D*borderWidth: 0", "Graphics2D*background: gray50",
  "Graphics2D*foreground: white", "Graphics2D*tester.plotback.borderWidth: 0",
  "Graphics2D*tester.plotback.opt_button.borderWidth: 0",
  "Graphics2D*tester.plotback.background: gray75",
  "Graphics2D*tester.plotback.plot.background: gray65",
  "Graphics2D*tester.plotback.plot.borderColor: gray55",
  "Graphics2D*tester.plotback.plot.borderWidth: 1",
  "*.XmToggleButton.selectColor:	red", NULL};


Graphics2D::Graphics2D(): num2dPlots(0), my2dPlots(NULL)
{
  //XtSetLanguageProc (NULL, NULL, NULL);

  int argc = 0;
  topLevel = XtAppInitialize(&appCon, "Graphics2D", NULL, 0, &argc, NULL,
                             const_cast<char**>(fallbackResources), NULL, 0);

  Arg args[20];
  XtSetArg(args[0], XmNwidth,  500);
  XtSetArg(args[1], XmNheight, 500);

  mainWindow = XtVaCreateManagedWidget("main_w", xmMainWindowWidgetClass, 
                                       topLevel,
                                       //XmNscrollingPolicy,  XmAUTOMATIC, 
                                       //XmNwidth,500, XmNheight,500,  
                                       NULL);
#ifdef GRAPHICS_DEBUG
  cout << "\nMain window done" << endl;
#endif

  menuBar   = CreateMenuBar(mainWindow, "the menu");
  pullDown1 = CreateMenu(menuBar, "File");
  //pullDown2 = CreateMenu(menuBar, "View");  

#ifdef GRAPHICS_DEBUG
  cout << "Menu bar done" << endl;
#endif

  CreateMenuChoice(pullDown1, "Exit",  choice_callback, (XtPointer)this);

  //CreateMenuChoice(pullDown1, "Open",  choice_callback, (XtPointer)NULL);
  //CreateMenuChoice(pullDown1, "Close", choice_callback, (XtPointer)NULL);
  //XtManageChild(workArea);

  std:string res_name = XtNshrinkToFit;
  XtSetArg(args[0], const_cast<char*>(res_name.c_str()), True);
  res_name = XtNframeWidth;
  XtSetArg(args[1], const_cast<char*>(res_name.c_str()),  10);
  res_name =  XtCOuterOffset;
  XtSetArg(args[2], const_cast<char*>(res_name.c_str()), 10);
  res_name =XtNstoreByRow ;
  XtSetArg(args[3], const_cast<char*>(res_name.c_str()),  True);
  workArea = XtCreateManagedWidget("tester", xfwfRowColWidgetClass, 
                                   //xmRowColumnWidgetClass,
                                   mainWindow, args, 4);

  XmMainWindowSetAreas(mainWindow, menuBar, (Widget)NULL,
                       (Widget)NULL, (Widget)NULL, workArea);

  XtSetMappedWhenManaged(topLevel, False);
  //XtManageChild(mainWindow);
}


Graphics2D::~Graphics2D()
{
  if (my2dPlots)
    delete [] my2dPlots;

  // kill the thread when graphics object goes out of scope  
  // BDS 06/12/2000: this functionality was replaced with pthread_exit and
  // moved to choice_callback()
  //pthread_cancel(threadId);
}


void Graphics2D::create_plots2d(int nplots)
{
  if (num2dPlots) {
    cerr << "Error: create_plots2d() may only be called once" << endl;
    exit(-1);
  }
  num2dPlots = nplots;
  my2dPlots  = new Graph2D[num2dPlots];
  for(int i=0; i<num2dPlots; i++){
    my2dPlots[i].add_window(workArea);
    my2dPlots[i].set_top_level(topLevel);
    my2dPlots[i].set_id(i+1);
  }
  // window is four plots wide until we exceed 16 plots, at which point the 
  // width grows proportional to the sqrt of the number of plots.
  int sqrt_np = (int)ceil(sqrt((double)num2dPlots));
  int num_w   = (sqrt_np > 4) ? sqrt_np : 4; // min width is 4 windows
  int num_h   = (int)ceil((double)num2dPlots/(double)num_w);
  int pixel_w = 25 + 200*num_w, pixel_h = 55 + 195*num_h;
  XtVaSetValues(mainWindow, XmNwidth, pixel_w, XmNheight, pixel_h, NULL);
}


void Graphics2D::add_datapoint2d(int i, double x, double y)
{
  if (i < 0 || i >= num2dPlots) {
    cerr << "Error: " << num2dPlots << " were created; you requested plot # "
         << i << endl;
    exit(-1);
  }
  my2dPlots[i].add_datapoint(x, y);
}


void Graphics2D::go()
{
  XtRealizeWidget(topLevel);

  for (int i=0; i<num2dPlots; i++) {
    // now we can change plot colors
    my2dPlots[i].change_colors();
    // set the initial time-out which calls draw_all to set up the event loop
    // for each individual plot.
    XtAppAddTimeOut(appCon, 2000, (XtTimerCallbackProc)draw_all,
                    (XtPointer) &my2dPlots[i]);
  }
  XtMapWidget(topLevel);     
  XtSetMappedWhenManaged(workArea, False);
  XtMapWidget(workArea);  
  //XtAppMainLoop(appCon); // moved to run_thread
 
  // a separate thread is required to run the graphics
  int err = pthread_create(&threadId, NULL, &run_thread, NULL);
  //XtSetMappedWhenManaged(workArea, False);

#ifdef GRAPHICS_DEBUG
  cout << "pthread_create: err = " << err << " id = " << threadId << endl;
#endif
}


void Graphics2D::draw_all(Graph2D *graph)
{
  // This function defines the event loop for each individual plot.
  // It is called only by XtAppAddTimeOut (calling it elsewhere would
  // cause growth in the number of time-outs).

  // redraw this plot
  graph->draw_plot();

  // The previous time-out is now complete, so the event loop is established
  // by setting another time-out that calls draw_all again.
  XtAppAddTimeOut(appCon, 2000, (XtTimerCallbackProc)draw_all,
                  (XtPointer)graph);
}


void* Graphics2D::run_thread(void*)
{
  XtAppMainLoop(appCon);
  return 0;
}


void Graphics2D::thread_wait()
{
#ifdef GRAPHICS_DEBUG
  cout << "Joining thread in thread_wait()" << endl;
#endif

  // allows calling thread to wait for completion of the graphics
  // thread before proceeding.  The graphics thread terminates via the
  // call to pthread_exit() in choice_callback().

  // thread_exit return value is not currently used, set val_ptr to NULL
  // so that nothing is returned.
  void **val_ptr = NULL;

  // wait for the graphics thread to complete
  int e_val = pthread_join(threadId, val_ptr);
}


void Graphics2D::choice_callback(Widget widget, XtPointer client_data, 
                                 XtPointer call_data)
{
#ifdef GRAPHICS_DEBUG
  cout << "Exiting thread in choice_callback()" << endl;
#endif

  // Exit the graphics thread.  The value passed in pthread_exit() can be
  // retrieved in pthread_join().
  pthread_exit(&threadExitVal);

  //Graphics2D *my_object =(Graphics2D *)client_data;
  //my_object->set_size2d(175,215);
  //XtSetMappedWhenManaged(my_object.workArea,FALSE);
  //XtUnmanageChild(my_object->workArea);
  //XtMapWidget(plotWidget);
  //SciPlotUpdate(plotWidget);
}
