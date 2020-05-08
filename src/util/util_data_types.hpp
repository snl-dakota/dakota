/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_DATA_TYPES_HPP
#define DAKOTA_UTIL_DATA_TYPES_HPP

// TODO: Consider organizing typedefs by purpose / category, including what's needed

#include "Teuchos_ParameterList.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"
#include "Teuchos_SerialDenseVector.hpp"

#include "Eigen/Dense"


/// dakota (lowercase) namespace for new %Dakota modules
namespace dakota {

/// namespace for new %Dakota utilities module
namespace util {
  // empty
}

// The following are in dakota namespace since shared across util and
// surrogates and the extra qualification doesn't help

// TODO: determine proper variadic template syntax for const &
/// silence unused parameter warning; use to indicate those parameters
/// are intentionally unused
template <typename ... Ts> void silence_unused_args(const Ts ...)
{  /* empty body */  }

// TODO: May want to have distinct Dakota names such as EigMatrixXd

/// Eigen generic vector of doubles in Dakota namespace
using VectorXd = Eigen::VectorXd;
/// Eigen generic matrix of doubles in Dakota namespace
using MatrixXd = Eigen::MatrixXd;
/// Eigen generic vector of integers in Dakota namespace
using VectorXi = Eigen::VectorXi;
/// Eigen generic matrix of integers in Dakota namespace
using MatrixXi = Eigen::MatrixXi;

/// Dakota real floating point type
using Real = double;
/// Dakota matrix of reals
using RealMatrix = Teuchos::SerialDenseMatrix<int, Real>;
/// Dakota vector of reals
using RealVector = Teuchos::SerialDenseVector<int, Real>;

/// Teuchos ParameterList for options management in Dakota namespace
using ParameterList = Teuchos::ParameterList;

}  // namespace dakota

#endif  // include guard
