#pragma once
#ifndef DAKOTA_ROL_INTERFACE_HPP
#define DAKOTA_ROL_INTERFACE_HPP

// Boost Includes
#include <boost/iostream/filter/line.hpp>

// Dakota Includes
#include "DakotaOptimizer.hpp"

// ROL Includes
#include "ROL_BLAS.hpp"
#include "ROL_LAPACK.hpp"
#include "ROL_LinearOperator.hpp"
#include "ROL_TeuchosVector.hpp"
#include "ROL_Teuchos_Objective.hpp"
#include "ROL_Teuchos_Constraint.hpp"
#include "ROL_Solver.hpp"


namespace rol_interface {

using Dakota::Real;
using Dakota::RealVector;
using Dakota::RealMatrix;

// Forward declaration
class ModelInterface;
class Objective;
class Constraint;
class Jacobian;
class Hessian;

template<typename T>
using ModelFunction = std::function<const T&(const Dakota::Model&)>;

using ModelVector = ModelFunction<RealVector>;
using ModelMatrix = ModelFunction<RealMatrix>;

/// Vector utility functions
inline auto make_vector( int n ) noexcept {
  return ROL::makePtr<ROL::TeuchosVector<int,Real>>(n);
}

inline auto get_vector( ROL::Vector<Real>& x ) noexcept {
  return *(static_cast<Dakota::RealVector&>(x)->getVector());
}

inline auto get_vector( ROL::Ptr<ROL::Vector<Real>>& x ) noexcept {
  return get_vector(*x);
}

inline auto get_vector( const ROL::Vector<Real>& x ) noexcept {
  return *(static_cast<const Dakota::RealVector&>(x)->getVector());
}

inline auto get_vector( const ROL::Ptr<const ROL::Vector<Real>>& x ) noexcept {
  return get_vector(*x);
}

inline auto get_vector_values( ROL::Vector<Real>& x ) noexcept {
  return get_vector(x).values();
}

inline auto get_vector_values( ROL::Ptr<ROL::Vector<Real>>& x ) noexcept {
  return get_vector_values(*x);
}

inline auto get_vector_values( const ROL::Vector<Real>& x ) noexcept {
  return get_vector(x).values(x);
}

inline auto get_vector_values( const ROL::Ptr<const ROL::Vector<Real>>& x ) noexcept {
  return get_vector_values(*x);
}

} // namespace rol_interface


// Dakota-ROL Interface Includes
#include "DakotaROLOutputStreamFilter.hpp"
#include "DakotaROLTraits.hpp"
#include "DakotaROLOutputStream.hpp"
#include "DakotaROLModelInterface.hpp"
#include "DakotaROLObjective.hpp"
#include "DakotaROLBounds.hpp"
#include "DakotaROLConstraint.hpp"
#include "DakotaROLJacobian.hpp"
#include "DakotaROLHessian.hpp"
#include "DakotaROLOptimizer.hpp"

} // namespace rol_interface


namespace Dakota {
  using ROLOptimizer = rol_interface::Optimzer;
} // namespace Dakota



#endif // DAKOTA_ROL_INTERFACE_HPP
