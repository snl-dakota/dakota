#pragma once
#ifndef DAKOTA_ROL_INTERFACE_HPP
#define DAKOTA_ROL_INTERFACE_HPP

// Dakota Includes
#include "DakotaOptimizer.hpp"

// ROL Includes
#include "ROL_BLAS.hpp"
#include "ROL_LAPACK.hpp"
#include "ROL_LinearOperator.hpp"
#include "ROL_Solver.hpp"


namespace rol_interface {

// Forward declaration
class Vector;
class ModelInterface;
class Objective;
class Constraint;
class Jacobian;
class Hessian;

template<typename T>
using ModelFunction = std::function<const T&(const Dakota::Model&)>;

using ModelVector = ModelFunction<Dakota::RealVector>;
using ModelMatrix = ModelFunction<Dakota::RealMatrix>;

} // namespace rol_interface


// Dakota-ROL Interface Includes
#include "DakotaROLOutputStreamFilter.hpp"
#include "DakotaROLTraits.hpp"
#include "DakotaROLVector.hpp"
#include "DakotaROLModelInterface.hpp"
#include "DakotaROLObjective.hpp"
#include "DakotaROLBounds.hpp"
#include "DakotaROLConstraint.hpp"
#include "DakotaROLJacobian.hpp"
#include "DakotaROLHessian.hpp"
#include "DakotaROLOptimizer.hpp"




namespace Dakota {
  using ROLOptimizer = rol_interface::Optimzer;
} // namespace Dakota



#endif // DAKOTA_ROL_INTERFACE_HPP
