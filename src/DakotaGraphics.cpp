/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Graphics
//- Description: Implementation code for the Graphics class
//- Owner:       Brian Dennis, Sandia National Laboratories

#include "DakotaGraphics.hpp"

#ifdef HAVE_X_GRAPHICS
#include "Graphics.hpp"
#endif // HAVE_X_GRAPHICS
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"


namespace Dakota {

Graphics::Graphics():
#ifdef HAVE_X_GRAPHICS
  graphics2D(NULL), 
#endif
  win2dOn(false) //, win3dOn(false)
{ }


// Since the Graphics instance is global, it's destruction occurs after
// main().  Similar to CommonIO within UTILIB, use of system resources in this
// destructor can lead to problems (e.g., hanging of the graphics thread).
// Therefore, Graphics::close() is now called explicitly (from the 
// ParallelLibrary destructor which is invoked at the bottom of main()).
Graphics::~Graphics()
{ }


/** Sets up a single event loop for duration of the dakotaGraphics object,
    continuously adding data to a single window.  There is no reset.
    To start over with a new data set, you need a new object (delete old and
    instantiate new). */
void Graphics::create_plots_2d(const Variables& vars, const Response& response)
{
#ifdef HAVE_X_GRAPHICS
  StringMultiArrayConstView cv_labels  = vars.continuous_variable_labels();
  StringMultiArrayConstView div_labels = vars.discrete_int_variable_labels();
  StringMultiArrayConstView drv_labels = vars.discrete_real_variable_labels();
  const StringArray& fn_labels = response.function_labels();
  int i, num_cv = vars.cv(), num_div = vars.div(), num_drv = vars.drv(),
    num_fns = response.num_functions();

  if (win2dOn) { // graphics already active
    // TO DO: sanity checking for change in number of plot windows
  }
  else { // protect multple calls to create_plots_2d 
    graphics2D = new Graphics2D; // instantiate the graphics object
    graphics2D->create_plots2d(num_fns+num_cv+num_div+num_drv);// set # of plots
  }

  // set title and plot line RGB color codes (see /usr/lib/X11/rgb.txt)
  // for all fns and all vars.
  int num_obj_fns   = 0, num_nln_ineq_con = 0, num_nln_eq_con = 0,
      num_lsq_terms = 0, num_resp_fns     = 0;
  char title[25];
  // Since the user can specify arbitrary response labels, regular
  // expression matching is preferable to using label.contains().
  boost::regex obj_re("obj_fn_[1-9][0-9]*"),lsq_re("least_sq_term_[1-9][0-9]*"),
    resp_re(   "response_fn_[1-9][0-9]*"), ineq_re( "nln_ineq_con_[1-9][0-9]*"),
    eq_re(      "nln_eq_con_[1-9][0-9]*");
  for (i=0; i<num_fns; ++i) {
    const std::string& label = fn_labels[i];
    if (label == "obj_fn") {
      std::strcpy(title, "Objective Fn");
      graphics2D->change_line_color2d(i, 0, 0, 192); // blue
    }
    else if (label == re_match(label, obj_re)) {
      std::sprintf(title, "Objective %d", ++num_obj_fns);
      graphics2D->change_line_color2d(i, 0, 0, 192); // blue
    }
    else if (label == re_match(label, lsq_re)) {
      std::sprintf(title, "Least Square Term %d", ++num_lsq_terms);
      graphics2D->change_line_color2d(i, 0, 0, 192); // blue
    }
    else if (label == re_match(label, resp_re)) {
      std::sprintf(title, "Response Function %d", ++num_resp_fns);
      graphics2D->change_line_color2d(i, 0, 0, 192); // blue
    }
    else if (label == re_match(label, ineq_re)) {
      std::sprintf(title, "Ineq Constraint %d", ++num_nln_ineq_con);
      graphics2D->change_line_color2d(i, 0, 192, 0); // green
    }
    else if (label == re_match(label, eq_re)) {
      std::sprintf(title, "Eq Constraint %d", ++num_nln_eq_con);
      graphics2D->change_line_color2d(i, 0, 192, 0); // green
    }
    else { // user-defined response descriptor
      std::strcpy(title, fn_labels[i].c_str());
      graphics2D->change_line_color2d(i, 0, 0, 192); // blue
    }
    graphics2D->set_y_label2d(i, label.data());
    graphics2D->set_title2d(i, title);
  }
  int num_cdv = 0, num_cuv  = 0, num_csv  = 0, num_ddriv = 0, num_ddsiv = 0,
    num_ddsrv = 0, num_duiv = 0, num_durv = 0, num_dsriv = 0, num_dssiv = 0,
    num_dssrv = 0;
  // Since the user can specify arbitrary variable labels, regular
  // expression matching is preferable to using label.contains().
  boost::regex  cdv_re(  "cdv_[1-9][0-9]*"), ddriv_re("ddriv_[1-9][0-9]*"),
              ddsiv_re("ddsiv_[1-9][0-9]*"), ddsrv_re("ddsrv_[1-9][0-9]*"),
                 uv_re(   "uv_[1-9][0-9]*"),  csv_re(   "csv_[1-9][0-9]*"),
              dsriv_re("dsriv_[1-9][0-9]*"), dssiv_re("dssiv_[1-9][0-9]*"),
              dssrv_re("dssrv_[1-9][0-9]*"); // TO DO
  for(i=0; i<num_cv; ++i) {
    const std::string& label = cv_labels[i];
    if (label == re_match(label, cdv_re))
      std::sprintf(title, "Cont Des Variable %d", ++num_cdv);
    else if (label == re_match(label, uv_re))
      std::sprintf(title, "Cont Unc Variable %d", ++num_cuv);
    else if (label == re_match(label, csv_re))
      std::sprintf(title, "Cont State Variable %d", ++num_csv);
    else
      std::strcpy(title, label.c_str());
    graphics2D->set_title2d(i+num_fns, title);
    graphics2D->set_y_label2d(i+num_fns, label.data());
    graphics2D->change_line_color2d(i+num_fns, 192, 0, 0); // red
  }
  for(i=0; i<num_div; ++i) {
    const std::string& label = div_labels[i];
    if (label == re_match(label, ddriv_re))
      std::sprintf(title, "Disc Range Des Var %d", ++num_ddriv);
    else if (label == re_match(label, ddsiv_re))
      std::sprintf(title, "Disc Set Int Des Var %d", ++num_ddsiv);
    else if (label == re_match(label, uv_re))
      std::sprintf(title, "Disc Int Unc Var %d", ++num_duiv);
    else if (label == re_match(label, dsriv_re))
      std::sprintf(title, "Disc Range State Var %d", ++num_dsriv);
    else if (label == re_match(label, dssiv_re))
      std::sprintf(title, "Disc Set Int State Var %d", ++num_dssiv);
    else
      std::strcpy(title, label.c_str());
    graphics2D->set_title2d(i+num_fns+num_cv, title);
    graphics2D->set_y_label2d(i+num_fns+num_cv, label.data());
    graphics2D->change_line_color2d(i+num_fns+num_cv, 192, 0, 0); // red
  }
  for(i=0; i<num_drv; ++i) {
    const std::string& label = drv_labels[i];
    if (label == re_match(label, ddsrv_re))
      std::sprintf(title, "Disc Set Real Des Var %d", ++num_ddsrv);
    else if (label == re_match(label, uv_re))
      std::sprintf(title, "Disc Real Unc Var %d", ++num_durv);
    else if (label == re_match(label, dssrv_re))
      std::sprintf(title, "Disc Set Real State Var %d", ++num_dssrv);
    else
      std::strcpy(title, label.c_str());
    graphics2D->set_title2d(i+num_fns+num_cv+num_div, title);
    graphics2D->set_y_label2d(i+num_fns+num_cv+num_div, label.data());
    graphics2D->change_line_color2d(i+num_fns+num_cv+num_div, 192, 0, 0); // red
  }

  // spawn thread to create window; only call go() once per object.
  // The go function calls pthread_create, which causes a separate thread to be
  // created which is configured with a 2 sec callback.  So every 2 sec, data
  // from arrays published in add_datapoint2d will be added to the plots.
  if (!win2dOn) {
    graphics2D->go();
    win2dOn = true;
  }
#else
  Cerr << "\nWarning: Dakota not compiled with X Windows support; consider "
       << "removing\n       \"graphics\" keyword from input file." 
       << std::endl;
#endif // HAVE_X_GRAPHICS
}



/** Adds data to each 2d plot and each tabular data column (one for
    each active variable and for each response function).
    graphicsCntr is used for the x axis in the graphics and the first
    column in the tabular data.  */
void Graphics::add_datapoint(int graphics_cntr,
			     const Variables& vars, const Response& response)
{
#ifdef HAVE_X_GRAPHICS
  if (win2dOn) {
    const RealVector& c_vars  = vars.continuous_variables();
    const IntVector&  di_vars = vars.discrete_int_variables();
    const RealVector& dr_vars = vars.discrete_real_variables();
    const ShortArray& asv = response.active_set_request_vector();
    const RealVector& fn_vals = response.function_values();
    int i, num_cv = c_vars.length(), num_div = di_vars.length(),
      num_drv = dr_vars.length(), num_fns = asv.size();
    for (i=0; i<num_fns; ++i) // add to each function graph
      if (asv[i] & 1) // better to skip a value than have a meaningless 0
        graphics2D->add_datapoint2d(i, (double)graphics_cntr, fn_vals[i]);
    for (i=0; i<num_cv; ++i)  // add to each variable graph
      graphics2D->add_datapoint2d(i+num_fns, (double)graphics_cntr, c_vars[i]);
    for (i=0; i<num_div; ++i)  // add to each variable graph
      graphics2D->add_datapoint2d(i+num_fns+num_cv, (double)graphics_cntr, 
				  (double)di_vars[i]);
    for (i=0; i<num_drv; ++i)  // add to each variable graph
      graphics2D->add_datapoint2d(i+num_fns+num_cv+num_div, 
				  (double)graphics_cntr, dr_vars[i]);
  }
#endif // HAVE_X_GRAPHICS
}


/** Adds data to a single 2d plot.  Allows complete flexibility in
    defining other kinds of x-y plotting in the 2D graphics. */
void Graphics::add_datapoint(int i, double x, double y)
{
#ifdef HAVE_X_GRAPHICS
  if (win2dOn)
    graphics2D->add_datapoint2d(i, x, y);
#endif // HAVE_X_GRAPHICS

  // Problem with this is that calls to this function may fill in the table
  // by columns -> data may require caching or a reorganization of the table
  //if (tabularDataFlag)
  //  tabularDataFStream << setw(8) << i << ' ' << x << ' ' << y;
}


/** Used for displaying multiple data sets within the same plot. */
void Graphics::new_dataset(int i)
{
#ifdef HAVE_X_GRAPHICS
  if (win2dOn)
    graphics2D->new_dataset2d(i);
#endif // HAVE_X_GRAPHICS
}


/* 3D plotting clears data set and builds from scratch each time show_data3d
   is called.  This still involves an event loop waiting for a mouse click
   (right button) to continue.  X = 1-D x grid values only and 
   Y = 1-D Y grid values only [X and Y are _not_ (X,Y) pairs].
   F = 2-d grid of values for a single function for all (X,Y) combinations.
void Graphics::
show_data_3d(const RealVector& X, const RealVector& Y,
	     const RealMatrix& F)
{
  int num_axis_pts = X.length();
  if (!num_axis_pts)
    return;

#ifdef HAVE_X_GRAPHICS
  if (!win3dOn) {
    plsdev("xwin");
    plinit();
    win3dOn = true;
  }

  // create a matrix to store the values to be displayed
  int i, j;
  PLFLT **vis_mat = new PLFLT * [num_axis_pts];
  for (i=0; i<num_axis_pts; ++i)
    vis_mat[i] = new PLFLT [num_axis_pts];
  PLFLT *x = new PLFLT [num_axis_pts];
  PLFLT *y = new PLFLT [num_axis_pts];

  // load matrix with points
  PLFLT zhigh = -DBL_MAX, zlow = DBL_MAX; // PLFLT typedef defined in plplot.h
  for (i=0; i<num_axis_pts; ++i) {
    x[i] = X[i];
    y[i] = Y[i];
    for (j=0; j<num_axis_pts; ++j) {  
      vis_mat[i][j] = F(j,i);
      if (vis_mat[i][j] > zhigh)
        zhigh = vis_mat[i][j];
      if (vis_mat[i][j] < zlow)
        zlow = vis_mat[i][j];
    }
  }
  // ensure that a suitable plot range is defined for z
  // (important when plotting z = constant).
  if ( std::fabs(zhigh-zlow) < 1.0e-6 ) {
    zhigh += 1.0e-6;
    zlow  -= 1.0e-6;
  }

  // X and Y are monotonic so their min/max values are at the ends
  PLFLT xl_ = (X[0] < X[num_axis_pts-1]) ? X[0] : X[num_axis_pts-1];
  PLFLT xu_ = (X[0] < X[num_axis_pts-1]) ? X[num_axis_pts-1] : X[0];
  PLFLT yl_ = (Y[0] < Y[num_axis_pts-1]) ? Y[0] : Y[num_axis_pts-1];
  PLFLT yu_ = (Y[0] < Y[num_axis_pts-1]) ? Y[num_axis_pts-1] : Y[0];

  // Now generate the 3D PLPLOT window
  plvsta(); // set to standard device independent viewport
  plwind(-1.0, 1.0, -.80, 1.4); // sets the world coordinates of the view port
  plcol(1); // set the current pen color
  // 45 deg. altitude, 45 deg azimuth
  plw3d(1.0, 1.0, 1.0, xl_, xu_, yl_, yu_, zlow, zhigh, 45, 45);
  plbox3("bnstu", "x axis", 0.0, 0, "bnstu", "y axis", 0.0, 0,
         "bcdmnstuv", "z axis", 0.0, 0); // draw the axes, labels, etc.
  plcol(1);
  static const char *t = "#frApproximation Surface";
  plmtex("t", -1.0, 0.5, 0.5, t); // add a title to the plot
  //plmtex("t", -2.5, 0.5, 0.5, t); // add a title to the plot   
  plcol(2);
  plot3d(x, y, vis_mat, num_axis_pts, num_axis_pts, 3, 0); // make the 3D plot
  pladv(0); // advance to the next page, overwriting existing plot

  // clean up
  for (i=0; i<num_axis_pts; ++i)
    delete [] vis_mat[i];  
  delete [] vis_mat; 
  delete [] x;
  delete [] y;
#endif // HAVE_X_GRAPHICS
}
*/


void Graphics::close()
{
#ifdef HAVE_X_GRAPHICS
  //if (win3dOn)
  //  plend(); // 3D
  if (win2dOn) {
    // hold for user action so graphics don't close too quickly
    Cout << "Exit graphics window to terminate DAKOTA." << std::endl;
    graphics2D->thread_wait(); // wait for user to exit the graphics thread

    delete graphics2D; // 2D
    // reset member data since this Graphics is currently global
    win2dOn = false;
  }
#endif // HAVE_X_GRAPHICS
}


void Graphics::set_x_labels2d(const char* x_label)
{
#ifdef HAVE_X_GRAPHICS
  if (win2dOn) {
    int num_2d_plots = graphics2D->num_2d_plots();
    for (int i=0; i<num_2d_plots; ++i)
      graphics2D->set_x_label2d(i, x_label);
  }
#endif // HAVE_X_GRAPHICS
}


void Graphics::set_y_labels2d(const char* y_label)
{
#ifdef HAVE_X_GRAPHICS
  if (win2dOn) {
    int num_2d_plots = graphics2D->num_2d_plots();
    for (int i=0; i<num_2d_plots; ++i)
      graphics2D->set_y_label2d(i, y_label);
  }
#endif // HAVE_X_GRAPHICS
}


void Graphics::set_x_label2d(int i, const char* x_label)
{
#ifdef HAVE_X_GRAPHICS
  if (win2dOn)
    graphics2D->set_x_label2d(i, x_label);
#endif // HAVE_X_GRAPHICS
}


void Graphics::set_y_label2d(int i, const char* y_label)
{
#ifdef HAVE_X_GRAPHICS
  if (win2dOn)
    graphics2D->set_y_label2d(i, y_label);
#endif // HAVE_X_GRAPHICS
}

} // namespace Dakota
