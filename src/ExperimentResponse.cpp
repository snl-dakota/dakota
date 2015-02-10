/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ExperimentResponse
//- Description:  Class implementation
//- Owner:        Laura Swiler

#include "ExperimentResponse.hpp"
#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: ExperimentResponse.cpp 7029 2010-10-22 00:17:02Z mseldre $";


namespace Dakota {

ExperimentResponse::
ExperimentResponse(const Variables& vars, const ProblemDescDB& problem_db):
  Response(BaseConstructor(), vars, problem_db)
{
  sharedRespData.response_type(EXPERIMENT_RESPONSE);

  // initialize the response covariance data...
}


ExperimentResponse::
ExperimentResponse(const SharedResponseData& srd):
  Response(BaseConstructor(), srd)
{
  // initialize the response covariance data...
}


ExperimentResponse::ExperimentResponse(const ActiveSet& set):
  Response(BaseConstructor(), set)
{
  sharedRespData.response_type(EXPERIMENT_RESPONSE);

  // initialize the response covariance data...
}

void ExperimentResponse::set_scalar_covariance(RealVector& scalarSigmas)
{
  std::vector<RealMatrix> matrices;
  std::vector<RealVector> diagonals;
  //RealVector scalars;
  IntVector matrix_map_indices, diagonal_map_indices, scalar_map_indices;

  int num_scalars = scalarSigmas.length() ;
  //Real scalar_array[] = {1.,2.,4.};
  //int scalar_map_index_array[] = {0, 1, 2};
  //scalars.sizeUninitialized(num_scalars);
  scalar_map_indices.sizeUninitialized(num_scalars);
  for ( int i=0; i<num_scalars; i++ ){
    //scalars[i] = scalar_array[i];
    scalar_map_indices[i] = i;
  }

  expDataCovariance.set_covariance_matrices( matrices, diagonals, scalarSigmas,
                                     matrix_map_indices,
                                     diagonal_map_indices,
                                     scalar_map_indices );
  // Might make this depend on a verbosity output level - RWH
  //expDataCovariance.print_cov();
}

void ExperimentResponse::set_full_covariance(std::vector<RealMatrix> &matrices,
                           std::vector<RealVector> &diagonals,
                           RealVector &scalars,
                           IntVector matrix_map_indices,
                           IntVector diagonal_map_indices,
                           IntVector scalar_map_indices )
{
  expDataCovariance.set_covariance_matrices( matrices, diagonals, scalars,
                                     matrix_map_indices,
                                     diagonal_map_indices,
                                     scalar_map_indices );
  // Might make this depend on a verbosity output level - RWH
  //expDataCovariance.print_cov();
}

Real ExperimentResponse::get_scalar_covariance(const int this_response)
{
  RealMatrix thisCovariance;
  //expDataCovariance.get_covariance(thisCovariance);

  // TODO: inquire about the proper return (SCALAR?) value
  //return thisCovariance[0][0];
  // BUT, in the meantime, properly satisfy the interface (return a Real)
  return 0;
}
 
Real ExperimentResponse::apply_covariance(RealVector &residual)
{
  return expDataCovariance.apply_experiment_covariance(residual);
}

void ExperimentResponse::
apply_covariance_inv_sqrt(RealVector& residuals, RealVector& weighted_residuals)
{
  expDataCovariance.
    apply_experiment_covariance_inverse_sqrt(residuals, weighted_residuals);
}

void ExperimentResponse::
apply_covariance_inv_sqrt(RealMatrix& gradients, RealMatrix& weighted_gradients)
{
  expDataCovariance.
    apply_experiment_covariance_inverse_sqrt_to_gradients(gradients, 
							  weighted_gradients);
}

void  ExperimentResponse::
apply_covariance_inv_sqrt(RealSymMatrixArray& hessians)
{
  expDataCovariance.
    apply_experiment_covariance_inverse_sqrt_to_hessians(hessians); 
}

void ExperimentResponse::copy_rep(Response* source_resp_rep)
{
  // copy base class data
  Response::copy_rep(source_resp_rep);
  // specialization for experiment; assume the source and destination
  // rep are same derived type
  ExperimentResponse * expt_resp_rep = static_cast<ExperimentResponse*>(source_resp_rep);
  if(expt_resp_rep)
    expDataCovariance = expt_resp_rep->expDataCovariance;
  else
    throw std::runtime_error("Cast to ExperimentResponse failed.");
}


} // namespace Dakota
