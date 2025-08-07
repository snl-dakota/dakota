#pragma once
#ifndef ROL_OPTIMIZER_COMPATIBILITY_HPP
#define ROL_OPTIMIZER_COMPATIBILITY_HPP

#include "DakotaROLInterface.hpp"

namespace Dakota {

// Compatibility layer: Forward the old ROLOptimizer interface to our new rol_interface::Optimizer
class ROLOptimizer : public rol_interface::Optimizer {
public:
  /// Standard constructor
  ROLOptimizer(       ProblemDescDB&          problem_db, 
                      ParallelLibrary&        parallel_lib,
               const std::shared_ptr<Model>& model ) 
    : rol_interface::Optimizer(problem_db, model) {}

  /// Alternate constructor for Iterator instantiations by name
  ROLOptimizer( const String&                 method_string, 
               const std::shared_ptr<Model>& model ) 
    : rol_interface::Optimizer(method_string, model) {}

  /// Destructor
  virtual ~ROLOptimizer() = default;
};

} // namespace Dakota

#endif // ROL_OPTIMIZER_COMPATIBILITY_HPP