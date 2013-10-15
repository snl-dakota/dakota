/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2013, Sandia National Laboratories.
    This software is distributed under the GNU General Public License.
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

// Sub Class: EvaluatorCreator, this class creates an Evaluator
// using a Dakota Model

// Sub Class: Evaluator, this class bridges NOMAD-style inputs with 
// DAKOTA style communication w/Black Box program.

     NomadOptimizer::Evaluator::Evaluator(const NOMAD::Parameters &p, Model model) : NOMAD::Evaluator (p),_model(model)
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
	  // Prepare Vectors for Dakota model.
	  RealVector contVars;
	  IntVector  discIntVars;
	  RealVector discRealVars;
	  vector<NOMAD::bb_input_type> input_types;
	  
	  // Parameters contain Problem Information (variable: _p)
	  
	  // Here we are going to fetch the input information in x in a format that can be read by DAKOTA
	  // Transform values in x into something DAKOTA can read...
	  
	  // In the NOMAD parameters model:
	  // 1. Continuous Variables = NOMAD::CONTINUOUS
	  // 2. Discrete Integer Variables = NOMAD::INTEGER and NOMAD::BINARY
	  // 3. Discrete Real Variables = NOMAD::CATEGORICAL (??)
	  
	  // x.get_n() = # of BB Inputs
	  // x.get_m() = # of BB Outputs
	  
	  input_types = _p.get_bb_input_type();
	  
	  contVars.resize(n_cont);
	  discIntVars.resize(n_disc_int);
	  discRealVars.resize(n_disc_real);
	  
	  for(int i=0;i<x.get_n();i++)
	  {
	       if(input_types[i]==NOMAD::CONTINUOUS)
	       {
		    contVars[i] = x[i].value();
	       }
	       //else if( (input_types[i]==NOMAD::INTEGER) || (input_types[i]==NOMAD::INTEGER) )
	       else
	       {
		    discIntVars[i-n_cont] = x[i].value();
	       }
	       
	  }
	  
	  _model.continuous_variables(contVars);
	  _model.discrete_int_variables(discIntVars);
	  _model.discrete_real_variables(discRealVars);
	  
	  // Compute the Response using Dakota Interface
	  _model.compute_response();
	  
	  // Obtain Model response
	  const RealVector& ftn_vals = _model.current_response().function_values();
	  
	  for(int i=0;i<ftn_vals.length();i++)
	  {
	       x.set_bb_output  ( i , NOMAD::Double(ftn_vals[i])  );
	  }
	    
	  count_eval = true;
	  
	  return true;
	  
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
     // NOMAD::begin ( argc , argv );  
     
     // Create parameters
     NOMAD::Parameters p (out);
     
     // Load Input Parameters
     cout << this->numTotal << "\n";
     p.set_DIMENSION(this->numTotal);
     
     vector<NOMAD::bb_output_type> bb_responses (nResponses);
     
     for(int i=0;i<this->numCont;i++)
     {
	  p.set_BB_INPUT_TYPE(i,NOMAD::CONTINUOUS);
     }
     for(int i=0;i<this->numDisc;i++)
     {
	  p.set_BB_INPUT_TYPE(i+this->numCont,NOMAD::INTEGER);
     }
     
     // Obtain response types
     for(int i=0; i<this->nObjFn;i++)
     {
	  bb_responses[i] = NOMAD::OBJ;
     }
     
     for(int i=0; i<this->nIneqConstrs;i++)
     {
	  bb_responses[i+this->nObjFn] = NOMAD::PB;
     }
     
     for(int i=0; i<this->nEqConstrs;i++)
     {
	  bb_responses[i+this->nObjFn+nIneqConstrs] = NOMAD::EB;
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
     
     // Create Algorithm ( NOMAD::Mads mads ( params, &evaluator ) )
     NOMAD::Mads mads (p,&ev);
     
     // Run!
     mads.run();
     
     // Stop NOMAD output.
     NOMAD::Slave::stop_slaves ( out );
     NOMAD::end();

     // Retrieve best iterate and convert from NOMAD to DAKOTA
     // vector.

     const NOMAD::Eval_Point * bestX = mads.get_best_feasible();
     RealVector contVars(numCont);
     IntVector  discIntVars(numDisc);
     vector<NOMAD::bb_input_type> input_types;
     input_types = p.get_bb_input_type();
     for(int i=0;i<numTotal;i++)
       {
	 if(input_types[i]==NOMAD::CONTINUOUS)
	   {
	     contVars[i] = (*bestX)[i].value();
	   }
	 else
	   {
	     discIntVars[i-numCont] = (*bestX)[i].value();
	   }
	       
       }
     bestVariablesArray.front().continuous_variables(contVars);
     bestVariablesArray.front().discrete_int_variables(discIntVars);

     // Retrieve the best responses and convert from NOMAD to
     // DAKOTA vector.h
     if (!localObjectiveRecast) {
       // else local_objective_recast_retrieve() is used in Optimizer::post_run()

       RealVector best_fns(numFunctions);
       const NOMAD::Point & bestFs = bestX->get_bb_outputs();
       const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
       best_fns[0] = (!max_sense.empty() && max_sense[0]) ?
	 -bestFs[0].value() : bestFs[0].value();
       for (int i=1; i<numFunctions; i++)
	 best_fns[i] = bestFs[i].value();
       bestResponseArray.front().function_values(best_fns);
     }
}

