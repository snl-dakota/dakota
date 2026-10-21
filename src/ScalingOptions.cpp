/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ScalingOptions.hpp"

#include "ProblemDescDB.hpp"
#include "SharedResponseData.hpp"

namespace Dakota {

ScalingOptions::ScalingOptions(const ProblemDescDB& pdb,
			       const SharedResponseData& srd):
  cvScales(pdb.get<const RealVector>("variables.continuous_design.scales")),
  nlnIneqScales(pdb.get<const RealVector>("responses.nonlinear_inequality_scales")),
  nlnEqScales(pdb.get<const RealVector>("responses.nonlinear_equality_scales")),
  linIneqScales(pdb.get<const RealVector>("variables.linear_inequality_scales")),
  linEqScales(pdb.get<const RealVector>("variables.linear_equality_scales"))
{
  cvScaleTypes = scale_str2enum(pdb.get<const StringArray>("variables.continuous_design.scale_types"));
  nlnIneqScaleTypes = scale_str2enum(pdb.get<const StringArray>("responses.nonlinear_inequality_scale_types"));
  nlnEqScaleTypes = scale_str2enum(pdb.get<const StringArray>("responses.nonlinear_equality_scale_types"));
  linIneqScaleTypes = scale_str2enum(pdb.get<const StringArray>("variables.linear_inequality_scale_types"));
  linEqScaleTypes = scale_str2enum(pdb.get<const StringArray>("variables.linear_equality_scale_types"));

  // For downstream code, populate a single SCALE_VALUE if needed
  default_scale_types(cvScales, cvScaleTypes);
  default_scale_types(nlnIneqScales, nlnIneqScaleTypes);
  default_scale_types(nlnEqScales, nlnEqScaleTypes);
  default_scale_types(linIneqScales, linIneqScaleTypes);
  default_scale_types(linEqScales, linEqScaleTypes);

  // TODO: relax overly conservative expansion of primary weights, scales, sense

  UShortArray pri_st = scale_str2enum(pdb.get<const StringArray>("responses.primary_response_fn_scale_types"));
  const RealVector& pri_s = pdb.get<const RealVector>("responses.primary_response_fn_scales");

  default_scale_types(pri_s, pri_st);

  // allow 1 or num_groups
  expand_for_fields_stl(srd, pri_st, "primary_scale_types", false, priScaleTypes);
  // allow 1, num_groups, num_elements
  expand_for_fields_sdv(srd, pri_s, "primary_scales", true, priScales);
}


UShortArray ScalingOptions::scale_str2enum(const StringArray& scale_strs)
{
  static std::map<String, unsigned short> scale_str_enum =
    { {"none",  SCALE_NONE},
      {"value", SCALE_VALUE},
      {"log",   SCALE_LOG},
      {"auto",  SCALE_AUTO} };

  UShortArray scale_enums;
  scale_enums.reserve(scale_strs.size());
  for (const String& scale_str : scale_strs)
    scale_enums.push_back(scale_str_enum[scale_str]);

  return scale_enums;
}


void ScalingOptions::default_scale_types(const RealVector& scale_values,
					 UShortArray& scale_types)
{
  if (scale_types.empty() && scale_values.length() > 0)
    scale_types.push_back(SCALE_VALUE);
}


} // namespace Dakota
