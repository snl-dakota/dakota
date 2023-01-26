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

inline container_type& get_vector( ROL::Vector<Real>& x ) noexcept {
  return *(static_cast<vector_type>(x)->getVector());
}

inline container_type& get_vector( Ptr<ROL::Vector<Real>>& x ) noexcept {
  return *(ROL::staticPtrCast<vector_type>(x)->getVector());
}

inline Dakota::RealVector 
copy_as_RealVector( const ROL::Vector<Real>& x ) noexcept {
  return RealVector( Teuchos::Copy,
                     get_vector(x)->data(),
                     x.dimension() );
}

inline Dakota::RealVector 
view_as_RealVector( ROL::Vector<Real>& x ) noexcept {
  return RealVector( Teuchos::View,
                     get_vector(x)->data(),
                     x.dimension() );
}

 

} // namespace rol_interface
} // namespace Dakota


#endif // DAKOTA_ROL_INTERFACE_HPP
