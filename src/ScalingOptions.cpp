/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ScalingOptions.hpp"

#include "ProblemDescDB.hpp"
#include "SharedResponseData.hpp"

namespace Dakota {

ScalingOptions::ScalingOptions(const ProblemDescDB& pdb,
			       const SharedResponseData& srd):
  cvScaleTypes(pdb.get_sa("variables.continuous_design.scale_types")),
  cvScales(pdb.get_rv("variables.continuous_design.scales")),
  nlnIneqScaleTypes(pdb.get_sa("responses.nonlinear_inequality_scale_types")),
  nlnIneqScales(pdb.get_rv("responses.nonlinear_inequality_scales")),
  nlnEqScaleTypes(pdb.get_sa("responses.nonlinear_equality_scale_types")),
  nlnEqScales(pdb.get_rv("responses.nonlinear_equality_scales")),
  linIneqScaleTypes(pdb.get_sa("variables.linear_inequality_scale_types")),
  linIneqScales(pdb.get_rv("variables.linear_inequality_scales")),
  linEqScaleTypes(pdb.get_sa("variables.linear_equality_scale_types")),
  linEqScales(pdb.get_rv("variables.linear_equality_scales"))
{
  // For downstream code, populate a single "value" if needed
  default_scale_types(cvScales, cvScaleTypes);
  default_scale_types(nlnIneqScales, nlnIneqScaleTypes);
  default_scale_types(nlnEqScales, nlnEqScaleTypes);
  default_scale_types(linIneqScales, linIneqScaleTypes);
  default_scale_types(linEqScales, linEqScaleTypes);

  // TODO: relax overly conservative expansion of primary weights, scales, sense

  StringArray pri_st = pdb.get_sa("responses.primary_response_fn_scale_types");
  const RealVector& pri_s = pdb.get_rv("responses.primary_response_fn_scales");

  default_scale_types(pri_s, pri_st);

  // TODO: should only allow 1 or num_groups
  expand_for_fields_stl(srd, pri_st, "primary_scale_types", priScaleTypes);
  // allow 1, num_groups, num_elements
  expand_for_fields_sdv(srd, pri_s, "primary_scales", priScales);
}


void ScalingOptions::default_scale_types(const RealVector& scale_values,
					 StringArray& scale_types)
{
  if (scale_types.empty() && scale_values.length() > 0)
    scale_types.push_back("value");
}


} // namespace Dakota
