/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SCALING_OPTIONS_H
#define DAKOTA_SCALING_OPTIONS_H

#include "dakota_data_types.hpp"

namespace Dakota {

/// indicate type of scaling active for a component (bitwise)
enum {SCALE_NONE = 0, SCALE_VALUE = 1, SCALE_LOG = 2, SCALE_AUTO = 4};

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
  UShortArray cvScaleTypes;
  /// continuous variables scale values
  RealVector  cvScales;
  /// primary response scale types
  UShortArray priScaleTypes;
  /// primary response scale values
  RealVector  priScales;
  /// nonlinear inequality constraint scale  types
  UShortArray nlnIneqScaleTypes;
  /// nonlinear inequality constraint scale values
  RealVector  nlnIneqScales;
  /// nonlinear equality constraint scale types
  UShortArray nlnEqScaleTypes;
  /// nonlinear equality constraint scale values
  RealVector  nlnEqScales;
  /// linear inequality constraint scale types
  UShortArray linIneqScaleTypes;
  /// linear inequality constraint scale values
  RealVector  linIneqScales;
  /// linear equality constraint scale types
  UShortArray linEqScaleTypes;
  /// linear equality constraint scale values
  RealVector  linEqScales;               

private:

  /// when values are given, but not types, initialize type to value
  static void default_scale_types(const RealVector& scale_values,
				  UShortArray& scale_types);

  /// convert problem DB strings to unsigned shorts
  static UShortArray scale_str2enum(const StringArray& scale_strs);


};

} // namespace Dakota

#endif // include guard
