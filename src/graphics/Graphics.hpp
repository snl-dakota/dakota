/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef GraphicsGUI_H
#define GraphicsGUI_H

#include "Graph2D.hpp"
// This include file is needed for Red Hat Linux 6.2 on a PC
//#include <thread_db.h>


class Graphics2D
{

public:

  Graphics2D();
  ~Graphics2D();

  void create_plots2d(int num_2dplots);
  void add_datapoint2d(int i, double x, double y);

  void change_line_color2d(int i, int r, int g, int b);
  void change_foreground_color2d(int i, int r, int g, int b);
  void change_background_color2d(int i, int r, int g, int b);

  void set_x_label2d(int i, const char* x_label);
  void set_y_label2d(int i, const char* y_label);
  void set_title2d(int i, const char* title);
  void new_dataset2d(int i);

  //void set_x_range2d(int i, double xmin, double xmax);
  //void set_y_range2d(int i, double ymin, double ymax);
  //void set_size2d(int w, int h);

  void go();
  void thread_wait();

  int num_2d_plots() const;

private:

  // Static member functions passed by pointer into graphics routines
  static void  draw_all(Graph2D *graph);

  static void* run_thread(void* );

  static void  choice_callback(Widget widget, XtPointer client_data, 
                               XtPointer call_data);

  static XtAppContext appCon;
  static int threadExitVal;
  // workaround for const correctness with string literals
  //  static String fallbackResources[];
  static const char* fallbackResources[];

  Widget mainWindow, menuBar, pullDown1, workArea, topLevel; //pullDown2
  pthread_t threadId;

  int num2dPlots;
  Graph2D *my2dPlots;
};


//inline pthread_t Graphics2D::get_thread_id()
//{ return threadId; }


inline void Graphics2D::change_line_color2d(int i, int r, int g, int b)
{ my2dPlots[i].set_line_color(r, g, b); }


inline void Graphics2D::change_foreground_color2d(int i, int r, int g, int b)
{ my2dPlots[i].set_foreground_color(r, g, b); }


inline void Graphics2D::change_background_color2d(int i, int r, int g, int b)
{ my2dPlots[i].set_background_color(r, g, b); }


inline void Graphics2D::set_x_label2d(int i, const char* x_label)
{ my2dPlots[i].set_x_label(x_label); }


inline void Graphics2D::set_y_label2d(int i, const char* y_label)
{ my2dPlots[i].set_y_label(y_label); }


inline void Graphics2D::set_title2d(int i, const char* title)
{ my2dPlots[i].set_title(title); }


inline void Graphics2D::new_dataset2d(int i)
{ my2dPlots[i].new_dataset(); }


//inline void Graphics2D::set_x_range2d(int i, double xmin, double xmax)
//{ my2dPlots[i].set_x_range(xmin, xmax); }


//inline void Graphics2D::set_y_range2d(int i, double ymin, double ymax)
//{ my2dPlots[i].set_y_range(ymin, ymax); }


//inline void Graphics2D::set_size2d(int w, int h)
//{
//  for (int i=0; i<num2dPlots; i++)
//    my2dPlots[i].set_size(w, h);
//}


inline int Graphics2D::num_2d_plots() const
{ return num2dPlots; }

#endif
