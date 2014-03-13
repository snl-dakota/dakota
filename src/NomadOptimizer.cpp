/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2013, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NomadOptimizer
//- Description: Implementation of the NOMADOptimizer class, including
//		    derived subclasses used by the NOMAD Library.
//- Owner:       Patty Hough
//- Checked by:
//- Version: $Id$

#include <ProblemDescDB.hpp>

#include <algorithm>
#include <sstream>

#include <nomad.hpp>
#include "NomadOptimizer.hpp"


using namespace std;

namespace Dakota {

// Main Class: NomadOptimizer

NomadOptimizer::NomadOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model)
{     
     // load_parameters
     this->load_parameters(model);

     // Set Rnd Seed
     randomSeed = probDescDB.get_int("method.random_seed");
          
     // Set Max # of BB Evaluations
     maxBlackBoxEvals = probDescDB.get_int("method.max_function_evaluations");
          
     // STATS_FILE -- File Output    
     outputFormat = probDescDB.get_string("method.mesh_adaptive_search.display_format");

     // DISPLAY_ALL_EVAL -- If set, shows all evaluation points during Outputs, instead of just improvements
     displayAll = probDescDB.get_bool("method.mesh_adaptive_search.display_all_evaluations");
     
     epsilon = probDescDB.get_real("method.function_precision");
     
     maxIterations = probDescDB.get_int("method.max_iterations");
          
     // VNS = Variable Neighbor Search, it is used to escape local minima
     // if VNS >0.0, the NOMAD Parameter must be set with a Real number.
     vns = probDescDB.get_real("method.mesh_adaptive_search.variable_neighborhood_search");
          
     // Set the History File, which will contain all the evaluations history
     historyFile =  probDescDB.get_string("method.mesh_adaptive_search.history_file");     
}

NomadOptimizer::NomadOptimizer(Model& model):
  Optimizer(MESH_ADAPTIVE_SEARCH, model)
{
     // load_parameters
     this->load_parameters(model);
}

// Sub Class: Driver, this class runs the Mads algorithm using
// an Evaluator Object (See Above)
// This is the stuff that normally went into the main of the
// sample programs

