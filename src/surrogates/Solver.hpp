/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_SOLVER_HPP
#define DAKOTA_SURROGATES_SOLVER_HPP

#include "DataScaler.hpp"

namespace dakota {
namespace surrogates {

class Solver {

public:

  // Constructor
  Solver();

  // Destructor
  virtual ~Solver();

  // Virtual functions
  virtual const VectorXd solve(const MatrixXd& basis_matrix, const MatrixXd& responses);

};
} // namespace surrogates
} // namespace dakota

#endif