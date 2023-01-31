#pragma once
#ifndef DAKOTA_ROL_INTERFACE_HPP
#define DAKOTA_ROL_INTERFACE_HPP

#include "dakota_data_types.hpp" // Defines Real

#include "Teuchos_ArrayView.hpp"

#include "ROL_TeuchosVector.hpp"
#include "ROL_Teuchos_Objective.hpp"
#include "ROL_Teuchos_Constraint.hpp"
#include "ROL_Solver.hpp"


namespace Dakota {
namespace rol_interface {

using ROL::Ptr;
using ROL::UpdateType;

using ObjectiveType  = ROL::TeuchosObjective<int,Real>;
using ConstraintType = ROL::TeuchosConstraint<int,Real>;
using BoundType      = ROL::BoundConstraint<Real>;
using ProblemType    = ROL::Problem<Real>;
using SolverType     = ROL::Solver<Real>;

inline auto make_vector( int n ) noexcept {
  return ROL::makePtr<ROL::TeuchosVector<int,Real>>(n);
}


inline auto get_vector( ROL::Vector<Real>& x ) noexcept {
  return *(static_cast<Dakota::RealVector&>(x)->getVector());
}

inline auto get_vector( Ptr<ROL::Vector<Real>>& x ) noexcept {
  return get_vector(*x);
}

inline auto get_vector( const ROL::Vector<Real>& x ) noexcept {
  return *(static_cast<const Dakota::RealVector&>(x)->getVector());
}

inline auto get_vector( const Ptr<const ROL::Vector<Real>>& x ) noexcept {
  return get_vector(*x);
}

inline auto get_vector_values( ROL::Vector<Real>& x ) noexcept {
  return get_vector(x).values();
}

inline auto get_vector_values( Ptr<ROL::Vector<Real>>& x ) noexcept {
  return get_vector_values(*x);
}

inline auto get_vector_values( const ROL::Vector<Real>& x ) noexcept {
  return get_vector(x).values(x);
}

inline auto get_vector_values( const Ptr<const ROL::Vector<Real>>& x ) noexcept {
  return get_vector_values(*x);
}
 

} // namespace rol_interface
} // namespace Dakota


#endif // DAKOTA_ROL_INTERFACE_HPP
