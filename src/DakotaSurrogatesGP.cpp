/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaSurrogatesGP.hpp"

#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"
#include "SharedSurfpackApproxData.hpp"

// Headers from Surrogates module
#include "SurrogatesGaussianProcess.hpp"
 
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
  else if (trend_string == "reduced_quadratic")
  {
    surrogateOpts.sublist("Trend").sublist("Options").set("max degree", 2);
    surrogateOpts.sublist("Trend").sublist("Options").set("reduced basis", true);
  }
  else if (trend_string == "none")
    surrogateOpts.sublist("Trend").set("estimate trend", false);
  surrogateOpts.sublist("Trend").sublist("Options").set("verbosity",
      surrogateOpts.get<int>("verbosity"));

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
  std::shared_ptr<SharedSurfpackApproxData> shared_surf_data_rep =
    std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep);
  shared_surf_data_rep->validate_metrics(allowed_metrics);

  if (problem_db.get_bool("model.surrogate.import_surrogate"))
    import_model(problem_db);
}


/// On-the-fly constructor
SurrogatesGPApprox::
SurrogatesGPApprox(const SharedApproxData& shared_data):
  SurrogatesBaseApprox(shared_data)
{
  // other GPs default to reduced_quadratic
  //surrogateOpts.sublist("Trend").set("estimate trend", true);
  //surrogateOpts.sublist("Trend").sublist("Options").set("max degree", 2);
  //surrogateOpts.sublist("Trend").sublist("Options").set("reduced basis", true);

  surrogateOpts.set("num restarts", 20);

  // allow larger bounds for functions with high variability
  //VectorXd sig_bnds(2);
  //sig_bnds << 1.0e-2, 1.0e4;
  //surrogateOpts.sublist("Sigma Bounds").set("lower bound", sig_bnds(0));
  //surrogateOpts.sublist("Sigma Bounds").set("upper bound", sig_bnds(1));

  // use same verbosity level for polynomial trend
  //surrogateOpts.sublist("Trend").sublist("Options").set("verbosity",
  //    surrogateOpts.get<int>("verbosity"));

  // by default, estimate the nugget
  //surrogateOpts.sublist("Nugget").set("estimate nugget", true);
  //surrogateOpts.sublist("Nugget").set("fixed nugget", 0.0);

  // nugget bounded by [1.0e-15, 1.0e-8]
  //VectorXd nugget_bounds(2);
  //nugget_bounds << 3.17e-8, 1.0e-4;
  //surrogateOpts.sublist("Nugget").sublist("Bounds")
  //  .set("lower bound", nugget_bounds(0));
  //surrogateOpts.sublist("Nugget").sublist("Bounds")
  //  .set("upper bound", nugget_bounds(1));
}

int
SurrogatesGPApprox::min_coefficients() const
{
  // TODO (with @dtseidl): This should be based on minimum points
  // needed to build the trend, when present, or some other reasonable
  // default
  return sharedDataRep->numVars + 1;
}


void
SurrogatesGPApprox::build(int num_resp)
{
  // clear any imported model mapping
  modelIsImported = false;
  std::static_pointer_cast<SharedSurfpackApproxData>(sharedDataRep)->
    varsMapIndices.clear();

  MatrixXd vars, resp;
  convert_surrogate_data(vars, resp);

  /* DTS: Should also consider the case when we want config options to change
   * over the course of EG*-type algorithms */

  if (!advanced_options_file.empty()) {
    model.reset(new dakota::surrogates::GaussianProcess
          (vars, resp, advanced_options_file));
  }
  else {
    model.reset(new dakota::surrogates::GaussianProcess
          (vars, resp, surrogateOpts));
  }

  /* DTS: This is not working as I thought it would ... */
  /*
  if (!model) {
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
  else {
    model->build(vars, resp);
  }
  */
}

Real SurrogatesGPApprox::prediction_variance(const Variables& vars)
{
  return prediction_variance(map_eval_vars(vars));
}

Real SurrogatesGPApprox::prediction_variance(const RealVector& c_vars)
{
  if (!model) {
    Cerr << "Error: surface is null in SurrogatesBaseApprox::value()"
	 << std::endl;
    abort_handler(-1);
  }

  Eigen::Map<Eigen::RowVectorXd> eval_point(c_vars.values(), c_vars.length());

  auto gp_model =
      std::static_pointer_cast<dakota::surrogates::GaussianProcess>(model);

  return gp_model->variance(eval_point)(0);
}

void set_model_gp_options(Model& model, const String& options_file) {
  auto custom_param_list = Teuchos::getParametersFromYamlFile(options_file);
  std::vector<Approximation>& exp_gp_approxs = model.approximations();
  for (int i = 0; i < exp_gp_approxs.size(); ++i) {
    auto exp_gp_derived
      = std::static_pointer_cast<SurrogatesGPApprox>(
      exp_gp_approxs[i].approx_rep());
    auto& gp_param_list = exp_gp_derived->getSurrogateOpts();
    gp_param_list = *custom_param_list;
  }
}

} // namespace Dakota
