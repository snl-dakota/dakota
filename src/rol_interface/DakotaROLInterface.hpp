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
class Optimizer;

//  template<typename T>
//  using ModelFunction = std::function<const T&(const Dakota::Model&)>;
//  
//  using ModelVector = ModelFunction<Dakota::RealVector>;
//  using ModelMatrix = ModelFunction<Dakota::RealMatrix>;

} // namespace rol_interface


// Dakota-ROL Interface Includes
#include "DakotaROLOutputStreamFilter.hpp"
#include "DakotaROLTraits.hpp"
#include "DakotaROLVector.hpp"
#include "DakotaROLConstraint.hpp"
#include "DakotaROLModelInterface.hpp"
#include "DakotaROLObjective.hpp"
#include "DakotaROLBounds.hpp"
#include "DakotaROLHessian.hpp"
#include "DakotaROLOptimizer.hpp"

#include "constraint/DakotaROLJacobian.hpp"
#include "constraint/DakotaROLLinearEqualityConstraint.hpp"
#include "constraint/DakotaROLLinearInequalityConstraint.hpp"
#include "constraint/DakotaROLNonlinearEqualityConstraint.hpp"
#include "constraint/DakotaROLNonlinearInequalityConstraint.hpp"


namespace Dakota {
  using ROLOptimizer = rol_interface::Optimizer;
} // namespace Dakota



#endif // DAKOTA_ROL_INTERFACE_HPP
