/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "bayes_calibration_utils.hpp"
#include "dakota_data_util.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"
#include <boost/math/distributions/students_t.hpp>
using namespace boost::math;

namespace Dakota {

void batch_means_interval(RealMatrix& mcmc_matrix, RealMatrix& interval_matrix,
                          RealMatrix& means_matrix, int moment, Real alpha)
{
  int num_qoi = mcmc_matrix.numRows();
  int num_samples = mcmc_matrix.numCols();
  int batch_size = sqrt(num_samples);
  int num_batches = num_samples/batch_size; // What about when this doesn't divide evenly? Do we put the extras in the last batch? 
  interval_matrix.reshape(2, num_qoi);
  means_matrix.reshape(num_batches, num_qoi);

  // Compute statistic for whole chain
  //accumulator_set<RealVector, stats<tag::mean> > acc;
  RealMatrix mcmc_matrix_transpose(mcmc_matrix, Teuchos::TRANS);
  RealVector mean_totalchain(num_qoi);
  compute_col_means(mcmc_matrix_transpose, mean_totalchain);
  RealVector func_totalchain(num_qoi);
  if (moment == 1)
    func_totalchain = mean_totalchain;
  if (moment == 2) {
    RealVector stdev_totalchain(num_qoi);
    compute_col_stdevs(mcmc_matrix_transpose, mean_totalchain, 
                       stdev_totalchain);
    for (int i = 0; i < num_qoi; i++) 
      func_totalchain[i] = pow(stdev_totalchain[i], 2);
  }

  // Compute statistic for batches
  RealVector approx_var_chain(num_qoi);
  RealMatrix means_matrix_t(num_qoi, num_batches);
  for (int i = 0; i < num_batches; i++) {
    RealMatrix batch_j_matrix(Teuchos::View, mcmc_matrix_transpose,
                              batch_size, num_qoi, i*batch_size, 0);
    RealVector mean_subchain(num_qoi);
    compute_col_means(batch_j_matrix, mean_subchain);
    RealVector func_subchain(num_qoi);
    if (moment == 1)
      func_subchain = mean_subchain;
    else if (moment == 2) {
      RealVector stdev_subchain(num_qoi);
      compute_col_stdevs(batch_j_matrix, mean_subchain, stdev_subchain);
      for (int j = 0; j < num_qoi; j++) 
        func_subchain[j] = pow(stdev_subchain[j], 2);
    }
    for (int j = 0; j < num_qoi; j++) 
      approx_var_chain[j] += pow(func_subchain[j] - func_totalchain[j],2);
    Teuchos::setCol(func_subchain, i, means_matrix_t);
  }
  RealMatrix means_matrix_tt(means_matrix_t, Teuchos::TRANS);
  means_matrix = means_matrix_tt;

  // Calculate approximate variance
  Real scale = batch_size/(num_batches - 1);
  for (int i = 0; i < num_qoi; i++) {
    approx_var_chain[i] = scale*approx_var_chain[i];
  }

  // Calculate interval
  // currently assume the same quantile for all components
  boost::math::students_t t_dist(num_samples-1);
  Real t_star = quantile(complement(t_dist, (1-alpha)/2));
  RealVector interval_i(2);
  for (int i = 0; i < num_qoi; i++) {
    Real half_width = t_star*std::sqrt(approx_var_chain[i]/num_samples);
    interval_i[0] = func_totalchain[i] - half_width;
    if (moment == 2 && interval_i[0] < 0) 
      interval_i[0] = 0; // variance must be positive
    interval_i[1] = func_totalchain[i] + half_width;
    Teuchos::setCol(interval_i, i, interval_matrix);
  }
}

void batch_means_percentile(RealMatrix& mcmc_matrix, RealMatrix& 
                            interval_matrix, RealMatrix& means_matrix, Real 
                            percentile, Real alpha)
{
  int num_qoi = mcmc_matrix.numRows();
  int num_samples = mcmc_matrix.numCols();
  int batch_size = sqrt(num_samples);
  int num_batches = num_samples/batch_size; // What about when this doesn't divide evenly? Do we put the extras in the last batch? 
  interval_matrix.reshape(2, num_qoi);
  means_matrix.reshape(num_batches, num_qoi);
  
  // Compute percentile for whole chain
  RealMatrix mcmc_matrix_transpose(mcmc_matrix, Teuchos::TRANS);
  RealVector func_totalchain(num_qoi);
  int chain_index = floor(percentile/100*num_samples);
  for (int i = 0; i < num_qoi; i++) {
    RealVector qoi_vec = Teuchos::getCol(Teuchos::Copy, 
                                  mcmc_matrix_transpose, i);
    std::sort(qoi_vec.values(), qoi_vec.values() + num_samples);
    func_totalchain[i] = qoi_vec[chain_index];
  }
  
  // Compute percentile for batches
  RealVector approx_var_chain(num_qoi);
  RealMatrix means_matrix_t(num_qoi, num_batches);
  int subchain_index = floor(percentile/100*batch_size);
  for (int i = 0; i < num_batches; i++) {
    RealMatrix batch_j_matrix(Teuchos::View, mcmc_matrix_transpose,
                              batch_size, num_qoi, i*batch_size, 0);
    RealVector func_subchain(num_qoi);
    for (int j = 0; j < num_qoi; j++) {
      RealVector batch_qoi_vec = Teuchos::getCol(Teuchos::Copy, 
                                                 batch_j_matrix,j);
      std::sort(batch_qoi_vec.values(), batch_qoi_vec.values() + batch_size);
      func_subchain[j] = batch_qoi_vec[subchain_index];
      approx_var_chain[j] += pow(func_subchain[j] - func_totalchain[j],2);
    }
    Teuchos::setCol(func_subchain, i, means_matrix_t);
  }
  RealMatrix means_matrix_tt(means_matrix_t, Teuchos::TRANS);
  means_matrix = means_matrix_tt;

  // Calculate approximate variance
  Real scale = batch_size/(num_batches - 1);
  for (int i = 0; i < num_qoi; i++) {
    approx_var_chain[i] = scale*approx_var_chain[i];
  }

  // Calculate interval
  // currently assume the same quantile for all components
  boost::math::students_t t_dist(num_samples-1);
  Real t_star = quantile(complement(t_dist, (1-alpha)/2));
  RealVector interval_i(2);
  for (int i = 0; i < num_qoi; i++) {
    Real half_width = t_star*std::sqrt(approx_var_chain[i]/num_samples);
    interval_i[0] = func_totalchain[i] - half_width;
    interval_i[1] = func_totalchain[i] + half_width;
    Teuchos::setCol(interval_i, i, interval_matrix);
  }
}

} // namespace Dakota
