/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ROLOptimizer
//- Description: Implementation of the ROLOptimizer class, Only able to
//               call line search optimization for now (i.e. ROLOptimizer
//               is single-method TPL for this iterations)
//- Owner:       Moe Khalil
//- Checked by:
//- Version: $Id$

#include "ROL_OptimizationSolver.hpp"

#include "ROL_RandomVector.hpp"
#include "ROL_StdObjective.hpp"
#include "ROL_StdConstraint.hpp"
#include "ROL_Bounds.hpp"

#include "ROLOptimizer.hpp"
#include "ProblemDescDB.hpp"
using std::endl;

//
// - ROLOptimizer implementation
//

namespace Dakota {
 

 /* ROL Objectibe function class */

template<class Real> 
class ObjectiveF : public ROL::StdObjective<Real> {

public:

  ObjectiveF() {}

  Real value(const std::vector<Real> &x, Real &tol) {

    RealVector act_cont_vars(iteratedModel.cv(), false);

    size_t j;

    for(j=0; j<iteratedModel.cv(); j++){
      act_cont_vars[j] = x[j];
    }
    
    iteratedModel.continuous_variables(act_cont_vars);

    iteratedModel.evaluate();

    Real fn_val = iteratedModel.current_response().function_value(0);

    return fn_val;
  }

  // provide access to Dakota model
  void pass_model(Model& model) {
    iteratedModel = model;
  }

private:

  /// Total across all types of variables
  int numVars;

  /// Shallow copy of the model on which COLIN will iterate.
  Model iteratedModel;

}; // class ObjectiveF

/// Standard constructor.

ROLOptimizer::ROLOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model),
  iteratedModel(model)
{
  set_problem();
}

/// Alternate constructor for Iterator instantiations by name.

ROLOptimizer::
ROLOptimizer(const String& method_string, Model& model):
  Optimizer(method_string_to_enum(method_string), model),
  iteratedModel(model)
{
  set_problem();
}

// need to move functionality from core_run below here.
void ROLOptimizer::set_problem() {

  // typedef double RealT;
  // size_t j;

  // const RealVector& initial_points = iteratedModel.continuous_variables();

  // std::vector<RealT> x_stdv(iteratedModel.cv(),0.0);

  // for(j=0; j<iteratedModel.cv(); j++){
  //   x_stdv[j] = initial_points[j];
  // }

  // Teuchos::RCP<std::vector<RealT> > x_rcp = Teuchos::rcpFromRef( x_stdv );
  // // x_rcp = Teuchos::rcpFromRef( x_stdv );

  // Teuchos::RCP<ROL::Vector<RealT> > x  = Teuchos::rcp( new ROL::StdVector<RealT>(x_rcp) );



  // const RealVector& c_l_bnds = iteratedModel.continuous_lower_bounds();
  // const RealVector& c_u_bnds = iteratedModel.continuous_upper_bounds();

  // std::vector<RealT> l_stdv(iteratedModel.cv(),0.0);
  // std::vector<RealT> u_stdv(iteratedModel.cv(),0.0);

  // for(j=0; j<iteratedModel.cv(); j++){
  //   l_stdv[j] = c_l_bnds[j];
  //   u_stdv[j] = c_u_bnds[j];
  // }
    

  // Teuchos::RCP<std::vector<RealT> > l_rcp = Teuchos::rcpFromRef( l_stdv );
  // Teuchos::RCP<std::vector<RealT> > u_rcp = Teuchos::rcpFromRef( u_stdv );
    
  // Teuchos::RCP<ROL::Vector<RealT> > lower = Teuchos::rcp( new ROL::StdVector<RealT>( l_rcp ) );
  // Teuchos::RCP<ROL::Vector<RealT> > upper = Teuchos::rcp( new ROL::StdVector<RealT>( u_rcp ) );

  // Teuchos::RCP<ROL::BoundConstraint<RealT> > bnd  = Teuchos::rcp( new ROL::Bounds<RealT>(lower,upper) );

  //   ObjectiveF<RealT> obj_rcp = ObjectiveF<RealT>();
  //   obj_rcp.pass_model(iteratedModel);
  //   Teuchos::RCP<ROL::Objective<RealT> > obj  = Teuchos::rcpFromRef( obj_rcp );

 
  //   problem = ROL::OptimizationProblem<RealT> ( obj, x, bnd, Teuchos::null, Teuchos::null, Teuchos::null, Teuchos::null, Teuchos::null);  
}


