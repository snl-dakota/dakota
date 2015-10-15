/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Graphics
//- Description: This class encapsulates 2D (motif) and 3D (PLPLOT) graphics.
//- Owner:       Brian Dennis, Sandia National Laboratories

#ifndef DAKOTA_GRAPHICS_H
#define DAKOTA_GRAPHICS_H

#include <boost/regex.hpp>
#include "dakota_system_defs.hpp"

#ifdef HAVE_X_GRAPHICS
class Graphics2D;
#endif // HAVE_X_GRAPHICS

namespace Dakota {

class Variables;
class Response;


/// The Graphics class provides a single interface to 2D (motif) and
/// 3D (PLPLOT) graphics; there is only one instance of this
/// OutputManager::dakotaGraphics.
class Graphics
{
public:

  Graphics();  ///< constructor
  ~Graphics(); ///< destructor

  /// creates the 2d graphics window and initializes the plots
  void create_plots_2d(const Variables& vars, const Response& response);

  /// adds data to each window in the 2d graphics based on the results
  /// of a model evaluation
  void add_datapoint(int graphics_cntr,
		     const Variables& vars, const Response& response);

  /// adds data to a single window in the 2d graphics
  void add_datapoint(int i, double x, double y);

  /// creates a separate line graphic for subsequent data points
  /// for a single window in the 2d graphics
  void new_dataset(int i);

  // generate a new 3d plot for F(X,Y)
  //void show_data_3d(const RealVector& X, const RealVector& Y,
  //		      const RealMatrix& F);

  /// close graphics windows
  void close();

  /// set x label for each plot equal to x_label
  void set_x_labels2d(const char* x_label);
  /// set y label for each plot equal to y_label
  void set_y_labels2d(const char* y_label);

  /// set x label for ith plot equal to x_label
  void set_x_label2d(int i, const char* x_label);
  /// set y label for ith plot equal to y_label
  void set_y_label2d(int i, const char* y_label);

private:

#ifdef HAVE_X_GRAPHICS
  Graphics2D* graphics2D; ///< pointer to the 2D graphics object
#endif // HAVE_X_GRAPHICS

  bool win2dOn; ///< flag to indicate if 2D graphics window is active
  //bool win3dOn; // flag to indicate if 3D graphics window is active

};

/// Global utility function to ease migration from CtelRegExp to Boost.Regex
inline std::string re_match(const std::string& token, const boost::regex& re)
{
  std::string str_match;
  boost::smatch found_substr;
  if( boost::regex_search(token, found_substr, re) )
    str_match = std::string(found_substr[0].first, found_substr[0].second);
  return str_match;
}


} // namespace Dakota

#endif