void NomadOptimizer::load_parameters(Model &model)
{
     // Use probDescDB
     //n_cont = probDescDB.get_int("variables.continuous_design");
     //n_disc = probDescDB.get_int("variables.discrete_design_range");
     //n_total = n_cont + n_disc;
     
     numCont = model.cv();
     numDisc = model.div();
     numTotal = numCont + numDisc;
     
     // Define Input Types and Bounds
     
     NOMAD::Point _initial_point (numTotal);
     NOMAD::Point _upper_bound (numTotal);
     NOMAD::Point _lower_bound (numTotal);

     const IntVector& initial_point_disc = model.discrete_int_variables();
     const IntVector& lower_bound_disc = model.discrete_int_lower_bounds();
     const IntVector& upper_bound_disc = model.discrete_int_upper_bounds();
     
     const RealVector& initial_point_cont = model.continuous_variables();
     const RealVector& lower_bound_cont = model.continuous_lower_bounds();
     const RealVector& upper_bound_cont = model.continuous_upper_bounds();
     
     // Define Output Types
     // responses.
     //		objective_functions
     //		nonlinear_inequality_constraints
     //		nonlinear_equality_constraints
     
     nIneqConstrs = model.num_nonlinear_ineq_constraints();
     nEqConstrs = model.num_nonlinear_eq_constraints();
     nResponses = model.num_functions();
     nObjFn = nResponses-nIneqConstrs-nEqConstrs;
     
     for(int i=0;i<numCont;i++)
     {
	  _initial_point[i] = initial_point_cont[i];
	  _lower_bound[i] = lower_bound_cont[i];
	  _upper_bound[i] = upper_bound_cont[i];
     }
     for(int i=0;i<numDisc;i++)
     {
	  _initial_point[i+numCont] = initial_point_disc[i];
	  _lower_bound[i+numCont] = lower_bound_disc[i];
	  _upper_bound[i+numCont] = upper_bound_disc[i];
     }
     
     initialPoint = _initial_point;
     lowerBound = _lower_bound;
     upperBound = _upper_bound;
}

NomadOptimizer::NomadOptimizer(Model &model):Optimizer(model)
{
     // load_parameters
     this->load_parameters(model);
     
     probDescDB.unlock();
     
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
     
     probDescDB.lock();
}

NomadOptimizer::NomadOptimizer(Model &model, int random_seed, int max_bb_evals, int max_iterations, 
			      Real _epsilon, Real _vns, bool display_all_evaluations,
			      string output_format, string history_file)
{
     // load_parameters
     this->load_parameters(model);
     
     // Set Rnd Seed
     randomSeed = random_seed;
          
     // Set Max # of BB Evaluations
     maxBlackBoxEvals = max_bb_evals;
          
     // STATS_FILE -- File Output    
     outputFormat = output_format;

     // DISPLAY_ALL_EVAL -- If set, shows all evaluation points during Outputs, instead of just improvements
     displayAll = display_all_evaluations;
     
     epsilon = _epsilon;
     
     maxIterations = max_iterations;
          
     // VNS = Variable Neighbor Search, it is used to escape local minima
     // if VNS >0.0, the NOMAD Parameter must be set with a Real number.
     vns = _vns;
          
     // Set the History File, which will contain all the evaluations history
     historyFile =  history_file;
}

NomadOptimizer::NomadOptimizer(const RealVector& lower_bound_disc, const RealVector& upper_bound_disc, 
	       const RealVector& lower_bound_cont, const RealVector& upper_bound_cont, 
	       const RealVector& initial_point_disc, const RealVector& initial_point_cont,
	       int num_cont, int num_disc, int num_functions,
	       int num_nonlinear_ineq_constraints, int num_nonlinear_eq_constraints,
	       int random_seed, int max_bb_evals, int max_iterations, 
	       Real _epsilon, Real _vns, bool display_all_evaluations,
	       string output_format, string history_file)
{
     numCont = num_cont;
     numDisc = num_disc;
     numTotal = numCont + numDisc;
     
     // Define Input Types and Bounds
     
     NOMAD::Point _initial_point (numTotal);
     NOMAD::Point _upper_bound (numTotal);
     NOMAD::Point _lower_bound (numTotal);
     
     // Define Output Types
     // responses.
     //		objective_functions
     //		nonlinear_inequality_constraints
     //		nonlinear_equality_constraints
     
     nIneqConstrs = num_nonlinear_ineq_constraints;
     nEqConstrs = num_nonlinear_eq_constraints;
     nResponses = num_functions;
     nObjFn = nResponses-nIneqConstrs-nEqConstrs;
     
     for(int i=0;i<numCont;i++)
     {
	  _initial_point[i] = initial_point_cont[i];
	  _lower_bound[i] = lower_bound_cont[i];
	  _upper_bound[i] = upper_bound_cont[i];
     }
     for(int i=0;i<numDisc;i++)
     {
	  _initial_point[i+numCont] = initial_point_disc[i];
	  _lower_bound[i+numCont] = lower_bound_disc[i];
	  _upper_bound[i+numCont] = upper_bound_disc[i];
     }
     
     initialPoint = _initial_point;
     lowerBound = _lower_bound;
     upperBound = _upper_bound;
     
     // Set Rnd Seed
     randomSeed = random_seed;
          
     // Set Max # of BB Evaluations
     maxBlackBoxEvals = max_bb_evals;
          
     // STATS_FILE -- File Output    
     outputFormat = output_format;

     // DISPLAY_ALL_EVAL -- If set, shows all evaluation points during Outputs, instead of just improvements
     displayAll = display_all_evaluations;
     
     epsilon = _epsilon;
     
     maxIterations = max_iterations;
          
     // VNS = Variable Neighbor Search, it is used to escape local minima
     // if VNS >0.0, the NOMAD Parameter must be set with a Real number.
     vns = _vns;
          
     // Set the History File, which will contain all the evaluations history
     historyFile =  history_file;
}

NomadOptimizer::~NomadOptimizer() {};

}