/** core_run redefines the Optimizer virtual function to perform
    the optimization using ROL. It first sets up the simplified ROL
    problem data, then executes solve() on the simplified ROL
    solver interface and finally catalogues the results. */

void ROLOptimizer::core_run()
{
  typedef double RealT;
  size_t j;

  // create ROL variable vector
  const RealVector& initial_points = iteratedModel.continuous_variables();
  std::vector<RealT> x_stdv(iteratedModel.cv(),0.0);
  for(j=0; j<iteratedModel.cv(); j++){
    x_stdv[j] = initial_points[j];
  }
  Teuchos::RCP<std::vector<RealT> > x_rcp = Teuchos::rcpFromRef( x_stdv );
  Teuchos::RCP<ROL::Vector<RealT> > x  = Teuchos::rcp( new ROL::StdVector<RealT>(x_rcp) );

  // create ROL::BoundConstraint object to house variable bounds information
  const RealVector& c_l_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector& c_u_bnds = iteratedModel.continuous_upper_bounds();
  std::vector<RealT> l_stdv(iteratedModel.cv(),0.0);
  std::vector<RealT> u_stdv(iteratedModel.cv(),0.0);
  for(j=0; j<iteratedModel.cv(); j++){
    l_stdv[j] = c_l_bnds[j];
    u_stdv[j] = c_u_bnds[j];
  }
  Teuchos::RCP<std::vector<RealT> > l_rcp = Teuchos::rcpFromRef( l_stdv );
  Teuchos::RCP<std::vector<RealT> > u_rcp = Teuchos::rcpFromRef( u_stdv );
  Teuchos::RCP<ROL::Vector<RealT> > lower = Teuchos::rcp( new ROL::StdVector<RealT>( l_rcp ) );
  Teuchos::RCP<ROL::Vector<RealT> > upper = Teuchos::rcp( new ROL::StdVector<RealT>( u_rcp ) );
  Teuchos::RCP<ROL::BoundConstraint<RealT> > bnd  = Teuchos::rcp( new ROL::Bounds<RealT>(lower,upper) );

  // create objective function object and give it access to Dakota model 
  ObjectiveF<RealT> obj_rcp = ObjectiveF<RealT>();
  obj_rcp.pass_model(iteratedModel);
  Teuchos::RCP<ROL::Objective<RealT> > obj  = Teuchos::rcpFromRef( obj_rcp );

  // Call simplified interface problem generator
  ROL::OptimizationProblem<RealT> problem( obj, x, bnd, Teuchos::null, Teuchos::null, Teuchos::null, Teuchos::null, Teuchos::null);  




  // Print iterates to screen, need to control using Dakota output keyword
  Teuchos::RCP<std::ostream> outStream;
  outStream = Teuchos::rcp(&std::cout, false);

  // Extract method from iteratedModel and pass on to ROL::solver
  Teuchos::ParameterList parlist;
  parlist.sublist("Step").set("Type","Line Search");
  
  // Call simplified interface solver
  ROL::OptimizationSolver<RealT> solver( problem, parlist );
  solver.solve(*outStream); 

  // copy ROL solution to Dakota bestVariablesArray
  RealVector contVars(iteratedModel.cv());
  for(j=0; j<iteratedModel.cv(); j++){
    contVars[j] = (*x_rcp)[j];
  }
  bestVariablesArray.front().continuous_variables(contVars);

  // copy ROL objective to Dakota bestResponseArray
  RealVector best_fns(iteratedModel.num_functions());
  iteratedModel.continuous_variables(contVars);
  iteratedModel.evaluate();
  best_fns[0] = iteratedModel.current_response().function_value(0);
  bestResponseArray.front().function_values(best_fns);
}

} // namespace Dakota
