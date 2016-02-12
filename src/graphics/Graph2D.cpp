/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "Graph2D.hpp"

#include <iostream>
using namespace std;

#include <cstdio>
#include <cstdlib>

//#include <X11/Intrinsic.h>
//#include <X11/StringDefs.h>
//#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/DialogS.h>
#include <Xm/ToggleB.h>
//#include <Xm/TextF.h>
//#include <Xm/MainW.h>
//#include <Xm/PanedW.h>
//#include <Xm/FileSBP.h>
//#include <Xm/FileSB.h>
//#include <Xm/XmP.h>
//#include "RowCol.h"
#include "ThWheel.h"
#include "SciPlot.h"

//#define GRAPHICS_DEBUG


Graph2D::Graph2D(): numEntries(0), graphId(1), xMin(0.), xMax(0.), yMin(0.), 
  yMax(0.), xDrawMin(0.), markersOn(True), axisLabelsOn(False),
  legendOn(False), xLogOn(False), yLogOn(False), xLogAllow(True),
  yLogAllow(True), hWheelIncr(0), vWheelIncr(0)
  //hWheelChange(false), vWheelChange(false)
{
  lineColor[0] = 192; lineColor[1] = lineColor[2] = 0; // red (default)

  // black on off-white
  foreColor[0] = foreColor[1] = foreColor[2] = 0;
  backColor[0] = backColor[1] = backColor[2] = 235;

  // white on black
  //foreColor[0] = foreColor[1] = foreColor[2] = 255;
  //backColor[0] = backColor[1] = backColor[2] = 0;
}


Graph2D::~Graph2D()
{ }


void Graph2D::add_window(Widget& parent)
{
  // called by Graphics2D::create_plots2d()

  Arg args[20];
  XtSetArg(args[0], XmNresizable, True);
  XtSetArg(args[1], XmNwidth,     200);
  // workaround to pass string literal to char* interface
  std::string name_str("plotback");
  plotBack = XmCreateForm(parent, const_cast<char*>(name_str.c_str()), args, 2);
  XtManageChild(plotBack);
  // workaround to pass string literal to char* interface
  std::string text_str("Options");
  XmString xmstr1 = XmStringCreateLtoR(const_cast<char*>(text_str.c_str()), 
				       XmFONTLIST_DEFAULT_TAG);
  vertWheel  = XtVaCreateManagedWidget("wheel_R", xfwfThumbWheelWidgetClass,
				       plotBack, XtNvertical, True,
				       XmNrightAttachment, XmATTACH_POSITION,
				       XmNtopAttachment, XmATTACH_POSITION,
				       XmNtopPosition, 25, XmNrightPosition, 99,
				       XmNresizable, True, XmNrubberPositioning,
				       True, NULL);
  horizWheel = XtVaCreateManagedWidget("wheel_R", xfwfThumbWheelWidgetClass,
                                       plotBack, XtNvertical, False,
                                       XmNtopAttachment,  XmATTACH_POSITION,    
                                       XmNtopPosition, 90, XmNrightAttachment,
                                       XmATTACH_POSITION, XmNrightPosition, 70,
                                       XmNresizable, True, XmNrubberPositioning,
                                       True, NULL);
  optButton  = XtVaCreateManagedWidget("opt_button", xmPushButtonWidgetClass,
                                       plotBack, XtNwidth, 55, XtNheight, 20,
                                       XmNtopAttachment,  XmATTACH_POSITION,    
                                       XmNtopPosition, 90, XmNrightAttachment,
                                       XmATTACH_POSITION, XmNrightPosition, 25,
                                       XmNlabelString, xmstr1,
                                       //XmNrightAttachment,  XmATTACH_WIDGET,
                                       //XmNrightWidget , horizWheel,
                                       XmNresizable, True, XmNrubberPositioning,
                                       False, NULL);
  // Legend and axis labels are currently turned off although they can be
  // activated through plotting options.
  plotWidget = XtVaCreateManagedWidget("plot", sciplotWidgetClass, plotBack,
                                       XtNwidth, 150, XtNheight, 150,
                                       XtNshowTitle, True, XtNshowLegend,
                                       False, XtNshowYLabel, False,
                                       XtNshowXLabel, False, XtNplotTitle,
                                       "Iteration History", XtNxLabel,
				       "Evaluation Number", XtNyLabel,
                                       "Attribute Value", XtNtitleFont,
                                       XtFONT_TIMES|18, XtNlabelFont,
                                       XtFONT_TIMES|14, XtNtitleMargin, 2,
                                       XtNdrawMinorTics, False, XtNdrawMinor,
                                       False, //XtNyLog, True, 
				       XtNdefaultMarkerSize, 3,
				       XtNchartType, XtCARTESIAN, 
				       XmNresizable, True,
				       XmNtopAttachment, XmATTACH_POSITION,
				       XmNleftAttachment, XmATTACH_POSITION,
				       XmNrightAttachment, XmATTACH_WIDGET,
				       XmNrightWidget, vertWheel,
				       XmNbottomAttachment, XmATTACH_WIDGET,
				       XmNbottomWidget, horizWheel,
				       XmNrubberPositioning, True, NULL);

  // List id's 0 and 1
  yMaxListId = SciPlotListCreateDouble(plotWidget, 1, &xDrawMin, &yMax, "max");
  yMinListId = SciPlotListCreateDouble(plotWidget, 1, &xDrawMin, &yMin, "min");

  // List id's range from 2 to current
  double x = 0., y = 0.;
  xyListCurrId = xyListFirstId
    = SciPlotListCreateDouble(plotWidget, 0, &x, &y, "value");

  SciPlotSetYAutoScale(plotWidget);
  SciPlotSetXAutoScale(plotWidget);
  SciPlotUpdate(plotWidget);

  XtAddCallback(horizWheel, XtNscrollCallback, h_wheel_callback,
		(XtPointer)this);
  XtAddCallback(vertWheel,  XtNscrollCallback, v_wheel_callback,
		(XtPointer)this);
  XtAddCallback(optButton, XmNactivateCallback, options_callback,
                (XtPointer)this);
}


