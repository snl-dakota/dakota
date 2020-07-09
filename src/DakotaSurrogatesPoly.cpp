/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaSurrogatesPoly.hpp"

#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"

// Headers from Surrogates module
#include "PolynomialRegression.hpp"
 
using dakota::MatrixXd;

namespace Dakota {


SurrogatesPolyApprox::
SurrogatesPolyApprox(const ProblemDescDB& problem_db,
		const SharedApproxData& shared_data,
		const String& approx_label):
  SurrogatesBaseApprox(problem_db, shared_data, approx_label)
{
  surrogateOpts.set("max degree",
		    static_cast<int>(problem_db.get_short("model.surrogate.polynomial_order"))
		    );
//  surrogateOpts.set("advanced_options_file",
//		    problem_db.get_string("model.advanced_options_file"));
}


/// On-the-fly constructor
SurrogatesPolyApprox::
SurrogatesPolyApprox(const SharedApproxData& shared_data):
  SurrogatesBaseApprox(shared_data)
{
}


int
SurrogatesPolyApprox::min_coefficients() const
{
  // TODO (with @dtseidl): This should be based on minimum points
  // needed to build the trend, when present, or some other reasonable
  // default
  return sharedDataRep->numVars + 1;
}

void
SurrogatesPolyApprox::build()
{
  MatrixXd vars, resp;
  convert_surrogate_data(vars, resp);

  // construct the surrogate
  if (!advanced_options_file.empty()) {
    model.reset(new dakota::surrogates::PolynomialRegression
	        (vars, resp, advanced_options_file));
  }
  else {
    model.reset(new dakota::surrogates::PolynomialRegression
	        (vars, resp, surrogateOpts));
  }
}


} // namespace Dakota
