/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "DakotaSurrogates.hpp"

#include "ProblemDescDB.hpp"
#include "DakotaVariables.hpp"

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

  // Need to use Teuchos-to-Eigen converters - RWH
  MatrixXd eval_pts(num_evals, num_vars);
  MatrixXd pred    (num_evals, num_qoi);
  for (size_t j = 0; j < num_vars; j++)
    eval_pts(0,j) = c_vars[j];

  model->value(eval_pts, pred);

  return pred(0,0); // should only be one prediction using this particular call? - RWH 
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