void Graph2D::change_colors()
{
  // called by Graphics2D::go()

  // plot line
  lColor = SciPlotAllocRGBColor(plotWidget, lineColor[0], lineColor[1],
				lineColor[2]);
  SciPlotListSetStyle(plotWidget, xyListCurrId, lColor, XtMARKER_NONE, lColor,
                      XtLINE_SOLID);
  // yMin marker
  bColor = SciPlotAllocRGBColor(plotWidget, 0, 0, 255); // blue (max saturation)
  SciPlotListSetStyle(plotWidget, yMinListId, bColor, XtMARKER_UTRIANGLE,
                      bColor, XtLINE_NONE);
  // yMax marker
  rColor = SciPlotAllocRGBColor(plotWidget, 255, 0, 0); // red (max saturation)
  SciPlotListSetStyle(plotWidget, yMaxListId, rColor, XtMARKER_DTRIANGLE,
                      rColor, XtLINE_NONE);
  // foreground color
  int color = SciPlotAllocRGBColor(plotWidget, foreColor[0], foreColor[1],
				   foreColor[2]);
  SciPlotSetForegroundColor(plotWidget, color);
  // background color
  color = SciPlotAllocRGBColor(plotWidget, backColor[0], backColor[1],
                               backColor[2]);
  SciPlotSetBackgroundColor(plotWidget, color); 
}


void Graph2D::add_datapoint(double xx, double yy)
{
  // called by Graphics2D::add_datapoint2d()

  // track min and max of x and y values.  Can't use internal SciPlot values
  // (plot.Min.[x,y] and plot.Max.[x,y]) since SciPlotSet[X,Y]UserScale
  // interferes with them.
  if (numEntries == 0) {
    xMin = xMax = xx;
    yMin = yMax = yy;
  }
  else {
    if (xx > xMax)
      xMax = xx;
    else if (xx < xMin)
      xMin = xx;
    if (yy > yMax)
      yMax = yy;
    else if (yy < yMin)
      yMin = yy;
  }

  // disallow log scale options if y or x values are nonpositive
  if (xx <= 0 && xLogAllow == True) {
    xLogAllow = False;
    xLogOn    = False;
    XtVaSetValues(plotWidget, XtNxLog, xLogOn, NULL);
  }
  if (yy <= 0 && yLogAllow == True) {
    yLogAllow = False;
    yLogOn    = False;
    XtVaSetValues(plotWidget, XtNyLog, yLogOn, NULL);
  }

  // add new data to the SciPlot list
  SciPlotListAddDouble(plotWidget, xyListCurrId, 1, &xx, &yy);
  numEntries++;
}


