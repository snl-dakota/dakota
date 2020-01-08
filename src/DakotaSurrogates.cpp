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

#include <stdexcept>
#include <typeinfo>

#include "DakotaSurrogates.hpp"

// Headers from Surrogates module
#include "GaussianProcess.hpp"
 
#include <algorithm>
#include <boost/math/special_functions/round.hpp>


namespace Dakota {

/// On-the-fly constructor
GPApproximation::
GPApproximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data)
{
}


int
GPApproximation::min_coefficients() const
{
  return model->get_num_variables();
}

void
GPApproximation::build()
{
  // Hard-coded values to quickly get things working ...
  // See src/surrogates/unit/gp_approximation_ts.cpp for correspondence

  int num_qoi             = 1; // only using 1 for now
  std::string scaler_type = "standardization";
  int num_restarts        = 10;
  int gp_seed             = 42;
  double nugget           = 1.0e-12;

  // bound constraints -- will be converted to log-scale internally
  // sigma bounds - lower and upper
  VectorXd sigma_bounds(2);
  sigma_bounds(0) = 1.0e-2;
  sigma_bounds(1) = 1.0e2;
  // length scale bounds - num_vars x 2
  MatrixXd length_scale_bounds(1,2);
  length_scale_bounds(0,0) = 1.0e-2;
  length_scale_bounds(0,1) = 1.0e2;

  const Pecos::SurrogateData& approx_data = surrogate_data();
  size_t i, j, num_v = sharedDataRep->numVars;
  const Pecos::SDVArray& sdv_array = approx_data.variables_data();
  const Pecos::SDRArray& sdr_array = approx_data.response_data();

  int num_pts = approx_data.points();

  // num_samples x num_features
  MatrixXd xs_u(num_pts, num_qoi);
  // num_samples x num_qoi
  MatrixXd response(num_pts, num_qoi);

  // Need to use Teuchos-to-Eigen converters - RWH
  for (i=0; i<num_pts; ++i)
  {
    const RealVector& c_vars = sdv_array[i].continuous_variables();
    for (j=0; j<num_v; j++){
      xs_u(i,j) = c_vars[j];
    }
    response(i) = sdr_array[i].response_function();
  }

  // construct the surrogate
  model = new dakota::surrogates::GaussianProcess(
                                    xs_u, response,
                                    sigma_bounds, length_scale_bounds,
                                    scaler_type, num_restarts, nugget, gp_seed);
}


Real
GPApproximation::value(const RealVector& c_vars)
{
  if (!model)
  {
    Cerr << "Error: surface is null in SurfpackApproximation::value()"
      << std::endl;
    abort_handler(-1);
  }

  size_t num_vars = c_vars.length();

  //if (num_vars != 1 )
  //{
  //  Cerr << "Error: GPApproximation currently supports a sigle parameter for now."
  //    << std::endl;
  //  abort_handler(-1);
  //}

  // Need to use Teuchos-to-Eigen converters - RWH
  MatrixXd eval_pts(num_vars,1);
  MatrixXd pred    (num_vars,1);
  for (size_t i = 0; i < num_vars; i++)
    eval_pts(i,0) = c_vars[i];

  model->value(eval_pts, pred);

  return pred(0,00); // should only be one prediuction using this particular call? - RWH
}
    

} // namespace Dakota
