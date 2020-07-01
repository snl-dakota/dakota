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
#include "SurrogatesBase.hpp"
 

using dakota::MatrixXd;
using dakota::VectorXd;


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


RealArray
SurrogatesBaseApprox::cv_diagnostic(const StringArray& metric_types,
				    unsigned num_folds)
{
  MatrixXd vars, resp;
  convert_surrogate_data(vars,resp);

  VectorXd cv_metrics_eigen =
    model->cross_validate(vars, resp, metric_types, num_folds, 6716);

  return RealArray(cv_metrics_eigen.data(),
		   cv_metrics_eigen.data() + cv_metrics_eigen.size());
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

    if (shared_surf_data_rep->crossValidateFlag) {
      size_t num_folds = shared_surf_data_rep->numFolds;
      RealArray cv_metrics = cv_diagnostic(diag_set, num_folds);
      Cout << "\nSurrogate quality metrics (" << num_folds << "-fold CV) for "
           << func_description << ":\n";
      for (size_t i=0; i<diag_set.size(); ++i)
	Cout << std::setw(20) << diag_set[i] << "  " << cv_metrics[i] << '\n';
    }

    if (shared_surf_data_rep->pressFlag) {
      RealArray cv_metrics = cv_diagnostic(diag_set, vars.rows());
      Cout << "\nSurrogate quality metrics (PRESS/leave-one-out) for "
           << func_description << ":\n";
      for (size_t i=0; i<diag_set.size(); ++i)
	Cout << std::setw(20) << diag_set[i] << "  " << cv_metrics[i] << '\n';
    }

  }
}


void SurrogatesBaseApprox::
challenge_diagnostics(int fn_index, const RealMatrix& challenge_points,
		      const RealVector& challenge_responses)
{
  String func_description = approxLabel.empty() ?
    "function " + std::to_string(fn_index+1) : approxLabel;
  StringArray diag_set =
    ((SharedSurfpackApproxData*)sharedDataRep)->diagnosticSet;

  // conditionally print default diagnostics
  if (diag_set.empty() && sharedDataRep->outputLevel > NORMAL_OUTPUT)
    diag_set = {"root_mean_squared", "mean_abs", "rsquared"};

  if (!diag_set.empty()) {

    // using Eigen Map to avoid reliance on Teuchos adapter
    Eigen::Map<Eigen::MatrixXd> vars(challenge_points.values(),
				     challenge_points.numRows(),
				     challenge_points.numCols());
    Eigen::Map<Eigen::MatrixXd> resp(challenge_responses.values(),
				     challenge_responses.length(), 1);

    auto metric_vals = model->evaluate_metrics(diag_set, vars, resp);

    Cout << "\nSurrogate quality metrics at challenge (test) points for "
	 << func_description << ":\n";
    for (size_t i=0; i<diag_set.size(); ++i)
      Cout << std::setw(20) << diag_set[i] << "  " << metric_vals[i] << '\n';
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

  // gymnastics since underlying merge_data_partial is strongly typed
  // and can't pass an Eigen type
  RealArray x(num_v);
  Eigen::Map<VectorXd> x_eig(x.data(), num_v);
  for (size_t i=0; i<num_pts; ++i) {
    ((SharedSurfpackApproxData*)sharedDataRep)->
      sdv_to_realarray(sdv_array[i], x);
    vars.row(i) = x_eig;
    resp(i,0) = sdr_array[i].response_function();
  }
}


Real SurrogatesBaseApprox::value(const Variables& vars)
{
  RealVector x_rv(sharedDataRep->numVars);
  ((SharedSurfpackApproxData*)sharedDataRep)->vars_to_realarray(vars, x_rv);
  return value(x_rv);
}


const RealVector& SurrogatesBaseApprox::gradient(const Variables& vars)
{
  RealVector x_rv(sharedDataRep->numVars);
  ((SharedSurfpackApproxData*)sharedDataRep)->vars_to_realarray(vars, x_rv);
  return gradient(x_rv);
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

  // Could instead use RowVectorXd
  Eigen::Map<Eigen::MatrixXd> eval_pts(c_vars.values(), num_evals, num_vars);
  MatrixXd pred(num_evals, num_qoi);

  model->value(eval_pts, pred);

  return pred(0,0);
}
    
const RealVector& SurrogatesBaseApprox::gradient(const RealVector& c_vars)
{
  const size_t num_evals = 1;
  const size_t num_vars = c_vars.length();

  Eigen::Map<Eigen::MatrixXd> eval_pts(c_vars.values(), num_evals, num_vars);

  // not sending Eigen view of approxGradient as model->gradient calls resize()
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