void Graph2D::draw_plot()
{
  // called by Graphics2D::draw_all() as part of the 2 sec event loop as well
  // as by the wheel callback functions.  Not called by add_datapoint.
  // Therefore, wheel operations results in immediate refresh but new
  // datapoints do not.

  // TO DO: what if add_datapoint called draw_plot and the event loop was
  // eliminated?  Would need to cap maximum frequency of plot refreshing.
  // Might slow down execution of main thread if secondary thread is eliminated.

  // This function must account for changes both in wheel increments _and_
  // in the x/y data pairs.

  if (numEntries > 1 && (hWheelIncr || vWheelIncr)) {

    double x_view_min = xMin, x_view_max = xMax,
           y_view_min = yMin, y_view_max = yMax;

    // NOTE: hWheelIncr/vWheelIncr could be a strictly either-or adjustment
    // (the user is moving only one wheel at a time, resulting in draw_plot
    // invocation by a wheel callback function), except that new data may have
    // been introduced which requires recalculation of scaled ranges.
    if (hWheelIncr) { //(hWheelChange) {
      // compute the new minimum x for the reduced view (window contracts from
      // left, omitting earlier evaluation data).  Maximum reduction is 99.5%.
      int h_wheel_mag = abs(hWheelIncr);
      double factor = (h_wheel_mag < 100) ? (double)h_wheel_mag/100. : .995;
      if (hWheelIncr > 0) // window in towards max x
        x_view_min += factor*(xMax - xMin);
      else // window in towards min x
        x_view_max -= factor*(xMax - xMin);
      // now compute a new y range for this reduced window
      y_view_min = yMax; // initialize min to large number
      y_view_max = yMin; // initialize max to small number
      for (int i=xyListFirstId; i<=xyListCurrId; i++) {
	int num_xy_pairs = 0;
        realpair* xy_pairs = SciPlotListReturnPtr(plotWidget, i, &num_xy_pairs);
	for (int j=0; j<num_xy_pairs; j++) {
	  double x = xy_pairs[j].x;
	  if (x >= x_view_min && x <= x_view_max) { // visible w/i reduced view
	    double y = xy_pairs[j].y;
	    if (y > y_view_max)
	      y_view_max = y;
	    if (y < y_view_min)
	      y_view_min = y;
	  }
	}
      }
      //hWheelChange = false;
    }
    if (vWheelIncr) { //(vWheelChange) {
      // following the y range reduction implied by an x range reduction, the y
      // range may be further reduced by the vertical wheel.  Maximum reduction
      // is 99.5%.
      int v_wheel_mag = abs(vWheelIncr);
      double factor = (v_wheel_mag < 100) ? (double)v_wheel_mag/100. : .995;
      if (vWheelIncr > 0) // window in towards min y
        y_view_max -= factor*(y_view_max - y_view_min);
      else // window in towards max y
        y_view_min += factor*(y_view_max - y_view_min);
      /* No good: the search of xy_pairs below overrides the x range above
      // now compute a new x range for this reduced window
      x_view_min = xMax; // initialize min to large number
      x_view_max = xMin; // initialize max to small number
      for (int i=xyListFirstId; i<=xyListCurrId; i++) {
	int num_xy_pairs = 0;
        realpair* xy_pairs = SciPlotListReturnPtr(plotWidget, i, &num_xy_pairs);
	for (int j=0; i<num_xy_pairs; j++) {
	  double y = xy_pairs[j].y;
	  if (y >= y_view_min && y <= y_view_max) { // visible w/i reduced view
	    double x = xy_pairs[j].x;
	    if (x > x_view_max)
	      x_view_max = x;
	    if (x < x_view_min)
	      x_view_min = x;
	  }
	}
      }
      */
      //vWheelChange = false;
    }

#ifdef GRAPHICS_DEBUG
    cout << "x: (" << x_view_min << ", " << x_view_max << ") y: ("
	 << y_view_min << ", " <<  y_view_max << ")" << endl;
#endif // GRAPHICS_DEBUG
    SciPlotSetXUserScale(plotWidget, x_view_min, x_view_max);
    SciPlotSetYUserScale(plotWidget, y_view_min, y_view_max);
  }
  else {
    SciPlotSetXAutoScale(plotWidget);
    SciPlotSetYAutoScale(plotWidget);
  }

  SciPlotListUpdateDouble(plotWidget, yMinListId, 1, &xDrawMin, &yMin);
  SciPlotListUpdateDouble(plotWidget, yMaxListId, 1, &xDrawMin, &yMax);
  if (hWheelIncr || vWheelIncr || SciPlotQuickUpdateCheck(plotWidget)==True) {
    // full update
    SciPlotPrepareFullUpdate(plotWidget);
    double draw_max;
    SciPlotReturnXAxis(plotWidget, &xDrawMin, &draw_max);
    SciPlotListUpdateDouble(plotWidget, yMinListId, 1, &xDrawMin, &yMin);
    SciPlotListUpdateDouble(plotWidget, yMaxListId, 1, &xDrawMin, &yMax);
  }
  else // quick update
    SciPlotPrepareQuickUpdate(plotWidget);

  SciPlotDrawAll(plotWidget);

  /*
  if (hWheelIncr || vWheelIncr) // quick update not valid
    SciPlotUpdate(plotWidget);  // perform full update
  else if (SciPlotQuickUpdate(plotWidget)) // try quick update
    SciPlotUpdate(plotWidget);  // only perform full update if required
  */
}


