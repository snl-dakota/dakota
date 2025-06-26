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
class ModelInterface;
class Vector;
class Objective;
class Constraint;
class LinearConstraint;
class LinearEqualityConstraint;
class LinearInequalityConstraint;
class NonlinearEqualityConstraint;
class NonlinearInequalityConstraint;
class Hessian;
class Optimizer;

}  // namespace rol_interface

// Dakota-ROL Interface Includes
#include "DakotaROLConstraint.hpp"
#include "DakotaROLHessian.hpp"
#include "DakotaROLModelInterface.hpp"
#include "DakotaROLObjective.hpp"
#include "DakotaROLOptimizer.hpp"
#include "DakotaROLOutputStreamFilter.hpp"
#include "DakotaROLTraits.hpp"
#include "DakotaROLVector.hpp"
#include "constraint/DakotaROLConstraint.hpp"
#include "constraint/DakotaROLLinearConstraint.hpp"
#include "constraint/DakotaROLLinearEqualityConstraint.hpp"
#include "constraint/DakotaROLLinearInequalityConstraint.hpp"
#include "constraint/DakotaROLNonlinearEqualityConstraint.hpp"
#include "constraint/DakotaROLNonlinearInequalityConstraint.hpp"

namespace Dakota {
using ROLOptimizer = rol_interface::Optimizer;
}  // namespace Dakota

#endif  // DAKOTA_ROL_INTERFACE_HPP
