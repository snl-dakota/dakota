/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_types.hpp"

namespace Dakota {

void batch_means_interval(RealMatrix& mcmc_matrix, RealMatrix& interval_matrix,
                          RealMatrix& means_matrix, int moment, Real alpha);
void batch_means_percentile(RealMatrix& mcmc_matrix, RealMatrix& 
                            interval_matrix, RealMatrix& means_matrix, Real 
                            percentile, Real alpha);

} // namespace Dakota
