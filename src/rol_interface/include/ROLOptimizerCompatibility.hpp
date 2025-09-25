#pragma once
#ifndef ROL_OPTIMIZER_COMPATIBILITY_HPP
#define ROL_OPTIMIZER_COMPATIBILITY_HPP

#include "DakotaROLOptimizer.hpp"
#include <memory>

// Forward declarations
namespace Dakota {
  class ProblemDescDB;
  class Model;
  class ParallelLibrary;
  using String = std::string;
}

namespace Dakota {

// Compatibility layer: Wrapper around the new factory-based ROL interface
// This provides backward compatibility for existing code that expects ROLOptimizer
class ROLOptimizer {
public:
  /// Standard constructor
  ROLOptimizer(       ProblemDescDB&          problem_db, 
                      ParallelLibrary&        parallel_lib,
               const std::shared_ptr<Model>& model ) 
    : impl(rol_interface::OptimizerFactory::create(problem_db, model)) {}

  /// Alternate constructor for Iterator instantiations by name
  ROLOptimizer( const String&                 method_string, 
                const std::shared_ptr<Model>& model ) 
    : impl(rol_interface::OptimizerFactory::create(method_string, model)) {}

  /// Destructor
  virtual ~ROLOptimizer() = default;

  /// Run the optimization (delegate to implementation)
  void core_run() { impl->core_run(); }

  /// Set ROL parameters (delegate to implementation)
  void set_rol_parameters() { impl->set_rol_parameters(); }

private:
  std::unique_ptr<rol_interface::OptimizerInterface> impl;
};

} // namespace Dakota

#endif // ROL_OPTIMIZER_COMPATIBILITY_HPP