void Graph2D::set_options()
{
  // called by okay_callback() to set the marker, axis label, legend, and log
  // scale selections after the user has clicked on the "OK" button.

  // Activate/deactive min/max markers on left side of plot based on the user
  // selection within the Options window.
  if (markersOn == True) { // activate markers by specifying a point style
    SciPlotListSetStyle(plotWidget, yMinListId, bColor, XtMARKER_UTRIANGLE,
                        bColor, XtLINE_NONE);
    SciPlotListSetStyle(plotWidget, yMaxListId, rColor, XtMARKER_DTRIANGLE,
                        rColor, XtLINE_NONE);
  }
  else { // deactivate markers by changing point style to none
    SciPlotListSetStyle(plotWidget, yMinListId, bColor, XtMARKER_NONE,
                        bColor, XtLINE_NONE);
    SciPlotListSetStyle(plotWidget, yMaxListId, rColor, XtMARKER_NONE,
                        rColor, XtLINE_NONE);
  }

  // Activate/deactive x and y axis labels based on the user
  // selection within the Options window.
  XtVaSetValues(plotWidget, XtNshowYLabel, axisLabelsOn, NULL);
  XtVaSetValues(plotWidget, XtNshowXLabel, axisLabelsOn, NULL);

  // Activate/deactive legend for the 3 SciPlot lists based on the user
  // selection within the Options window.
  XtVaSetValues(plotWidget, XtNshowLegend, legendOn, NULL);

  // Activate/deactivate log scales for x/y axes based on the user selection
  // within the Options window.
  XtVaSetValues(plotWidget, XtNxLog, xLogOn, NULL);
  XtVaSetValues(plotWidget, XtNyLog, yLogOn, NULL);

  // full SciPlot update.  Could use SciPlotUpdate() except for need to capture
  // change to xDrawMin due to xLogOn change.
  SciPlotPrepareFullUpdate(plotWidget);
  double draw_max;
  SciPlotReturnXAxis(plotWidget, &xDrawMin, &draw_max);
  SciPlotListUpdateDouble(plotWidget, yMinListId, 1, &xDrawMin, &yMin);
  SciPlotListUpdateDouble(plotWidget, yMaxListId, 1, &xDrawMin, &yMax);
  SciPlotDrawAll(plotWidget);
}


void Graph2D::set_x_label(const char* x_label)
{
  // called by Graphics2D::set_x_label2d()

  XtVaSetValues(plotWidget, XtNxLabel, x_label, NULL);
  //SciPlotUpdate(plotWidget);
}


void Graph2D::set_y_label(const char* y_label)
{
  // called by Graphics2D::set_y_label2d()

  XtVaSetValues(plotWidget, XtNyLabel, y_label, NULL);
  //SciPlotUpdate(plotWidget);
}


