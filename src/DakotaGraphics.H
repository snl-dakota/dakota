/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Graphics
//- Description: This class encapsulates 2D (motif) and 3D (PLPLOT) graphics.
//- Owner:       Brian Dennis, Sandia National Laboratories

#ifndef DAKOTA_GRAPHICS_H
#define DAKOTA_GRAPHICS_H

#include "system_defs.h"
#include "data_types.h"

#ifdef HAVE_X_GRAPHICS
class Graphics2D;
#endif // HAVE_X_GRAPHICS

namespace Dakota {

class Variables;
class Response;


/// The Graphics class provides a single interface to 2D (motif)
/// and 3D (PLPLOT) graphics as well as tabular cataloguing of data
/// for post-processing with Matlab, Tecplot, etc. 

/** There is only one Graphics object (dakotaGraphics) and it is
    global (for convenient access from strategies, models, and
    approximations). */

class Graphics
{
public:

  Graphics();  ///< constructor
  ~Graphics(); ///< destructor

  /// creates the 2d graphics window and initializes the plots
  void create_plots_2d(const Variables& vars, const Response& response);

  /// opens the tabular data file stream and prints the headings
  void create_tabular_datastream(const Variables& vars,
				 const Response& response,
				 const String& tabular_data_file);

  /// adds data to each window in the 2d graphics and adds a row to
  /// the tabular data file based on the results of a model evaluation
  void add_datapoint(const Variables& vars, const Response& response);

  /// adds data to a single window in the 2d graphics
  void add_datapoint(int i, double x, double y);

  /// creates a separate line graphic for subsequent data points
  /// for a single window in the 2d graphics
  void new_dataset(int i);

  // generate a new 3d plot for F(X,Y)
  //void show_data_3d(const RealVector& X, const RealVector& Y,
  //		      const RealMatrix& F);

  /// close graphics windows and tabular datastream
  void close();

  /// set x label for each plot equal to x_label
  void set_x_labels2d(const char* x_label);
  /// set y label for each plot equal to y_label
  void set_y_labels2d(const char* y_label);

  /// set x label for ith plot equal to x_label
  void set_x_label2d(int i, const char* x_label);
  /// set y label for ith plot equal to y_label
  void set_y_label2d(int i, const char* y_label);

  /// set graphicsCntr equal to cntr
  void graphics_counter(int cntr);
  /// return graphicsCntr
  int graphics_counter() const;

  /// set tabularCntrLabel equal to label
  void tabular_counter_label(const String& label);

private:

#ifdef HAVE_X_GRAPHICS
  Graphics2D* graphics2D; ///< pointer to the 2D graphics object
#endif // HAVE_X_GRAPHICS

  bool win2dOn; ///< flag to indicate if 2D graphics window is active
  //bool win3dOn; // flag to indicate if 3D graphics window is active
  bool tabularDataFlag; ///< flag to indicate if tabular data stream is active

  /// used for x axis values in 2D graphics and for 1st column in tabular data
  int graphicsCntr;

  /// label for counter used in first line comment w/i the tabular data file
  String tabularCntrLabel;

  /// file stream for tabulation of graphics data within compute_response
  std::ofstream tabularDataFStream;
};


inline void Graphics::graphics_counter(int cntr)
{ graphicsCntr = cntr; }


inline int Graphics::graphics_counter() const
{ return graphicsCntr; }


inline void Graphics::tabular_counter_label(const String& label)
{ tabularCntrLabel = label; }

} // namespace Dakota

#endif
