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
  set_verbosity();
}


SurrogatesBaseApprox::
SurrogatesBaseApprox(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data)
{ set_verbosity(); }


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
  std::shared_ptr<SharedSurfpackApproxData> shared_surf_data_rep =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep);
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
  StringArray diag_set = std::static_pointer_cast<SharedSurfpackApproxData>
    (sharedDataRep)->diagnosticSet;

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
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
      sdv_to_realarray(sdv_array[i], x);
    vars.row(i) = x_eig;
    resp(i,0) = sdr_array[i].response_function();
  }
}


Real SurrogatesBaseApprox::value(const Variables& vars)
{
  RealVector x_rv(sharedDataRep->numVars);
  std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
    vars_to_realarray(vars, x_rv);
  return value(x_rv);
}


const RealVector& SurrogatesBaseApprox::gradient(const Variables& vars)
{
  RealVector x_rv(sharedDataRep->numVars);
  std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
    vars_to_realarray(vars, x_rv);
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

  const int num_vars = c_vars.length();
  Eigen::Map<Eigen::RowVectorXd> eval_point(c_vars.values(), num_vars);

  return model->value(eval_point)(0);
}
    
const RealVector& SurrogatesBaseApprox::gradient(const RealVector& c_vars)
{
  const size_t num_evals = 1;
  const size_t num_vars = c_vars.length();

  Eigen::Map<Eigen::MatrixXd> eval_pts(c_vars.values(), num_evals, num_vars);

  // not sending Eigen view of approxGradient as model->gradient calls resize()
  MatrixXd pred_grad = model->gradient(eval_pts);

  approxGradient.sizeUninitialized(c_vars.length());
  for (size_t j = 0; j < num_vars; j++)
    approxGradient[j] = pred_grad(0,j);

  // BMA TODO: redesign Approximation to not return the class member
  // as its state could be invalidated
  return approxGradient;
}


void SurrogatesBaseApprox::
export_model(const Variables& vars, const String& fn_label,
	     const String& export_prefix, const unsigned short export_format)
{
  // order the variable labels the way the surrogate inputs are ordered
  StringArray var_labels(vars.continuous_variable_labels().begin(),
			 vars.continuous_variable_labels().end());
  var_labels.insert(var_labels.end(),
		    vars.discrete_int_variable_labels().begin(),
		    vars.discrete_int_variable_labels().end());
  var_labels.insert(var_labels.end(),
		    vars.discrete_real_variable_labels().begin(),
		    vars.discrete_real_variable_labels().end());
  export_model(var_labels, fn_label, export_prefix, export_format);
}


void SurrogatesBaseApprox::
export_model(const StringArray& var_labels, const String& fn_label,
	     const String& export_prefix, const unsigned short export_format)
{
  // Surrogates may not be built for some (or all) responses
  if (!model) {
    Cout << "Info: Surrogate for response '" << fn_label << "' not built; "
        << "skipping export." << std::endl;
    return;
  }

  model->variable_labels(var_labels);

  // This block uses prefix, label, maybe formats
  String without_extension;
  unsigned short formats;
  if(export_format) {
    model->response_labels(StringArray(1, fn_label));
    without_extension = export_prefix + "." + fn_label;
    formats = export_format;
  }
  else {
    model->response_labels(StringArray(1, approxLabel));
    without_extension = sharedDataRep->modelExportPrefix + "." + approxLabel;
    formats = sharedDataRep->modelExportFormat;
  }

  // This block without_extension, formats
  // Saving to text archive
  if(formats & TEXT_ARCHIVE) {
    String filename = without_extension + ".txt";
    dakota::surrogates::Surrogate::save(model, filename, false);
  }
  // Saving to binary archive
  if(formats & BINARY_ARCHIVE) {
    String filename = without_extension + ".bin";
    dakota::surrogates::Surrogate::save(model, filename, false);
  }
}

void SurrogatesBaseApprox::set_verbosity()
{
  auto dak_verb = sharedDataRep->outputLevel;
  if (dak_verb == SILENT_OUTPUT || dak_verb == QUIET_OUTPUT)
    surrogateOpts.set("verbosity", 0);
  else if (dak_verb == NORMAL_OUTPUT)
    surrogateOpts.set("verbosity", 1);
  else if (dak_verb == VERBOSE_OUTPUT || dak_verb == DEBUG_OUTPUT)
    surrogateOpts.set("verbosity", 2);
}

} // namespace Dakota
