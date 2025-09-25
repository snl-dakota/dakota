#include "DakotaROLOptimizer.hpp"
#include <stdexcept>

namespace rol_interface {

// Factory methods - these will be implemented in dakota_src where
// the actual Dakota dependencies are available.
std::unique_ptr<OptimizerInterface> 
OptimizerFactory::create(Dakota::ProblemDescDB&          problem_db, 
                        const std::shared_ptr<Dakota::Model>& model) {
  // This is a stub that will be replaced by the actual implementation
  // in dakota_src. For now, throw an error to indicate this is not implemented.
  throw std::runtime_error("OptimizerFactory::create() must be implemented in dakota_src");
}

std::unique_ptr<OptimizerInterface> 
OptimizerFactory::create(const Dakota::String&                 method_name, 
                        const std::shared_ptr<Dakota::Model>& model) {
  // This is a stub that will be replaced by the actual implementation
  // in dakota_src. For now, throw an error to indicate this is not implemented.
  throw std::runtime_error("OptimizerFactory::create() must be implemented in dakota_src");
}

} // namespace rol_interface

