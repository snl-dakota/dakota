#pragma once
#ifndef DAKOTA_ROL_OPTIMIZER_HPP
#define DAKOTA_ROL_OPTIMIZER_HPP

namespace rol_interface {

// -----------------------------------------------------------------
/** Optimizer specializes Dakota::Optimizer to construct and run a
    ROL solver appropriate for the type of problem specified by the
    user. */

class Optimizer : public Dakota::Optimizer {
public:

  /// Standard constructor
  Optimizer( Dakota::ProblemDescDB& problem_db, 
                Dakota::Model&         model );

  /// Alternate constructor for Iterator instantiations by name
  Optimizer( const Dakota::String& method_name, 
                      Dakota::Model&  model );
  
  /// Destructor
  virtual ~Optimizer() = default;

  void core_run() override;

  inline std::shared_ptr<Dakota::Optimizer> 
  create( Dakota::ProblemDescDB& problem_db, 
          Dakota::Model&         model ) {
    return std::make_shared<Optimizer>(problem_db, model);
  }

  friend class Initializer;

private:

  ROL::Ptr<ROL::Problem<Dakota::Real>> problem;
  ROL::Ptr<Cache> modelCache;
  ROL::ParameterList parList;	
  // -----------------------------------------------------------------
  /** Initializer is a helper class used to initialize Optimizer. */

  class Initializer {

    static void initialize( Optimizer* opt );
    static void set_default_parameters( Optimizer* opt );

    friend class Optimizer;

  }; // class ROLInitializer



}; // class Optimizer

} // namespace rol_interface

#endif // DAKOTA_ROL_OPTIMIZER_HPP
