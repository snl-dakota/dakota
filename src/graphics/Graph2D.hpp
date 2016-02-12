/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef GRAPH2D_H
#define GRAPH2D_H

#include <Xm/Xm.h>


class Graph2D
{

public:

  Graph2D();
  ~Graph2D();

  void add_window(Widget &parent);
  void add_datapoint(double xx, double yy);
  void change_colors();
  void draw_plot();
  void set_options();
  //void set_x_range(double xmin1, double xmax1);
  //void set_y_range(double ymin1, double ymax1);
  void set_x_label(const char* x_label);
  void set_y_label(const char* y_label);
  void set_title(const char* title);
  void new_dataset();
  //void set_size(int w, int h);
  void set_line_color(int r, int g, int b);
  void set_foreground_color(int r, int g, int b);
  void set_background_color(int r, int g, int b);
  void set_top_level(Widget tl);
  void set_id(int id);

private:

  static void v_wheel_callback(Widget widget, XtPointer client_data,
			       XtPointer call_data);
  static void h_wheel_callback(Widget widget, XtPointer client_data,
			       XtPointer call_data);
  static void options_callback(Widget widget, XtPointer client_data, 
			       XtPointer call_data);
  static void print_callback(Widget widget, XtPointer client_data,
			     XtPointer call_data);
  static void okay_callback(Widget widget, XtPointer client_data, 
                            XtPointer call_data);
  static void cancel_callback(Widget widget, XtPointer client_data,
                              XtPointer call_data);

  //static void print_file_callback(Widget widget, XtPointer client_data,
  //                                XtPointer call_data);
  //static void set_path(Widget FileBox);
  //static char* XdbXmString2String(XmString xms);

  int numEntries, xyListFirstId, xyListCurrId, yMinListId, yMaxListId, graphId,
      lColor, rColor, bColor, lineColor[3], foreColor[3], backColor[3];
  Widget plotWidget, topLevel, plotBack, db, horizWheel, vertWheel, optButton,
         markersButton, axisLabelsButton, legendButton, xLogButton, yLogButton;
  double xMin, xMax, yMin, yMax, xDrawMin;
  Boolean markersOn, axisLabelsOn, legendOn, xLogOn, yLogOn, xLogAllow,
          yLogAllow;
  int hWheelIncr, vWheelIncr; // # of wheel mouse clicks/drags over (-100, 100)
  //bool hWheelChange, vWheelChange; // indicates wheel change since last redraw
};


inline void Graph2D::set_line_color(int r, int g, int b)
{ lineColor[0] = r; lineColor[1] = g; lineColor[2] = b; }

inline void Graph2D::set_foreground_color(int r, int g, int b)
{ foreColor[0] = r; foreColor[1] = g; foreColor[2] = b; }

inline void Graph2D::set_background_color(int r, int g, int b)
{ backColor[0] = r; backColor[1] = g; backColor[2] = b; }

inline void Graph2D::set_top_level(Widget tl)
{ topLevel = tl; }

inline void Graph2D::set_id(int id)
{ graphId = id; }

#endif
