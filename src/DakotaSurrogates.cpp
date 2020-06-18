/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "DakotaSurrogates.hpp"

#include "ProblemDescDB.hpp"
#include "DakotaVariables.hpp"
#include "DataMethod.hpp"
#include "SharedSurfpackApproxData.hpp"

// Headers from Surrogates module
#include "Surrogate.hpp"
 

using dakota::MatrixXd;


namespace Dakota {


SurrogatesBaseApprox::
SurrogatesBaseApprox(const ProblemDescDB& problem_db,
		     const SharedApproxData& shared_data,
		     const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label)
{
  advanced_options_file = problem_db.get_string("model.advanced_options_file");
}


SurrogatesBaseApprox::
SurrogatesBaseApprox(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data)
{ }


bool SurrogatesBaseApprox::diagnostics_available()
{ return true; }


Real SurrogatesBaseApprox::diagnostic(const String& metric_type)
{
  // BMA TODO: Check for null in case not yet built?!?
  MatrixXd vars, resp;
  convert_surrogate_data(vars,resp);

  StringArray diag_set(1, metric_type);
  auto metric_vals = model->evaluate_metrics(diag_set, vars, resp);

  Cout << std::setw(20) << diag_set[0] << "  " << metric_vals[0] << '\n';

  return metric_vals[0];
}


void SurrogatesBaseApprox::primary_diagnostics(int fn_index)
{
  // BMA TODO: Check for null in case not yet built?!?
  String func_description = approxLabel.empty() ?
    "function " + std::to_string(fn_index+1) : approxLabel;
  SharedSurfpackApproxData* shared_surf_data_rep
    = (SharedSurfpackApproxData*)sharedDataRep;
  StringArray diag_set = shared_surf_data_rep->diagnosticSet;

  // conditionally print default diagnostics
  if (diag_set.empty() && sharedDataRep->outputLevel > NORMAL_OUTPUT)
    diag_set = {"root_mean_squared", "mean_abs", "rsquared"};

  if (!diag_set.empty()) {

    // making extra copy since may not be cached as Eigen
    MatrixXd vars, resp;
    convert_surrogate_data(vars,resp);

    auto metric_vals = model->evaluate_metrics(diag_set, vars, resp);

    Cout << "\nSurrogate quality metrics at build (training) points for "
	 << func_description << ":\n";
    for (size_t i=0; i<diag_set.size(); ++i)
      Cout << std::setw(20) << diag_set[i] << "  " << metric_vals[i] << '\n';
  }
}


void SurrogatesBaseApprox::
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


dakota::ParameterList& SurrogatesBaseApprox::getSurrogateOpts()
{
  return surrogateOpts;
}


void
SurrogatesBaseApprox::convert_surrogate_data(MatrixXd& vars, MatrixXd& resp)
{
  size_t num_v = sharedDataRep->numVars;
  int num_qoi             = 1; // only using 1 for now

  const Pecos::SurrogateData& approx_data = surrogate_data();
  const Pecos::SDVArray& sdv_array = approx_data.variables_data();
  const Pecos::SDRArray& sdr_array = approx_data.response_data();

  int num_pts = approx_data.points();

  // num_samples x num_features
  vars.resize(num_pts, num_v);
  // num_samples x num_qoi
  resp.resize(num_pts, num_qoi);

  // Need to use Teuchos-to-Eigen converters - RWH
  for (size_t i=0; i<num_pts; ++i)
  {
    const RealVector& c_vars = sdv_array[i].continuous_variables();
    for (size_t j=0; j<num_v; j++){
      vars(i,j) = c_vars[j];
    }
    resp(i,0) = sdr_array[i].response_function();
  }
}


Real SurrogatesBaseApprox::value(const Variables& vars)
{
  return value(vars.continuous_variables());
}


const RealVector& SurrogatesBaseApprox::gradient(const Variables& vars)
{
  return gradient(vars.continuous_variables());
}


Real
SurrogatesBaseApprox::value(const RealVector& c_vars)
{
  if (!model) {
    Cerr << "Error: surface is null in SurrogatesBaseApprox::value()"
	 << std::endl;
    abort_handler(-1);
  }

  const size_t num_evals = 1;
  const size_t num_vars = c_vars.length();
  const size_t num_qoi = 1;

  //Eigen::Map<Eigen::MatrixXd> eval_pts(c_vars.values(), num_evals, num_vars);
  // Need to use Teuchos-to-Eigen converters - RWH
  MatrixXd eval_pts(num_evals, num_vars);
  MatrixXd pred    (num_evals, num_qoi);
  for (size_t j = 0; j < num_vars; j++)
    eval_pts(0,j) = c_vars[j];

  model->value(eval_pts, pred);

  return pred(0,0);
}
    
const RealVector& SurrogatesBaseApprox::gradient(const RealVector& c_vars)
{
  const size_t num_evals = 1;
  const size_t num_vars = c_vars.length();

  // Need to use Teuchos-to-Eigen converters - RWH
  MatrixXd eval_pts(num_evals, num_vars);
  for (size_t j = 0; j < num_vars; j++)
    eval_pts(0,j) = c_vars[j];

  // could avoid the temporary and copy by passing an Eigen view of
  // approxGradient
  const size_t qoi = 0; // only one response for now
  MatrixXd pred_grad(num_evals, num_vars);
  model->gradient(eval_pts, pred_grad, qoi);

  approxGradient.sizeUninitialized(c_vars.length());
  for (size_t j = 0; j < num_vars; j++)
    approxGradient[j] = pred_grad(0,j);

  // BMA TODO: redesign Approximation to not return the class member
  // as its state could be invalidated
  return approxGradient;
}


} // namespace Dakota
