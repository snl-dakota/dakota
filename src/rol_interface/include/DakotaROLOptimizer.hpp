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
  Optimizer(       Dakota::ProblemDescDB&          problem_db, 
             const std::shared_ptr<Dakota::Model>& model );

  /// Alternate constructor for Iterator instantiations by name
  Optimizer( const Dakota::String& method_name, 
             const std::shared_ptr<Dakota::Model>& model );
  
  /// Destructor
  virtual ~Optimizer() = default;

  void core_run() override;

  static std::shared_ptr<Optimizer> 
  create(       Dakota::ProblemDescDB&          problem_db, 
          const std::shared_ptr<Dakota::Model>& model );

  static std::shared_ptr<Optimizer> 
  create ( const Dakota::String& method_name, 
           const std::shared_ptr<Dakota::Model>& model );
 

private:
  ROL::Ptr<ROL::Problem<Dakota::Real>> problem;
  ROL::ParameterList parList;	

  

  // -----------------------------------------------------------------
  /** Initializer is a helper class used to initialize Optimizer. */

//  class Initializer {
//
//    static void initialize( Optimizer* opt );
//    static void set_default_parameters( Optimizer* opt );
//
//    friend class Optimizer;
//
//  }; // class ROLInitializer



}; // class Optimizer

} // namespace rol_interface

#endif // DAKOTA_ROL_OPTIMIZER_HPP
