#ifndef DAKOTA_ROL_INTERFACE_HPP
#define DAKOTA_ROL_INTERFACE_HPP

// Standard Library Includes
#include <type_traits>
#include <utility>

// Boost Includes
#include <boost/iostreams/filter/line.hpp>
#include <boost/iostreams/filtering_stream.hpp>

// Forward declarations (no Dakota includes to avoid circular dependency)
namespace Dakota {
  class ProblemDescDB;
  class Model;
  class ROLOptimizer;
  using Real = double;
}

// ROL Includes
#include "ROL_Problem.hpp"
#include "ROL_Solver.hpp"

// Forward Declarations and aliases
namespace rol_interface {

class Vector;
class Objective;
class Constraint;

using Dakota::Real;
using ROLVector = ROL::Vector<Dakota::Real>;

} // namespace rol_interface

// Dakota-ROL Interface Includes
#include "BoolDispatch.hpp"
#include "DakotaROLOutputStreamFilter.hpp"
#include "DakotaROLVector.hpp"
#include "DakotaROLObjective.hpp"
#include "DakotaROLConstraint.hpp"
#include "DakotaROLOptimizer.hpp"

#endif // DAKOTA_ROL_INTERFACE_HPP