void Graph2D::set_title(const char* title)
{
  // called by Graphics2D::set_title2d()

  XtVaSetValues(plotWidget, XtNplotTitle, title, NULL);
  //SciPlotUpdate(plotWidget);
}


void Graph2D::new_dataset()
{
  // called by Graphics2D::new_dataset2d()

  // create another SciPlotList
  double x = 0., y = 0.;
  xyListCurrId = SciPlotListCreateDouble(plotWidget, 0, &x, &y, "value");
  SciPlotListSetStyle(plotWidget, xyListCurrId, lColor, XtMARKER_NONE, lColor,
                      XtLINE_SOLID);
}


/*
void Graph2D::set_x_range(double xmin1, double xmax1)
{
  // called by Graphics2D::set_x_range2d()

  if (xmin1 == 0 && xmax1 == 0)
    SciPlotSetXAutoScale(plotWidget);
  else
    SciPlotSetXUserScale(plotWidget, xmin1, xmax1);
  //SciPlotUpdate(plotWidget);
}


void Graph2D::set_y_range(double ymin1, double ymax1)
{
  // called by Graphics2D::set_y_range2d()

  if (ymin1 == 0 && ymax1 == 0)
    SciPlotSetYAutoScale(plotWidget);
  else
    SciPlotSetYUserScale(plotWidget, ymin1, ymax1);
  //SciPlotUpdate(plotWidget);
}


void Graph2D::set_size(int w, int h)
{
  // called by Graphics2D::set_size2d()

  XtVaSetValues(plotWidget, XtNwidth, w, XtNheight, h, NULL);
  //SciPlotUpdate(plotWidget);
}
*/


void Graph2D::h_wheel_callback(Widget widget, XtPointer client_data,
			       XtPointer call_data)
{
  // callback function for the horizontal wheel under each plot

  // A mouse click/drag on the horizontal wheel increments the hWheelIncr.
  // Range is from -100 to +100 with an initial value of 0.
  Graph2D *graph = (Graph2D *)client_data;
  XtVaGetValues(widget, XtNvalue, &graph->hWheelIncr, NULL);
#ifdef GRAPHICS_DEBUG
  cout << "h_wheel_callback: hWheelIncr = " << graph->hWheelIncr << endl;
#endif // GRAPHICS_DEBUG

  //hWheelChange = true;
  graph->draw_plot();
}


void Graph2D::v_wheel_callback(Widget widget, XtPointer client_data,
			       XtPointer call_data)
{
  // callback function for the vertical wheel next to each plot

  // A mouse click/drag on the vertical wheel increments the vWheelIncr.
  // Range is from -100 to +100 with an initial value of 0.
  Graph2D *graph = (Graph2D *)client_data;
  XtVaGetValues(widget, XtNvalue, &graph->vWheelIncr, NULL);
#ifdef GRAPHICS_DEBUG
  cout << "v_wheel_callback: vWheelIncr = " << graph->vWheelIncr << endl;
#endif // GRAPHICS_DEBUG

  //vWheelChange = true;
  graph->draw_plot();
}


