#pragma once
#ifndef DAKOTA_ROL_OPTIMIZER_HPP
#define DAKOTA_ROL_OPTIMIZER_HPP

#include <memory>

// Forward declarations (no Dakota includes here!)
#include <string>

namespace Dakota {
  class ProblemDescDB;
  class Model;
  using String = std::string;
}

namespace rol_interface {

// -----------------------------------------------------------------
/** Abstract interface for ROL optimizer. The actual implementation
    that inherits from Dakota::Optimizer lives in dakota_src to avoid
    circular dependencies. */

class OptimizerInterface {
public:

  /// Virtual destructor
  virtual ~OptimizerInterface() = default;

  /// Run the optimization
  virtual void core_run() = 0;

  /// Set ROL parameters from Dakota settings and optional XML file
  virtual void set_rol_parameters() = 0;

}; // class OptimizerInterface

// -----------------------------------------------------------------
/** Factory class for creating ROL optimizers. This uses the pIMPL
    idiom to break the circular dependency between rol_interface and
    dakota_src libraries. */

class OptimizerFactory {
public:

  /// Create optimizer from problem database
  static std::unique_ptr<OptimizerInterface> 
  create(       Dakota::ProblemDescDB&          problem_db, 
          const std::shared_ptr<Dakota::Model>& model );

  /// Create optimizer by method name
  static std::unique_ptr<OptimizerInterface> 
  create( const Dakota::String&                 method_name, 
          const std::shared_ptr<Dakota::Model>& model );

}; // class OptimizerFactory

// Legacy typedef for backward compatibility
using Optimizer = OptimizerInterface;

} // namespace rol_interface

#endif // DAKOTA_ROL_OPTIMIZER_HPP
