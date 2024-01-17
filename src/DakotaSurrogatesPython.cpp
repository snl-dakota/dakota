/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaSurrogatesPython.hpp"

#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"
#include "SharedSurfpackApproxData.hpp"

// Headers from Surrogates module
#include "SurrogatesPython.hpp"
 
using dakota::MatrixXd;

namespace Dakota {


SurrogatesPythonApprox::
SurrogatesPythonApprox(const ProblemDescDB& problem_db,
		const SharedApproxData& shared_data,
		const String& approx_label):
  SurrogatesBaseApprox(problem_db, shared_data, approx_label)
{
  surrogateOpts.set("max degree",
		    static_cast<int>(problem_db.get_short("model.surrogate.polynomial_order"))
		    );
//  surrogateOpts.set("advanced_options_file",
//		    problem_db.get_string("model.advanced_options_file"));

  // validate supported metrics
  std::set<std::string> allowed_metrics =
    { "sum_squared", "mean_squared", "root_mean_squared",
      "sum_abs", "mean_abs", "max_abs",
      "sum_abs_percent", "mean_abs_percent", // APE, MAPE
      "rsquared" };
  std::shared_ptr<SharedSurfpackApproxData> shared_surf_data_rep =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep);
  shared_surf_data_rep->validate_metrics(allowed_metrics);
}


/// On-the-fly constructor
SurrogatesPythonApprox::
SurrogatesPythonApprox(const SharedApproxData& shared_data):
  SurrogatesBaseApprox(shared_data)
{
}


int
SurrogatesPythonApprox::min_coefficients() const
{
  // TODO (with @dtseidl): This should be based on minimum points
  // needed to build the trend, when present, or some other reasonable
  // default
  return sharedDataRep->numVars + 1;
}

void
SurrogatesPythonApprox::build()
{
  // clear any imported model mapping
  modelIsImported = false;
  std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
    varsMapIndices.clear();

  MatrixXd vars, resp;
  convert_surrogate_data(vars, resp);

  // construct the surrogate
  if (!advanced_options_file.empty()) {
    model.reset(new dakota::surrogates::Python
	        (vars, resp, advanced_options_file));
  }
  else {
    model.reset(new dakota::surrogates::Python
	        (vars, resp, surrogateOpts));
  }
}


} // namespace Dakota