void Graph2D::options_callback(Widget widget, XtPointer client_data, 
                               XtPointer call_data)
{
  // callback function for the "Options" button under each plot

  Graph2D *graph = (Graph2D *)client_data;

  Widget Shell = XtVaCreateWidget ("Options", xmDialogShellWidgetClass,
                                   graph->topLevel,
                                   //XmNwidth, 165, XmNheight, 175, XmNnoResize,
                                   //True, XmNresizePolicy, XmRESIZE_NONE,
                                   NULL, 0);
  Widget rc = XtVaCreateManagedWidget("form", xmFormWidgetClass, Shell,
                                      XmNwidth, 200, XmNheight, 240, NULL);
  Widget Label = XtVaCreateManagedWidget("Graph Options", xmLabelWidgetClass,
                                         rc, XmNleftAttachment, XmATTACH_FORM,
                                         XmNleftOffset, 24, XmNtopAttachment,
                                         XmATTACH_FORM, XmNtopOffset, 8,
                                         XmNnoResize, True, NULL);
  Widget Frame = XtVaCreateManagedWidget("frame1", xmFrameWidgetClass, rc, 
                                         XmNleftAttachment,  XmATTACH_FORM, 
                                         XmNleftOffset,      24, 
                                         XmNrightAttachment, XmATTACH_FORM, 
                                         XmNrightOffset,     24, 
                                         XmNtopAttachment,   XmATTACH_WIDGET, 
                                         XmNtopWidget,       Label, 
                                         //XmNrightAttachment,XmATTACH_POSITION,
                                         //XmNrightPosition,   50,
                                         //XmNrightOffset,     4,  
                                         NULL);
  Widget RowCol = XtVaCreateManagedWidget("RowCol", xmRowColumnWidgetClass,
                                          Frame, NULL);

  // workaround to pass string literal to char* interface
  std::string widget_name;

  // Markers option button
  widget_name = "    Markers";
  graph->markersButton
    = XmCreateToggleButton(RowCol, const_cast<char*>(widget_name.c_str()), 
			   NULL, 0);
  // set the button to selected/checked or unselected/unchecked
  XtVaSetValues(graph->markersButton, XmNset, graph->markersOn, NULL);
  XtManageChild(graph->markersButton);

  // Axis labels option button
  widget_name = "    Axis labels";
  graph->axisLabelsButton
    = XmCreateToggleButton(RowCol, const_cast<char*>(widget_name.c_str()),
			   NULL, 0);
  // set the button to selected/checked or unselected/unchecked
  XtVaSetValues(graph->axisLabelsButton, XmNset, graph->axisLabelsOn, NULL);
  XtManageChild(graph->axisLabelsButton);

  // Legend option button
  widget_name = "    Legend";
  graph->legendButton = 
    XmCreateToggleButton(RowCol, const_cast<char*>(widget_name.c_str()),
			 NULL, 0);
  // set the button to selected/checked or unselected/unchecked
  XtVaSetValues(graph->legendButton, XmNset, graph->legendOn, NULL);
  XtManageChild(graph->legendButton);

  // Y log scale option button
  widget_name = "    Log scale - Y axis";
  graph->yLogButton
    = XmCreateToggleButton(RowCol, const_cast<char*>(widget_name.c_str()),
			   NULL, 0);
  // if y log scale is disallowed, gray out the selection as unavailable
  XtVaSetValues(graph->yLogButton, XmNsensitive, graph->yLogAllow, NULL);
  // set the button to selected/checked or unselected/unchecked
  XtVaSetValues(graph->yLogButton, XmNset, graph->yLogOn, NULL);
  XtManageChild(graph->yLogButton);

  // X log scale option button
  widget_name = "    Log scale - X axis";
  graph->xLogButton
    = XmCreateToggleButton(RowCol,  const_cast<char*>(widget_name.c_str()), NULL, 0);
  // if x log scale is disallowed, gray out the selection as unavailable
  XtVaSetValues(graph->xLogButton, XmNsensitive, graph->xLogAllow, NULL);
  // set the button to selected/checked or unselected/unchecked
  XtVaSetValues(graph->xLogButton, XmNset, graph->xLogOn, NULL);
  XtManageChild(graph->xLogButton);

  // Print, OK, and Cancel buttons
  Widget print_button
    = XtVaCreateManagedWidget("Print", xmPushButtonWidgetClass, RowCol, XmNx,
                              10, XmNy, 70, NULL, 0);
  Widget RowCol2
    = XtVaCreateManagedWidget("RowCol", xmRowColumnWidgetClass, rc,
                              XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget,
                              Frame, NULL);
  Widget okay_button
    = XtVaCreateManagedWidget("  OK  ", xmPushButtonWidgetClass, rc , XmNx, 10,
                              XmNy, 10, XmNbottomAttachment, XmATTACH_FORM,
                              XmNbottomOffset,8, XmNtopAttachment,
                              XmATTACH_WIDGET, XmNtopWidget, Frame,XmNtopOffset,
                              8, XmNleftAttachment, XmATTACH_FORM,
                              XmNleftOffset, 24, NULL, 0);
  Widget cancel_button
    = XtVaCreateManagedWidget("Cancel", xmPushButtonWidgetClass, rc,
                              XmNbottomAttachment, XmATTACH_FORM,
                              XmNbottomOffset, 8, XmNtopAttachment,
                              XmATTACH_WIDGET, XmNtopWidget, Frame,
                              XmNtopOffset, 8, XmNrightAttachment,
                              XmATTACH_FORM, XmNrightOffset, 24, XmNx, 50,
                              XmNy, 10, NULL, 0);

  XtAddCallback(print_button,  XmNactivateCallback, print_callback,  graph);
  XtAddCallback(okay_button,   XmNactivateCallback, okay_callback,   graph);
  XtAddCallback(cancel_button, XmNactivateCallback, cancel_callback, Shell);
  XtManageChild(rc);
  graph->db = Shell;
}


