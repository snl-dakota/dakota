/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2013, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       OptDartsOptimizer
//- Description: Implementation of the OptDarts class
//- Owner:       Mohamed Ebeida 
//- Checked by:
//- Version: $Id$

#include "ProblemDescDB.hpp"

#include <algorithm>
#include <sstream>

#include "OptDartsOptimizer.hpp"


namespace Dakota {

// Main Class: OptDartsOptimizer

OptDartsOptimizer::OptDartsOptimizer(Model &model):Optimizer(model)
{     
     // load_parameters
     this->load_parameters(model);

     // Set Rnd Seed
     randomSeed = probDescDB.get_int("method.random_seed");
          
     // Set Max # of BB Evaluations
     maxBlackBoxEvals = probDescDB.get_int("method.max_function_evaluations");
          
     maxIterations = probDescDB.get_int("method.max_iterations");
}

OptDartsOptimizer::OptDartsOptimizer(NoDBBaseConstructor, Model& model):
  Optimizer(NoDBBaseConstructor(), model)
{
     // load_parameters
     this->load_parameters(model);
}


void OptDartsOptimizer::find_optimum()
{
       
     //at the end, you need to return the best variables and corresponding responses
     //see NOMADOptimizer as an example 
     // for example, 
     // for(j=0; j<numContinuousVars; j++)
     //{
     //  contVars[j] = (*bestX)[j].value();
     //}
     //bestVariablesArray.front().continuous_variables(contVars);

     // at the end, need to return the bestResponseArray with the best function values
     // bestResponseArray.front().function_values(best_fns);
     Cout << "In body of find_optimizer " << '\n';

}


void OptDartsOptimizer::load_parameters(Model &model)
{
     numTotalVars = numContinuousVars +
                    numDiscreteIntVars + numDiscreteRealVars;
    
     // Define Input Types and Bounds
     
     //NOMAD::Point _initial_point (numTotalVars);
     //NOMAD::Point _upper_bound (numTotalVars);
     //NOMAD::Point _lower_bound (numTotalVars);
     
     const RealVector& initial_point_cont = model.continuous_variables();
     const RealVector& lower_bound_cont = model.continuous_lower_bounds();
     const RealVector& upper_bound_cont = model.continuous_upper_bounds();

     const IntVector& initial_point_int = model.discrete_int_variables();
     const IntVector& lower_bound_int = model.discrete_int_lower_bounds();
     const IntVector& upper_bound_int = model.discrete_int_upper_bounds();

     const RealVector& initial_point_real = model.discrete_real_variables();
     const RealVector& lower_bound_real = model.discrete_real_lower_bounds();
     const RealVector& upper_bound_real = model.discrete_real_upper_bounds();

     const BitArray& int_set_bits = iteratedModel.discrete_int_sets();
     const IntSetArray& initial_point_set_int = iteratedModel.discrete_set_int_values();
     const RealSetArray& initial_point_set_real = iteratedModel.discrete_set_real_values();

     // Define Output Types
     // responses.
     //		objective_functions
     //		nonlinear_inequality_constraints
     //		nonlinear_equality_constraints

     const RealVector& nln_ineq_lwr_bnds
       = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
     const RealVector& nln_ineq_upr_bnds
       = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
     const RealVector& nln_eq_targets
       = iteratedModel.nonlinear_eq_constraint_targets();

}

OptDartsOptimizer::~OptDartsOptimizer() {};

}