void NomadOptimizer::find_optimum()
{
     
     // Set up Output (NOMAD::Display)
     NOMAD::Display out (std::cout);
     out.precision (NOMAD::DISPLAY_PRECISION_STD);
     
     // This is only used when using NOMAD with MPI
     NOMAD::begin ( 0 , NULL );  
     
     // Create parameters
     NOMAD::Parameters p (out);
     
     // Load Input Parameters
     p.set_DIMENSION(this->numTotalVars);
     
     vector<NOMAD::bb_output_type> bb_responses (1+numNomadNonlinearIneqConstraints+numNonlinearEqConstraints);
     for(int i=0;i<numContinuousVars;i++)
     {
	  p.set_BB_INPUT_TYPE(i,NOMAD::CONTINUOUS);
     }
     for(int i=0;i<numDiscreteIntVars+numDiscreteRealVars;i++)
     {
	  p.set_BB_INPUT_TYPE(i+numContinuousVars,NOMAD::INTEGER);
     }
     
     // Obtain response types

	  bb_responses[0] = NOMAD::OBJ;
     
     for(int i=0; i<this->numNomadNonlinearIneqConstraints;i++)
     {
	  bb_responses[i+1] = NOMAD::PB;
     }
     
     for(int i=0; i<this->numNonlinearEqConstraints;i++)
     {
	  bb_responses[i+1+this->numNomadNonlinearIneqConstraints] = NOMAD::EB;
     }
     
     // Set output and inputs.
     p.set_BB_OUTPUT_TYPE (bb_responses);

     p.set_X0 (this->initialPoint);
     p.set_LOWER_BOUND(this->lowerBound);
     p.set_UPPER_BOUND(this->upperBound);
     
     // Set Rnd Seed
     NOMAD::RNG::set_seed(randomSeed);
     
     // Set Max # of BB Evaluations
     p.set_MAX_BB_EVAL (maxBlackBoxEvals);
     
          
     // DISPLAY_STATS -- Standard Output
     // STATS_FILE -- File Output
     // These two format the information displayed using keywords to indicate
     // how the information is displayed in their respective output media.
     // Keywords:
     // BBE		BB Evaluation
     // BBO		BB Output
     // EVAL		Evaluations
     // MESH_INDEX	Mesh Index
     // MESH_SIZE	Mesh Size Parameter
     // OBJ		Objective Function Value
     // POLL_SIZE	Poll Size pParameter
     // SGTE		# of Surrogate Evals
     // SIM_BBE		Simulated BB evaluations (when using cache)
     // SOL		Solution, format is iSOLj, i and j are optional strings
     // STAT_AVG	AVG statistic, defined in BB_OUTPUT_TYPE
     // STAT_SUM	SUM statistic, defined in BB_OUTPUT_TYPE
     // TIME		Wall clock time
     // VARi 		Value of Variable i (0-based)
     
     p.set_DISPLAY_STATS(outputFormat);

     p.set_DISPLAY_ALL_EVAL(displayAll);
     
     p.set_EPSILON(epsilon);
     
     p.set_MAX_ITERATIONS(maxIterations);
     
     // VNS = Variable Neighbor Search, it is used to escape local minima
     // if VNS >0.0, the NOMAD Parameter must be set with a Real number.
     if(vns>0.0)
     {
	  p.set_VNS_SEARCH( NOMAD::Double(vns) );
     }
     
     // Set the History File, which will contain all the evaluations history
     p.set_HISTORY_FILE( historyFile );
          
     // Check the parameters -- Required by NOMAD for execution
     p.check();
     
     // Create Evaluator object
     Model& m = this->iteratedModel;
     NomadOptimizer::Evaluator ev (p,m);
     ev.set_constraint_map(numNomadNonlinearIneqConstraints, numNonlinearEqConstraints, constraintMapIndices, constraintMapMultipliers, constraintMapOffsets);
     
     // Create Algorithm ( NOMAD::Mads mads ( params, &evaluator ) )
     NOMAD::Mads mads (p,&ev);
     // Use Dakota's recast model for multi-objective for now.  Enable
     // NOMAD's native support for multi-objective later.
     NOMAD::Mads::set_flag_check_bimads(false);
 
     // Run!
     mads.run();

     // Retrieve best iterate and convert from NOMAD to DAKOTA
     // vector.

     const NOMAD::Eval_Point * bestX;
     bestX = mads.get_best_feasible();
     if (!bestX) {
       Cout << "WARNING: No feasible solution was found. "
	    << "Best point shown is best infeasible point.\n" << std::endl;
       bestX = mads.get_best_infeasible();
     }

     RealVector contVars(numContinuousVars);
     IntVector  discIntVars(numDiscreteIntVars);
     RealVector discRealVars(numDiscreteRealVars);

     const BitArray& int_set_bits = iteratedModel.discrete_int_sets();
     const IntSetArray& set_int_vars = iteratedModel.discrete_set_int_values();
     const RealSetArray& set_real_vars = iteratedModel.discrete_set_real_values();

     size_t j, dsi_cntr;

     for(j=0; j<numContinuousVars; j++)
     {
       contVars[j] = (*bestX)[j].value();
     }
     bestVariablesArray.front().continuous_variables(contVars);
     for(j=0, dsi_cntr=0; j<numDiscreteIntVars; j++)
     { 
       if (int_set_bits[j]) { // this active discrete int var is a set type
	 discIntVars[j] = set_index_to_value((*bestX)[j+numContinuousVars].value(), set_int_vars[dsi_cntr]);
	 ++dsi_cntr;
       }
       else  {                // this active discrete int var is a range type
	 discIntVars[j] = (*bestX)[j+numContinuousVars].value();
       }
     }
     bestVariablesArray.front().discrete_int_variables(discIntVars);
     for (j=0; j<numDiscreteRealVars; j++) {
       discRealVars = set_index_to_value((*bestX)[j+numContinuousVars+numDiscreteIntVars].value(), set_real_vars[j]);
     }
     bestVariablesArray.front().discrete_real_variables(discRealVars);

     // Retrieve the best responses and convert from NOMAD to
     // DAKOTA vector.h
     if (!localObjectiveRecast) {
       // else local_objective_recast_retrieve() is used in Optimizer::post_run()
       const NOMAD::Point & bestFs = bestX->get_bb_outputs();
       RealVector best_fns(numFunctions);
       std::vector<double> bestIneqs(constraintMapIndices.size()-numNonlinearEqConstraints);
       std::vector<double> bestEqs(numNonlinearEqConstraints);
       const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
       best_fns[0] = (!max_sense.empty() && max_sense[0]) ?
	 -bestFs[0].value() : bestFs[0].value();
       if (numNonlinearIneqConstraints > 0) {
	 for (int i=0; i<numNomadNonlinearIneqConstraints; i++) {
	   best_fns[constraintMapIndices[i]+1] = (bestFs[i+1].value() -
		    constraintMapOffsets[i]) / constraintMapMultipliers[i];
	 }
       }
       if (numNonlinearEqConstraints > 0) {
	 for (int i=0; i<numNonlinearEqConstraints; i++)
	   best_fns[constraintMapIndices[i+numNomadNonlinearIneqConstraints]+1] = 
	     (bestFs[i+numNomadNonlinearIneqConstraints+1].value() -
	      constraintMapOffsets[i+numNomadNonlinearIneqConstraints]) /
	     constraintMapMultipliers[i+numNomadNonlinearIneqConstraints];
       }
       bestResponseArray.front().function_values(best_fns);
     }

     // Stop NOMAD output.
     NOMAD::Slave::stop_slaves ( out );
     NOMAD::end();
}

