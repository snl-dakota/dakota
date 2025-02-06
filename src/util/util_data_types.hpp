/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_DATA_TYPES_HPP
#define DAKOTA_UTIL_DATA_TYPES_HPP

// TODO: Consider organizing typedefs by purpose / category, including what's
// needed

#include "Teuchos_ParameterList.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"
#include "Teuchos_SerialDenseVector.hpp"

// extend Eigen matrices with Boost serialization
#include <boost/serialization/array.hpp>
#include <boost/serialization/serialization.hpp>
#define EIGEN_MATRIX_PLUGIN "util_eigen_plugins.hpp"
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
template <typename... Ts>
void silence_unused_args(const Ts...) { /* empty body */
}

// TODO: May want to have distinct Dakota names such as EigMatrixXd

/// Eigen generic row vector of doubles in Dakota namespace
using RowVectorXd = Eigen::RowVectorXd;
/// Eigen generic column vector of doubles in Dakota namespace
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

/// Array of strings
using StringArray = std::vector<std::string>;

/// Double precision difference tolerance
const double near_zero = std::abs(10.0 * std::numeric_limits<double>::min());

}  // namespace dakota

#endif  // include guard
