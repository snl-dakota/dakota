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
  
  // continuous variables scales
  StringArray cvScaleTypes;
  RealVector  cvScales;
  // primary response scales
  StringArray priScaleTypes;
  RealVector  priScales;
  // nonlinear constraint scales
  StringArray nlnIneqScaleTypes;
  RealVector  nlnIneqScales;
  StringArray nlnEqScaleTypes;
  RealVector  nlnEqScales;
  // linear constraint scales
  StringArray linIneqScaleTypes;
  RealVector  linIneqScales;
  StringArray linEqScaleTypes;
  RealVector  linEqScales;               

private:

  static void default_scale_types(const RealVector& scale_values,
				  StringArray& scale_types);

};

} // namespace Dakota

#endif // include guard
