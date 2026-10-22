/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ScalingOptions.hpp"

#include "IRStore.hpp"
#include "ProblemDescDB.hpp"
#include "SharedResponseData.hpp"

namespace Dakota {

ScalingOptions::ScalingOptions(const ProblemDescDB& pdb,
			       const SharedResponseData& srd)
{
  initialize(
    [&](const String& key) -> RealVector
    { return pdb.get<const RealVector>(key); },
    [&](const String& key) -> StringArray
    { return pdb.get<const StringArray>(key); }, srd);
}


ScalingOptions::ScalingOptions(const IRStore& variables_store,
			       const IRStore& responses_store,
			       const SharedResponseData& srd)
{
  // keys carry their block prefix as on the ProblemDescDB path; strip it to
  // address the block-local IR stores
  auto store_for = [&](const String& key, String& local) -> const IRStore& {
    const size_t dot = key.find('.');
    local = key.substr(dot + 1);
    return (key.compare(0, dot, "variables") == 0) ?
      variables_store : responses_store;
  };
  initialize(
    [&](const String& key) -> RealVector {
      String local; const IRStore& store = store_for(key, local);
      return store.contains(local) ? store.get<RealVector>(local) : RealVector();
    },
    [&](const String& key) -> StringArray {
      String local; const IRStore& store = store_for(key, local);
      return store.contains(local) ? store.get<StringArray>(local) : StringArray();
    }, srd);
}


void ScalingOptions::
initialize(const std::function<RealVector(const String&)>& real_vector,
	   const std::function<StringArray(const String&)>& string_array,
	   const SharedResponseData& srd)
{
  cvScales = real_vector("variables.continuous_design.scales");
  nlnIneqScales = real_vector("responses.nonlinear_inequality_scales");
  nlnEqScales = real_vector("responses.nonlinear_equality_scales");
  linIneqScales = real_vector("variables.linear_inequality_scales");
  linEqScales = real_vector("variables.linear_equality_scales");

  cvScaleTypes = scale_str2enum(string_array("variables.continuous_design.scale_types"));
  nlnIneqScaleTypes = scale_str2enum(string_array("responses.nonlinear_inequality_scale_types"));
  nlnEqScaleTypes = scale_str2enum(string_array("responses.nonlinear_equality_scale_types"));
  linIneqScaleTypes = scale_str2enum(string_array("variables.linear_inequality_scale_types"));
  linEqScaleTypes = scale_str2enum(string_array("variables.linear_equality_scale_types"));

  // For downstream code, populate a single SCALE_VALUE if needed
  default_scale_types(cvScales, cvScaleTypes);
  default_scale_types(nlnIneqScales, nlnIneqScaleTypes);
  default_scale_types(nlnEqScales, nlnEqScaleTypes);
  default_scale_types(linIneqScales, linIneqScaleTypes);
  default_scale_types(linEqScales, linEqScaleTypes);

  // TODO: relax overly conservative expansion of primary weights, scales, sense

  UShortArray pri_st = scale_str2enum(string_array("responses.primary_response_fn_scale_types"));
  const RealVector pri_s = real_vector("responses.primary_response_fn_scales");

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
