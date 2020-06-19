/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaSurrogatesPoly.hpp"

#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"
#include "SharedSurfpackApproxData.hpp"

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
SurrogatesPolyApprox::
SurrogatesPolyApprox(const SharedApproxData& shared_data):
  SurrogatesBaseApprox(shared_data)
{
}


bool SurrogatesPolyApprox::diagnostics_available()
{ return true; }


Real SurrogatesPolyApprox::diagnostic(const String& metric_type)
{
  // BMA TODO: Check for null in case not yet built?!?

  // BMA TODO: Once DTS finalizes interface for metric at build points
  Real approx_diag = std::numeric_limits<Real>::quiet_NaN();
  // = model->metric(metric_type);

  Cout << std::setw(20) << metric_type << "  " << approx_diag << '\n';

  return approx_diag;
}


void SurrogatesPolyApprox::primary_diagnostics(int fn_index)
{
  String func_description = approxLabel.empty() ?
    "function " + std::to_string(fn_index+1) : approxLabel;
  SharedSurfpackApproxData* shared_surf_data_rep
    = (SharedSurfpackApproxData*)sharedDataRep;
  const StringArray& diag_set = shared_surf_data_rep->diagnosticSet;
  if (diag_set.empty()) {
    // conditionally print default diagnostics
    if (sharedDataRep->outputLevel > NORMAL_OUTPUT) {
      Cout << "\nSurrogate quality metrics at build (training) points for "
	   << func_description << ":\n";
      for (const auto& req_diag : {"root_mean_squared", "mean_abs", "rsquared"})
	diagnostic(req_diag);
    }
  }
  else {
    Cout << "\nSurrogate quality metrics at build (training) points for "
	 << func_description << ":\n";
    for (const auto& req_diag : diag_set)
      diagnostic(req_diag);
  }
}


void SurrogatesPolyApprox::
challenge_diagnostics(int fn_index, const RealMatrix& challenge_points,
                             const RealVector& challenge_responses)
{
  // BMA TODO w/ DTS: possibly map chall pts/responses to Eigen once here

  String func_description = approxLabel.empty() ?
    "function " + std::to_string(fn_index+1) : approxLabel;
  StringArray diag_set =
    ((SharedSurfpackApproxData*)sharedDataRep)->diagnosticSet;
  // conditionally print default diagnostics
  if (diag_set.empty() && sharedDataRep->outputLevel > NORMAL_OUTPUT) {
    diag_set.push_back("root_mean_squared");
    diag_set.push_back("mean_abs");
    diag_set.push_back("rsquared");
  }
  Cout << "\nSurrogate quality metrics at challenge (test) points for "
       << func_description << ":\n";

  for (const auto& metric_type : diag_set) {
    // BMA TODO: Once DTS finalizes interface for metric at build points
    Real approx_diag = std::numeric_limits<Real>::quiet_NaN();
    // = model->metric(metric_type, challenge_points, challenge_reponses);

    Cout << std::setw(20) << metric_type << "  " << approx_diag << '\n';
  }
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
