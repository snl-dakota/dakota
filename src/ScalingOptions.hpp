/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SCALING_OPTIONS_H
#define DAKOTA_SCALING_OPTIONS_H

#include "dakota_data_types.hpp"

namespace Dakota {

class ProblemDescDB;
class SharedResponseData;

/// Simple container for user-provided scaling data, possibly expanded
/// by replicates through the models
class ScalingOptions {

public:
  /// default ctor: no scaling specified
  ScalingOptions() { /* empty ctor */ };
  /// standard ctor: scaling from problem DB
  ScalingOptions(const ProblemDescDB& problem_db, const SharedResponseData& srd);
  
  /// continuous variables scale types
  StringArray cvScaleTypes;
  /// continuous variables scale values
  RealVector  cvScales;
  /// primary response scale types
  StringArray priScaleTypes;
  /// primary response scale values
  RealVector  priScales;
  /// nonlinear inequality constraint scale  types
  StringArray nlnIneqScaleTypes;
  /// nonlinear inequality constraint scale values
  RealVector  nlnIneqScales;
  /// nonlinear equality constraint scale types
  StringArray nlnEqScaleTypes;
  /// nonlinear equality constraint scale values
  RealVector  nlnEqScales;
  /// linear inequality constraint scale types
  StringArray linIneqScaleTypes;
  /// linear inequality constraint scale values
  RealVector  linIneqScales;
  /// linear equality constraint scale types
  StringArray linEqScaleTypes;
  /// linear equality constraint scale values
  RealVector  linEqScales;               

private:

  /// when values are given, but not types, initialize type to "value"
  static void default_scale_types(const RealVector& scale_values,
				  StringArray& scale_types);

};

} // namespace Dakota

#endif // include guard
