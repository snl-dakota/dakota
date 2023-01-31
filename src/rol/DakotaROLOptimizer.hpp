#pragma once
#ifndef DAKOTA_ROL_OPTIMIZER_HPP
#define DAKOTA_ROL_OPTIMIZER_HPP

namespace Dakota {
namespace rol_interface {

// -----------------------------------------------------------------
/** ROLOptimizer specializes Dakota::Optimizer to construct and run a
    ROL solver appropriate for the type of problem specified by the
    user. */

class ROLOptimizer : public Optimizer {
public:

  /// Standard constructor
  ROLOptimizer( Dakota::ProblemDescDB& problem_db, 
                Dakota::Model&         model );

  /// Alternate constructor for Iterator instantiations by name
  ROLOptimizer( const Dakota::String& method_name, 
                      Dakota::Model&  model );
  
  /// Destructor
  ~ROLOptimizer() = default;

  friend class Initializer;

private:

  ROL::Ptr<ROL::Problem<Real>> problem;
  ROL::Ptr<Cache> modelCache;

  // -----------------------------------------------------------------
  /** Initializer is a helper class used to initialize ROLOptimizer. */

  class Initializer {

    static void initialize( ROLOptimizer& opt );

    friend class ROLOptimizer;

  }; // class ROLInitializer



}; // class ROLOptimizer

} // namespace rol_interface
} // namespace Dakota

#endif // DAKOTA_ROL_OPTIMIZER_HPP
