/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
  FieldApproximation(BaseConstructor(), problem_db, shared_data, StringArray{approx_label})
{
  advanced_options_file = problem_db.get_string("model.advanced_options_file");
  set_verbosity();
}


SurrogatesBaseApprox::
SurrogatesBaseApprox(const ProblemDescDB& problem_db,
		     const SharedApproxData& shared_data,
		     const StringArray& approx_labels):
  FieldApproximation(BaseConstructor(), problem_db, shared_data, approx_labels)
{
  advanced_options_file = problem_db.get_string("model.advanced_options_file");
  set_verbosity();
}


SurrogatesBaseApprox::
SurrogatesBaseApprox(const SharedApproxData& shared_data):
  FieldApproximation(NoDBBaseConstructor(), shared_data)
{ set_verbosity(); }


bool SurrogatesBaseApprox::diagnostics_available()
{ return model->diagnostics_available(); }


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


void SurrogatesBaseApprox::primary_diagnostics(size_t fn_index)
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
challenge_diagnostics(size_t fn_index, const RealMatrix& challenge_points,
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
SurrogatesBaseApprox::convert_surrogate_data(MatrixXd& vars, MatrixXd& resp, int num_resp)
{
  size_t num_v = sharedDataRep->numVars;
  int num_qoi             = 1; // only using 1 for now

  const Pecos::SurrogateData& approx_data = surrogate_data();
  const Pecos::SDVArray& sdv_array = approx_data.variables_data();
  const Pecos::SDRArray& sdr_array = approx_data.response_data();

  int num_pts = approx_data.points()/num_resp;

  // num_samples x num_features
  vars.resize(num_pts, num_v);
  // num_samples x num_resp
  resp.resize(num_pts, num_resp);

  // gymnastics since underlying merge_data_partial is strongly typed
  // and can't pass an Eigen type
  RealArray x(num_v);
  RealArray r(num_resp);
  Eigen::Map<VectorXd> x_eig(x.data(), num_v);
  Eigen::Map<VectorXd> r_eig(r.data(), num_resp);
  for (size_t i=0; i<num_pts; ++i) {
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
      sdv_to_realarray(sdv_array[i*num_resp], x);
    vars.row(i) = x_eig;
    //resp(i,0) = sdr_array[i].response_function();
    for (int j=0; j<num_resp; ++j)
      r_eig(j) = sdr_array[i*num_resp+j].response_function();
    resp.row(i) = r_eig;
  }
}


Real SurrogatesBaseApprox::value(const Variables& vars)
{
  return value(map_eval_vars(vars));
}


RealVector SurrogatesBaseApprox::values(const Variables& vars)
{
  return values(map_eval_vars(vars));
}


const RealVector& SurrogatesBaseApprox::gradient(const Variables& vars)
{
  return gradient(map_eval_vars(vars));
}


const RealSymMatrix& SurrogatesBaseApprox::hessian(const Variables& vars)
{
  return hessian(map_eval_vars(vars));
}


RealVector SurrogatesBaseApprox::map_eval_vars(const Variables& vars)
{
  if (modelIsImported)
    return std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
      imported_eval_vars<RealVector>(vars);
  else {
    // active or all variables
    RealVector surr_vars(sharedDataRep->numVars);
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
      vars_to_realarray(vars, surr_vars);
    return surr_vars;
  }
}


Real
SurrogatesBaseApprox::value(const RealVector& c_vars)
{
  if (!model) {
    Cerr << "Error: surface is null in SurrogatesBaseApprox::value()"
	 << std::endl;
    abort_handler(-1);
  }

  Eigen::Map<Eigen::RowVectorXd> eval_point(c_vars.values(), c_vars.length());
  return model->value(eval_point)(0);
}
    
RealVector
SurrogatesBaseApprox::values(const RealVector& c_vars)
{
  if (!model) {
    Cerr << "Error: surface is null in SurrogatesBaseApprox::values()"
	 << std::endl;
    abort_handler(-1);
  }

  Eigen::Map<Eigen::RowVectorXd> eval_point(c_vars.values(), c_vars.length());
  VectorXd vals = model->values(eval_point);

  RealVector ret_vals(vals.size());
  for (size_t j = 0; j < vals.size(); j++)
    ret_vals[j] = vals(j);

  return ret_vals;
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


const RealSymMatrix& SurrogatesBaseApprox::hessian(const RealVector& c_vars)
{
  const size_t num_evals = 1;
  const size_t num_vars = c_vars.length();

  Eigen::Map<Eigen::MatrixXd> eval_pts(c_vars.values(), num_evals, num_vars);

  // not sending Eigen view of approxGradient as model->gradient calls resize()
  MatrixXd pred_hess = model->hessian(eval_pts);
  Cout << "SurrogatesBaseApprox::hessian :\n" << pred_hess << std::endl;

  approxHessian.reshape(c_vars.length());
  for (size_t i = 0; i < num_vars; i++)
    for(size_t j = 0; j <= i; j++)
      approxHessian(i,j) = pred_hess(i,j);

  return approxHessian;
}


void SurrogatesBaseApprox::
import_model(const ProblemDescDB& problem_db)
{
  auto import_prefix =
    problem_db.get_string("model.surrogate.model_import_prefix");
  auto import_format =
    problem_db.get_ushort("model.surrogate.model_import_format");
  bool is_binary = import_format & BINARY_ARCHIVE;
  std::string filename = import_prefix + "." + approxLabel +
    (is_binary ? ".bin" : ".txt");

  model = dakota::surrogates::Surrogate::load(filename, is_binary);

  if (sharedDataRep->outputLevel >= NORMAL_OUTPUT)
    Cout << "Imported surrogate for response '" << approxLabel
	 << "' from file '" << filename << "'." << std::endl;

  if (sharedDataRep->outputLevel >= SILENT_OUTPUT &&
      !model->response_labels().empty()) {
    auto imported_label = model->response_labels()[0];
    if (imported_label != approxLabel)
      Cout << "\nWarning: Surrogate imported from file " << filename
	   << "\nhas response label '" << imported_label << "'; expected '"
	   << approxLabel << "'." << std::endl;
  }

  modelIsImported = true;
  std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
    varsMapIndices.clear();
}


void
SurrogatesBaseApprox::map_variable_labels(const Variables& dfsm_vars)
{
  // When importing, always map from all variables to the
  // subset needed by the surrogate...
  const auto& approx_labels = model->variable_labels();
  std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
    map_variable_labels(dfsm_vars, approx_labels);
}


void SurrogatesBaseApprox::
export_model(const Variables& vars, const String& fn_label,
	     const String& export_prefix, const unsigned short export_format)
{
  StringArray var_labels =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
    variable_labels(vars);
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
    dakota::surrogates::Surrogate::save(model, filename, true);
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
