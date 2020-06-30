/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaSurrogatesGP.hpp"

#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"
#include "SharedSurfpackApproxData.hpp"

// Headers from Surrogates module
#include "GaussianProcess.hpp"
 
using dakota::VectorXd;
using dakota::MatrixXd;

namespace Dakota {


SurrogatesGPApprox::
SurrogatesGPApprox(const ProblemDescDB& problem_db,
		   const SharedApproxData& shared_data,
		   const String& approx_label):
  SurrogatesBaseApprox(problem_db, shared_data, approx_label)
{
  // DTS: Updated default behavior to have no trend (i.e. if trend
  // keyword is absent there is no trend)
  surrogateOpts.sublist("Trend").set("estimate trend", true);
  const String& trend_string =
    problem_db.get_string("model.surrogate.trend_order");
  if (trend_string == "constant")
    surrogateOpts.sublist("Trend").sublist("Options").set("max degree", 0);
  else if (trend_string == "linear")
    surrogateOpts.sublist("Trend").sublist("Options").set("max degree", 1);
  else if (trend_string == "quadratic")
    surrogateOpts.sublist("Trend").sublist("Options").set("max degree", 2);
  else
    surrogateOpts.sublist("Trend").set("estimate trend", false);

  // TODO: Surfpack find_nugget is an integer; likely want bool or
  // different semantics
  Real nugget = problem_db.get_real("model.surrogate.nugget");
  short find_nugget = problem_db.get_short("model.surrogate.find_nugget");
  if (find_nugget > 0) {
    surrogateOpts.sublist("Nugget").set("estimate nugget", true);
    surrogateOpts.sublist("Nugget").set("fixed nugget", 0.0);
  }
  else {
    surrogateOpts.sublist("Nugget").set("estimate nugget", false);
    // defaults to 0.0 if not specified
    surrogateOpts.sublist("Nugget").set("fixed nugget", nugget);
  }

  // Number of optimization restarts
  int num_restarts = problem_db.get_int("model.surrogate.num_restarts");
  surrogateOpts.set("num restarts", num_restarts);

  // validate supported metrics
  std::set<std::string> allowed_metrics =
    { "sum_squared", "mean_squared", "root_mean_squared",
      "sum_abs", "mean_abs", "max_abs",
      "sum_abs_percent", "mean_abs_percent", // APE, MAPE
      "rsquared" };
  SharedSurfpackApproxData* shared_surf_data_rep
    = (SharedSurfpackApproxData*)sharedDataRep;
  shared_surf_data_rep->validate_metrics(allowed_metrics);
}


/// On-the-fly constructor
SurrogatesGPApprox::
SurrogatesGPApprox(const SharedApproxData& shared_data):
  SurrogatesBaseApprox(shared_data) {}

int
SurrogatesGPApprox::min_coefficients() const
{
  // TODO (with @dtseidl): This should be based on minimum points
  // needed to build the trend, when present, or some other reasonable
  // default
  return sharedDataRep->numVars + 1;
}


void
SurrogatesGPApprox::build()
{
  MatrixXd vars, resp;
  convert_surrogate_data(vars, resp);

  // construct the surrogate
  if (!advanced_options_file.empty()) {
    model.reset(new dakota::surrogates::GaussianProcess
	        (vars, resp, advanced_options_file));
  }
  else {
    model.reset(new dakota::surrogates::GaussianProcess
	        (vars, resp, surrogateOpts));
  }
}


} // namespace Dakota
