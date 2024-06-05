/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ApproximationFieldInterface.hpp"

namespace Dakota {


ApproximationFieldInterface::
ApproximationFieldInterface(ProblemDescDB& problem_db, const Variables& am_vars,
		       bool am_cache, const String& am_interface_id,
		       const Response & response):
  ApproximationInterface(problem_db, am_vars, am_cache, am_interface_id,
                       response.function_labels())
{
  // Create field-based function indices and reconstruct the approximations
  size_t num_fns = approximation_fn_indices().size();
  auto & field_lengths = response.field_lengths();
  numFields = field_lengths.length();
  int sum_fields = 0;
  fieldFnIndices.resize(numFields);
  for (int i=0; i<response.field_lengths().length(); ++i) {
    fieldFnIndices[i].resize(field_lengths[i]);
    sum_fields += field_lengths[i];
  }
  numScalars = (int)num_fns - sum_fields;
  Cout << "\nApproximationInterface: num_fns, numFields, numScalars --> " << num_fns << ", "
                                                                          << numFields << ", "
                                                                          << numScalars << std::endl;

  // Reconstruction to allow field-based approximations
  functionSurfaces.clear();
  functionSurfaces.resize(num_fns);
  for (StSIter it=approximation_fn_indices().begin(); it!=approximation_fn_indices().end(); ++it)
    functionSurfaces[*it] = Approximation(problem_db, shared_approximation(),
                                          response.function_labels()[*it]);
}

} // namespace Dakota
