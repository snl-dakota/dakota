/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_types.hpp"

namespace Dakota {

void batch_means_interval(RealMatrix& mcmc_matrix, RealMatrix& interval_matrix,
                          int moment, Real alpha);

} // namespace Dakota