// Sub Class: EvaluatorCreator, this class creates an Evaluator
// using a Dakota Model

// Sub Class: Evaluator, this class bridges NOMAD-style inputs with 
// DAKOTA style communication w/Black Box program.

     NomadOptimizer::Evaluator::Evaluator(const NOMAD::Parameters &p, Model& model) : NOMAD::Evaluator (p),_model(model)
     {
	  // Here we should take the parameters and get the problem information
	  // Like # of variables and types, # and types of constrs
	  /*
	  p.get_bb_nb_outputs(); // Get # of BB Outputs
	  p.get_bb_input_type(); // Returns a vector<NOMAD::bb_input_type>
	  p.get_bb_output_type(); // same, but with output_type
	  p.get_index_obj(); // get indices of obj fn, it's a list
	  */
	  // We can also access the parameters through _p
	  vector<NOMAD::bb_input_type> input_types;
	  input_types = _p.get_bb_input_type();
	  
	  n_cont = 0;
	  n_disc_int = 0;
	  n_disc_real = 0;

	  for(int i=0;i<input_types.size();i++)
	  {
	       if(input_types[i]==NOMAD::CONTINUOUS)
	       {
		    n_cont++;
	       }
	       //else if( (input_types[i]==NOMAD::INTEGER) || (input_types[i]==NOMAD::INTEGER) )
	       else
	       {
		    n_disc_int++; 
	       }
	  }
	  
     }
	  
     NomadOptimizer::Evaluator::~Evaluator(void){};
	  
     bool NomadOptimizer::Evaluator::eval_x ( NOMAD::Eval_Point &x,
	       const NOMAD::Double &h_max,
	       bool &count_eval) const
     {
	  int n_cont_vars = _model.cv();
	  int n_disc_int_vars = _model.div();
	  int n_disc_real_vars = _model.drv();
	  
          // Prepare Vectors for Dakota model.
          RealVector contVars(n_cont_vars);
          IntVector  discIntVars(n_disc_int_vars);
          RealVector discRealVars(n_disc_real_vars);

	  const BitArray& int_set_bits = _model.discrete_int_sets();
	  const IntSetArray&  set_int_vars = _model.discrete_set_int_values();
	  const RealSetArray& set_real_vars = _model.discrete_set_real_values();

	  const BoolDeque& sense = _model.primary_response_fn_sense();
	  bool max_flag = (!sense.empty() && sense[0]);

	  size_t i, dsi_cntr;

	  // Parameters contain Problem Information (variable: _p)
	  
	  // Here we are going to fetch the input information in x in a format that can be read by DAKOTA
	  // Transform values in x into something DAKOTA can read...
	  
	  // In the NOMAD parameters model:
	  // 1. Continuous Variables = NOMAD::CONTINUOUS
	  // 2. Discrete Integer Variables = NOMAD::INTEGER and NOMAD::BINARY
	  // 3. Discrete Real Variables = NOMAD::CATEGORICAL (??)
	 
	  // x.get_n() = # of BB Inputs
	  // x.get_m() = # of BB Outputs
	  for(i=0; i<n_cont_vars; i++)
	  {
	    _model.continuous_variable(x[i].value(), i);
	  }
	  for(i=0, dsi_cntr=0; i<n_disc_int_vars; i++)
	  { 
	    if (int_set_bits[i]) { // this active discrete int var is a set type
	      int dakota_value = set_index_to_value(x[i+n_cont_vars].value(), set_int_vars[dsi_cntr]);
	      _model.discrete_int_variable(dakota_value, i);
	      ++dsi_cntr;
	    }
	    else  {                // this active discrete int var is a range type
	      _model.discrete_int_variable(x[i+n_cont_vars].value(), i);
	    }
	  }
	  for (i=0; i<n_disc_real_vars; i++) {
	    Real dakota_value = set_index_to_value(x[i+n_cont_vars+n_disc_int_vars].value(), set_real_vars[i]);
	    _model.discrete_real_variable(dakota_value, i);
	  }
	  //	  _model.continuous_variables(contVars);
	  //	  _model.discrete_int_variables(discIntVars);
	  //	  _model.discrete_real_variables(discRealVars);

	  // Compute the Response using Dakota Interface
	  _model.compute_response();
	  // Obtain Model response
	  const RealVector& ftn_vals = _model.current_response().function_values();

	  Real obj_fcn = (max_flag) ? -ftn_vals[0] : ftn_vals[0];
	  x.set_bb_output  ( 0 , NOMAD::Double(obj_fcn) );

	  int numTotalConstr = numNomadNonlinearIneqConstr+numNomadNonlinearEqConstr;
	  for(int i=1;i<=numTotalConstr;i++)
	  {
	    Real constr_value = constrMapOffsets[i-1] +
	         constrMapMultipliers[i-1]*ftn_vals[constrMapIndices[i-1]+1];
	       x.set_bb_output  ( i , NOMAD::Double(constr_value)  );
	  }
	    
	  count_eval = true;
	  
	  return true;
	  
     }

