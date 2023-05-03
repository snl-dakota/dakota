#include "DakotaROLInterface.hpp"
 
namespace rol_interface {

// Standard constructor for Optimizer.  Sets up ROL solver based on
// information from the problem database.
Optimizer::Optimizer( Dakota::ProblemDescDB& problem_db, 
                            Dakota::Model&   model) :
  Dakota::Optimizer( problem_db, 
                     model, 
                     Traits::create() ),
  modelInterface(ROL::makePtr<ModelInterface>(this)) {
} // Standard constructor  


// Alternate constructor for Iterator instantiations by name.  Sets up
// ROL solver based on information passed as arguments.
Optimizer::Optimizer( const Dakota::String& method_string, 
                            Model&          model):
  Dakota::Optimizer( Dakota::method_string_to_enum(method_string), 
                     model, 
                     Traits::create() ),
  modelInterface(ROL::makePtr<ModelInterface>(this)) {
} // alternate constructor


// core_run redefines the Optimizer virtual function to perform the
// optimization using ROL and catalogue the results.
void Optimizer::core_run() {

  { // Prepend output in this section with "ROL: "
    auto rol_cout = OutputStreamFilter(Cout); 
    auto opt_solver = ROL::Solver<Dakota::Real>( problem, parList );
    opt_solver.solve(rol_cout.stream());
  } 

  auto& best_vars = bestVariablesArray.front();
  auto& x_opt = get_vector(problem->getPrimalOptimizationVector());
  x_opt = best_vars.continuous_variables_view();

} // core_run


} // namespace rol_interface

