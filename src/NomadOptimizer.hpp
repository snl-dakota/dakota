/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2013, Sandia National Laboratories.
    This software is distributed under the GNU General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NomadOptimizer
//- Description: Declaration of wrapper class for NOMAD solver
//- Owner:       Patty Hough/John Siirola/Brian Adams
//- Checked by:
//- Version: $Id$


#include <DakotaOptimizer.hpp>
#include <ProblemDescDB.hpp>

#include <algorithm>
#include <sstream>

#include <nomad.hpp>

using namespace std;

// Forward Declarations
class NomadOptimizer;

namespace Dakota {

     /// Wrapper class for NOMAD Optimizer

     /**  NOMAD (is a Nonlinear Optimization by Mesh Adaptive Direct 
	  search) is a simulation-based optimization package designed
	  to efficiently explore a design space using Mesh Adaptive 
	  Search.
	  
	  Mesh Adaptive Direct Search uses Meshes, discretizations of
	  the domain space of variables. It generates multiple meshes,
	  and as its name implies, it also adapts the refinement of
	  the meshes in order to find the best solution of a problem.
	  
	  The objective of each iteration is to find points in a mesh
	  that improves the current solution. If a better solution is
	  not found, the next iteration is done over a finer mesh.
	  
	  Each iteration is composed of two steps: Search and Poll.
	  The Search step finds any point in the mesh in an attempt
	  to find an improvement; while the Poll step generates trial
	  mesh points surrounding the current best current solution.
	  
	  The NomadOptimizer is a wrapper for the NOMAD library. It
	  features the following attributes: \c max_function_evaluations,
	  \c display_format, \c display_all_evaluations, \c function_precision,
	  \c max_iterations.
     */

     class NomadOptimizer : public Optimizer
     {
	  public:
	       /// Constructor
	       /** NOMAD Optimizer Constructor
	       @param model DAKOTA Model object
	       */
	       NomadOptimizer(Model &model);
	       
	       NomadOptimizer(Model &model, int random_seed, int max_bb_evals, int max_iterations, 
			      Real _epsilon, Real _vns, bool display_all_evaluations,
			      string output_format, string history_file);
	       NomadOptimizer(const RealVector& lower_bound_disc, const RealVector& upper_bound_disc, 
		    const RealVector& lower_bound_cont, const RealVector& upper_bound_cont, 
		    const RealVector& initial_point_disc, const RealVector& initial_point_cont,
		    int num_cont, int num_disc, int num_functions,
		    int num_nonlinear_ineq_constraints, int num_nonlinear_eq_constraints,
		    int random_seed, int max_bb_evals, int max_iterations, 
		    Real _epsilon, Real _vns, bool display_all_evaluations,
		    string output_format, string history_file);	      
	       /// Destructor
               ~NomadOptimizer();
	       
	       //
	       //- Heading: Virtual member function redefinitions
	       //
	       
	       /// Calls the NOMAD solver
	       void find_optimum();
	  private:
	       // Forward Declaration
	       class Evaluator;
	       
	       /// Convenience function for Parameter loading.
	       /** This function takes the Parameters provided
	       by the user in the DAKOTA model.
	       @param model NOMAD Model object
	       */
	       void load_parameters(Model &model);
	       // Variables for the stuff that must go in
	       // the parameters.
	       // Will be filled by calling 
	       // load_parameters in the constructor,
	       // where we have access to the model.
	       int numCont,numDisc,numTotal;
	       int nObjFn, nIneqConstrs, nEqConstrs, nResponses;
	       
	       // Parameters.
	       int randomSeed, maxBlackBoxEvals, maxIterations;
	       string outputFormat, historyFile;
	       bool displayAll;
	       Real epsilon, vns;
	       
	       NOMAD::Point initialPoint;
	       NOMAD::Point upperBound;
	       NOMAD::Point lowerBound;
     };

     ///  NOMAD-based Evaluator class.

     /**  The NOMAD process requires an evaluation step, which
	  calls the Simulation program. In the simplest version
	  of this call, NOMAD executes the black box executable,
	  which proceeds to write a file in a NOMAD-compatible
	  format, which NOMAD reads to continue the process.
	  
	  Because DAKOTA files are different form NOMAD files,
	  and the simulations processed by DAKOTA already produce
	  DAKOTA-compatible files, we cannot use this method for
	  NOMAD. Instead, we implement the \c NomadEvaluator class,
	  which takes the NOMAD inputs and passes them to DAKOTA's
	  Interface for processing. The evaluator then passes
	  the evaluation Responses into the NOMAD objects for
	  further analysis.
     */

     class NomadOptimizer::Evaluator : public NOMAD::Evaluator {
	  private:
	       Model& _model;
	       int n_cont,n_disc_int, n_disc_real;
	  public:
	       /// Constructor
	       /** NOMAD Evaluator Constructor
	       @param p NOMAD Parameters object
	       @param model DAKOTA Model object
	       */
	       Evaluator(const NOMAD::Parameters &p, Model model);
	       
	       /// Destructor
	       ~Evaluator(void);
	       
	       /// Main Evaluation Method
	       /** Method that handles the communication between 
	       the NOMAD search process and the Black Box
	       Evaluation managed by DAKOTA's Interface.
	       @param x Object that contains the points that
			 need to evaluated. Once the evaluation
			 is completed, this object also stores
			 the output back to be read by NOMAD.
	       @param h_max Current value of the barrier
			      parameter. Not used in this
			      implementation.
	       @param count_eval Flag that indicates whether
				   this evaluation counts
				   towards the max number of
				   evaluations, often set to
				   \c false when the evaluation
				   does not meet certain costs
				   during expensive evaluations.
				   Not used in this
				   implementation.
	       @return \c true if the evaluation was successful;
			 \c false otherwise.
	       */
	       bool eval_x (NOMAD::Eval_Point &x,
		    const NOMAD::Double &h_max,
		    bool &count_eval) const;
	       
     } ;   
}