void NomadOptimizer::load_parameters(Model &model)
{
     numTotalVars = numContinuousVars +
                    numDiscreteIntVars + numDiscreteRealVars;
    
     // Define Input Types and Bounds
     
     NOMAD::Point _initial_point (numTotalVars);
     NOMAD::Point _upper_bound (numTotalVars);
     NOMAD::Point _lower_bound (numTotalVars);
     
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

     size_t i, index, dsi_cntr;
    
     for(i=0;i<numContinuousVars;i++)
     {
	  _initial_point[i] = initial_point_cont[i];
	  if (lower_bound_cont[i] > -bigRealBoundSize)
	    _lower_bound[i] = lower_bound_cont[i];
	  else {
	    Cerr << "\nError: NomadOptimizer::load_parameters\n"
                 << "mesh_adaptive_search requires lower bounds "
                 << "for all continuous variables" << std::endl;
	    abort_handler(-1);
	  }
	  if (upper_bound_cont[i] < bigRealBoundSize)
	    _upper_bound[i] = upper_bound_cont[i];
	  else {
	    Cerr << "\nError: NomadOptimizer::load_parameters\n"
                 << "mesh_adaptive_search requires upper bounds "
                 << "for all continuous variables" << std::endl;
	    abort_handler(-1);
	  }
     }
     for(i=0, dsi_cntr=0; i<numDiscreteIntVars; i++)
     {
       if (int_set_bits[i]) {
	index = set_value_to_index(initial_point_int[i], initial_point_set_int[dsi_cntr]);
	if (index == _NPOS) {
	  Cerr << "\nError: failure in discrete integer set lookup within "
	       << "NomadOptimizer::load_parameters(Model &model)" << std::endl;
	  abort_handler(-1);
	}
	else {
	  _initial_point[i+numContinuousVars] = (int)index;
	  _lower_bound[i+numContinuousVars] = 0;
	  _upper_bound[i+numContinuousVars] = initial_point_set_int[dsi_cntr].size() - 1;
	}
	++dsi_cntr;
       }
       else {
 	 _initial_point[i+numContinuousVars] = initial_point_int[i];
	  if (lower_bound_int[i] > -bigIntBoundSize)
	    _lower_bound[i+numContinuousVars] = lower_bound_int[i];
	  else {
	    Cerr << "\nError: NomadOptimizer::load_parameters\n"
                 << "mesh_adaptive_search requires lower bounds "
                 << "for all discrete range variables" << std::endl;
	    abort_handler(-1);
	  }
	  if (upper_bound_int[i] < bigIntBoundSize)
	    _upper_bound[i+numContinuousVars] = upper_bound_int[i];
	  else {
	    Cerr << "\nError: NomadOptimizer::load_parameters\n"
                 << "mesh_adaptive_search requires upper bounds "
                 << "for all discrete range variables" << std::endl;
	    abort_handler(-1);
	  }
       }
     }
     for (i=0; i<numDiscreteRealVars; i++) {
       index = set_value_to_index(initial_point_real[i], initial_point_set_real[i]);
       if (index == _NPOS) {
	 Cerr << "\nError: failure in discrete real set lookup within "
	      << "NomadOptimizer::load_parameters(Model &model)" << std::endl;
	 abort_handler(-1);
       }
       else {
	 _initial_point[i+numContinuousVars+numDiscreteIntVars] = (int)index;
	 _lower_bound[i+numContinuousVars+numDiscreteIntVars] = 0;
	 _upper_bound[i+numContinuousVars+numDiscreteIntVars] = initial_point_set_real[i].size() - 1;
       }
     }
     
     initialPoint = _initial_point;
     lowerBound = _lower_bound;
     upperBound = _upper_bound;
     
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

  numNomadNonlinearIneqConstraints = 0;

  for (i=0; i<numNonlinearIneqConstraints; i++) {
    if (nln_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      numNomadNonlinearIneqConstraints++;
      constraintMapIndices.push_back(i);
      constraintMapMultipliers.push_back(-1.0);
      constraintMapOffsets.push_back(nln_ineq_lwr_bnds[i]);
    }
    if (nln_ineq_upr_bnds[i] < bigRealBoundSize) {
      numNomadNonlinearIneqConstraints++;
      constraintMapIndices.push_back(i);
      constraintMapMultipliers.push_back(1.0);
      constraintMapOffsets.push_back(-nln_ineq_upr_bnds[i]);
    }
  }

  for (i=0; i<numNonlinearEqConstraints; i++) {
    constraintMapIndices.push_back(i+numNonlinearIneqConstraints);
    constraintMapMultipliers.push_back(1.0);
    constraintMapOffsets.push_back(-nln_eq_targets[i]);
  }
}

NomadOptimizer::~NomadOptimizer() {};

}


