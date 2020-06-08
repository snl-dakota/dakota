/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        GPApproximation
//- Description:  .....
//-               
//- Owner:        .....

#include "DakotaSurrogatesPoly.hpp"

#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"
#include "SharedSurfpackApproxData.hpp"

// Headers from Surrogates module
#include "PolynomialRegression.hpp"
 

using dakota::VectorXd;
using dakota::MatrixXd;

namespace Dakota {


SurrogatesPolyApprox::
SurrogatesPolyApprox(const ProblemDescDB& problem_db,
		const SharedApproxData& shared_data,
		const String& approx_label):
  Approximation(BaseConstructor(), problem_db, shared_data, approx_label)
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
  Approximation(NoDBBaseConstructor(), shared_data)
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
  size_t num_v = sharedDataRep->numVars;
  int num_qoi             = 1; // only using 1 for now

  const Pecos::SurrogateData& approx_data = surrogate_data();
  const Pecos::SDVArray& sdv_array = approx_data.variables_data();
  const Pecos::SDRArray& sdr_array = approx_data.response_data();

  int num_pts = approx_data.points();

  // num_samples x num_features
  MatrixXd xs_u(num_pts, num_v);
  // num_samples x num_qoi
  MatrixXd response(num_pts, num_qoi);

  // Need to use Teuchos-to-Eigen converters - RWH
  for (size_t i=0; i<num_pts; ++i)
  {
    const RealVector& c_vars = sdv_array[i].continuous_variables();
    for (size_t j=0; j<num_v; j++){
      xs_u(i,j) = c_vars[j];
    }
    response(i,0) = sdr_array[i].response_function();
  }

  // construct the surrogate
  model.reset(new dakota::surrogates::PolynomialRegression(xs_u, response, surrogateOpts));
}


Real SurrogatesPolyApprox::value(const Variables& vars)
{
  return value(vars.continuous_variables());
}


const RealVector& SurrogatesPolyApprox::gradient(const Variables& vars)
{
  return gradient(vars.continuous_variables());
}


Real
SurrogatesPolyApprox::value(const RealVector& c_vars)
{
  if (!model)
  {
    Cerr << "Error: surface is null in SurfpackApproximation::value()"
      << std::endl;
    abort_handler(-1);
  }

  const size_t num_evals = 1;
  const size_t num_vars = c_vars.length();
  const size_t num_qoi = 1;

  //if (num_vars != 1 )
  //{
  //  Cerr << "Error: SurrogatesPolyApprox currently supports a sigle parameter for now."
  //    << std::endl;
  //  abort_handler(-1);
  //}

  // Need to use Teuchos-to-Eigen converters - RWH
  MatrixXd eval_pts(num_evals, num_vars);
  MatrixXd pred    (num_evals, num_qoi);
  for (size_t j = 0; j < num_vars; j++)
    eval_pts(0,j) = c_vars[j];

  model->value(eval_pts, pred);

  return pred(0,0); // should only be one prediuction using this particular call? - RWH 
}
    
const RealVector& SurrogatesPolyApprox::gradient(const RealVector& c_vars)
{
  const size_t num_evals = 1;
  const size_t num_vars = c_vars.length();

  // Need to use Teuchos-to-Eigen converters - RWH
  MatrixXd eval_pts(num_evals, num_vars);
  for (size_t j = 0; j < num_vars; j++)
    eval_pts(0,j) = c_vars[j];

  // could avoid the temporary and copy by passing an Eigen view of
  // approxGradient
  MatrixXd pred_grad(num_evals, num_vars);
  model->gradient(eval_pts, pred_grad);

  approxGradient.sizeUninitialized(c_vars.length());
  for (size_t j = 0; j < num_vars; j++)
    approxGradient[j] = pred_grad(0,j);

  // BMA TODO: redesign Approximation to not return the class member
  // as its state could be invalidated
  return approxGradient;
}


} // namespace Dakota