void Graph2D::print_callback(Widget widget, XtPointer client_data, 
                             XtPointer call_data)
{
  // callback function for the "Print" button in the Options window

  Graph2D *graph = (Graph2D *)client_data;

  /* This code attempts to open another window for file selection, but 
     LINUX fails for this creation of a second child with the message:
     "Error: DialogShell widget supports only one RectObj child"

  XtSetMappedWhenManaged(graph->db, False);
  //XtDestroyWidget(graph->db);
  Widget box = XmCreateFileSelectionBox(graph->db, "Box", NULL, 0);
  set_path(box);

  XtManageChild(graph->db);
  XtAddCallback(box, XmNokCallback, print_file_callback, NULL);
  XtAddCallback(box, XmNcancelCallback, cancel_callback, box);
  XtManageChild(box);
  */

  // Use SciPlot facility instead:
  char eps_file[32]; // tag with graph id (window number)
  sprintf(eps_file, "dakota_graphic_%d.eps", graph->graphId);
  SciPlotPSCreate(graph->plotWidget, eps_file);
  cout << "Created EPS file " << eps_file << endl;
}


void Graph2D::okay_callback(Widget widget, XtPointer client_data, 
                            XtPointer call_data)
{
  // callback function for the "OK" button in the Options window

  Graph2D *graph = (Graph2D *)client_data;
  XtVaGetValues(graph->markersButton,    XmNset, &graph->markersOn,    NULL);
  XtVaGetValues(graph->axisLabelsButton, XmNset, &graph->axisLabelsOn, NULL);
  XtVaGetValues(graph->legendButton,     XmNset, &graph->legendOn,     NULL);
  XtVaGetValues(graph->xLogButton,       XmNset, &graph->xLogOn,       NULL);
  XtVaGetValues(graph->yLogButton,       XmNset, &graph->yLogOn,       NULL);
  graph->set_options();
  XtDestroyWidget((Widget)graph->db);
}


void Graph2D::cancel_callback(Widget widget, XtPointer client_data, 
                              XtPointer call_data)
{
  // callback function for the "Cancel" button in the Options window

  XtDestroyWidget((Widget)client_data);
}


/* Inactive code used by previous version of print_callback() 
void Graph2D::print_file_callback(Widget widget, XtPointer client_data, 
                                  XtPointer call_data)
{
  // callback function for secondary file selection window created in
  // print_callback().  Currently inactive.

  //Graph2D *graph = (Graph2D *)client_data;
  XmFileSelectionBoxCallbackStruct *cbp
    = (XmFileSelectionBoxCallbackStruct *)call_data;

  char *filename = XdbXmString2String(cbp->value);
  printf("printing file %s \n", filename);
  XtDestroyWidget(widget);
}


char* Graph2D::XdbXmString2String(XmString xms)
{
  char* s = NULL;

#ifdef LesstifVersion
  if (xms == (XmString)XmUNSPECIFIED)
    return "XmUNSPECIFIED";
#endif
  XmStringGetLtoR(xms, XmFONTLIST_DEFAULT_TAG, &s);

  return (s) ? s : "(null)";
}


void Graph2D::set_path(Widget FileBox)
{
  XmString path, filter;

  path   = XmStringCreateLtoR("./", XmSTRING_DEFAULT_CHARSET);
  filter = XmStringCreateLtoR("*",  XmSTRING_DEFAULT_CHARSET);
  
  XtVaSetValues(FileBox, XmNdirectory, path, XmNpattern, filter, NULL);

  XmStringFree(path);
  XmStringFree(filter);
}
*/
