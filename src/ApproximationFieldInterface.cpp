/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ApproximationFieldInterface.hpp"
#include "DakotaFieldApproximation.hpp"

namespace Dakota {


ApproximationFieldInterface::
ApproximationFieldInterface(ProblemDescDB& problem_db, const Variables& am_vars,
		       bool am_cache, const String& am_interface_id,
		       const Response & response):
  ApproximationInterface(problem_db, am_vars, am_cache, am_interface_id,
                       response.function_labels()),
  activeSurface(-1),
  activeComponent(-1)
{
  // Create field-based function indices and reconstruct the approximations

  numSurfaces = approximation_fn_indices().size();
  auto & field_lengths = response.field_lengths();
  numFields = field_lengths.length();
  int sum_fields = 0;
  fieldFnIndices.resize(numFields);
  for (int i=0; i<response.field_lengths().length(); ++i) {
    fieldFnIndices[i].resize(field_lengths[i]);
    sum_fields += field_lengths[i];
  }
  numScalars = (int)numSurfaces - sum_fields;
  //Cout << "\nApproximationInterface: numSurfaces, numFields, numScalars --> " << numSurfaces << ", "
  //                                                                            << numFields << ", "
  //                                                                            << numScalars << std::endl;
  // Create field and component indices which respect the incoming
  // sorted function ids
  std::vector<size_t> all_indices;
  std::copy(approximation_fn_indices().begin(),
            approximation_fn_indices().end(),
            std::back_inserter(all_indices));
  for (int i=0; i<numScalars; ++i) {
    fn2SurfaceIdMap[i] = all_indices[i];
    fn2SurfaceComponentMap[i] = 0;
  }
  int offset = numScalars;
  for (int i=0; i<numFields; ++i ) {
    std::copy(all_indices.begin()+offset,
              all_indices.begin()+offset+field_lengths[i],
              fieldFnIndices[i].begin());
    offset += field_lengths[i];
  }
  for (int i=0; i<numFields; ++i )
    for (size_t j=0; j<fieldFnIndices[i].size(); ++j ) {
      fn2SurfaceIdMap[fieldFnIndices[i][j]] = numScalars+i;
      fn2SurfaceComponentMap[fieldFnIndices[i][j]] = j;
    }

  const auto & labels = response.function_labels();
  String2DArray field_labels(numFields);
  for (int i=0; i<numFields; ++i)
    for (auto const & fid : fieldFnIndices[i] )
      field_labels[i].push_back(labels[fid]);

  //Cout << "Scalar approx indices:" << std::endl;
  //for (int i=0; i<numScalars; ++i ) {
  //  auto id = *(all_indices.begin()+i);
  //  Cout << id << ", \"" << labels[id] << "\"" << std::endl;
  //}
  //Cout << "Field approx indices:" << std::endl;
  //for (int i=0; i<numFields; ++i ) {
  //  Cout << i << ": ";
  //  for (size_t idx=0; idx<fieldFnIndices[i].size(); ++idx )
  //    Cout << fieldFnIndices[i][idx] << "\"" << field_labels[i][idx] << "\"  ";
  //  Cout << std::endl;
  //}
  //Cout << "Function to Surface Id map:" << std::endl;
  //for (auto const & m:fn2SurfaceIdMap)
  //  Cout << m.first << " --> field: " << m.second 
  //       << ", component: " << fn2SurfaceComponentMap[m.first] << std::endl;


  // Reconstruction to allow field-based approximations
  functionSurfaces.clear();
  functionSurfaces.resize(numScalars+numFields);
  for (int i=0; i<numScalars; ++i)
    functionSurfaces[all_indices[i]] = Approximation(problem_db, shared_approximation(),
                                                     response.function_labels()[all_indices[i]]);
  for (int i=0; i<numFields; ++i)
    functionSurfaces[numScalars+i] = FieldApproximation(problem_db, shared_approximation(),
                                                     field_labels[i]);
}


Approximation&
ApproximationFieldInterface::function_surface(size_t index)
{
  activeSurface = fn2SurfaceIdMap.at(index);
  activeComponent = fn2SurfaceComponentMap.at(index);
  Approximation& approx = functionSurfaces.at(fn2SurfaceIdMap.at(index));
  auto field_approx = std::dynamic_pointer_cast<FieldApproximation>(approx.approx_rep());
  //FieldApproximation* field_approx = dynamic_cast<FieldApproximation*>(&approx);
  if (field_approx )
    field_approx->active_component(activeComponent);
  return functionSurfaces.at(fn2SurfaceIdMap.at(index));
}

const Approximation&
ApproximationFieldInterface::function_surface(size_t index) const 
{
  activeSurface = fn2SurfaceIdMap.at(index);
  activeComponent = fn2SurfaceComponentMap.at(index);
  return functionSurfaces.at(fn2SurfaceIdMap.at(index));
}

} // namespace Dakota
