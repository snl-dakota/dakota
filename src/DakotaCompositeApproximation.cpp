/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "DakotaCompositeApproximation.hpp"

#include "ProblemDescDB.hpp"
#include "DakotaVariables.hpp"


namespace Dakota {


CompositeApproximation::
CompositeApproximation(const ProblemDescDB& problem_db,
                       const SharedApproxData& shared_data,
                       const StringArray& approx_labels):
  FieldApproximation(BaseConstructor(), problem_db, shared_data, approx_labels)
{
}

Real 
CompositeApproximation::value(const Variables& vars)
{
  return approximations[activeComponent]->value(vars);
}

void 
CompositeApproximation::build()
{
  return approximations[activeComponent]->build();
}

} // namespace Dakota
