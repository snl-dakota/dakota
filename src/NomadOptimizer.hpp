/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NomadOptimizer
//- Description: Declaration of wrapper class for NOMAD solver
//- Owner:       Patty Hough
//- Checked by:
//- Version: $Id$


#include <DakotaOptimizer.hpp>
#include <ProblemDescDB.hpp>

#include <algorithm>
#include <sstream>

#include <nomad.hpp>

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
  NomadOptimizer(ProblemDescDB& problem_db, Model &model);

  /// alternate constructor for Iterator instantiations without DB
  NomadOptimizer(Model& model);
	       
  /// Destructor
  ~NomadOptimizer();
	       
  //
  //- Heading: Virtual member function redefinitions
  //
	       
  /// Calls the NOMAD solver
  void core_run();

private:
  
  // Forward Declaration
  class Evaluator;
  class Extended_Poll;
	       
  /// Convenience function for Parameter loading.
  /** This function takes the Parameters provided by the user 
      in the DAKOTA model.
        @param model NOMAD Model object
      Variables for the stuff that must go in the parameters.
      Will be filled by calling load_parameters after the
      constructor to capture model recasts.
  */
  void load_parameters(Model &model, NOMAD::Parameters &p);

  /// Total across all types of variables
  int numTotalVars;

  /// Number of nonlinear inequality constraints after
  /// put into the format required by NOMAD
  int numNomadNonlinearIneqConstraints;
	       
  /// Algorithm control parameters passed to NOMAD
  int randomSeed, maxBlackBoxEvals, maxIterations;
  Real initMesh, minMesh, epsilon, vns;

  /// Output control parameters passed to NOMAD
  std::string outputFormat, historyFile;
  bool displayAll;

  /// Parameters needed for categorical neighbor construction
  int numHops;
  BitArray discreteSetIntCat, discreteSetRealCat;
  RealMatrixArray discreteSetIntAdj, discreteSetRealAdj, discreteSetStrAdj;
  RealMatrixArray categoricalAdjacency;       
	       
  /// Pointer to Nomad initial point
  NOMAD::Point initialPoint;

  /// Pointer to Nomad upper bounds
  NOMAD::Point upperBound;

  /// Pointer to Nomad lower bounds
  NOMAD::Point lowerBound;

  /// map from Dakota constraint number to Nomad constraint number
  std::vector<int> constraintMapIndices;

  /// multipliers for constraint transformations
  std::vector<double> constraintMapMultipliers;

  /// offsets for constraint transformations
  std::vector<double> constraintMapOffsets;

  /// defines use of surrogate in NOMAD
  std::string useSurrogate;
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

class NomadOptimizer::Evaluator : public NOMAD::Evaluator
{
private:

  Model& _model;
  int n_cont,n_disc_int, n_disc_real;

  /// Number of nonlinear constraints after put into Nomad format
  int numNomadNonlinearIneqConstr, numNomadNonlinearEqConstr;

  /// map from Dakota constraint number to Nomad constraint number
  std::vector<int> constrMapIndices;

  /// multipliers for constraint transformations
  std::vector<double> constrMapMultipliers;

  /// offsets for constraint transformations
  std::vector<double> constrMapOffsets;

  /// defines use of surrogate in NOMAD
  std::string useSgte;

public:

  /// Constructor
  /** NOMAD Evaluator Constructor
        @param p NOMAD Parameters object
	@param model DAKOTA Model object
  */
  Evaluator(const NOMAD::Parameters &p, Model& model);
	       
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
	       
  /// publishes constraint transformation
  void set_constraint_map (int numNomadNonlinearIneqConstraints,
			   int numNomadNonlinearEqConstraints,
			   std::vector<int> constraintMapIndices,
			   std::vector<double> constraintMapMultipliers,
			   std::vector<double> constraintMapOffsets)
  { numNomadNonlinearIneqConstr = numNomadNonlinearIneqConstraints;
    numNomadNonlinearEqConstr = numNomadNonlinearEqConstraints,
      constrMapIndices = constraintMapIndices;
    constrMapMultipliers = constraintMapMultipliers;
    constrMapOffsets = constraintMapOffsets;}

  /// publishes surrogate usage
  void set_surrogate_usage (std::string useSurrogate)
  { useSgte = useSurrogate;}
};

class NomadOptimizer::Extended_Poll : public NOMAD::Extended_Poll
{
private:

  RealMatrixArray &adjacency_matrix;
  int nHops;

public:

  /// Constructor
  Extended_Poll(NOMAD::Parameters &p, RealMatrixArray &categoricalAdjacency,
		int numHops) : NOMAD::Extended_Poll(p),
			       adjacency_matrix(categoricalAdjacency),
			       nHops(numHops) {};

  /// Destructor
  ~Extended_Poll(void){};

  /// Construct the extended poll points.  Called by NOMAD.
  void construct_extended_points(const NOMAD::Eval_Point &nomad_point);

  /// Recursive helper function to construct the multi-hop neighbors
  /// derived from adjacency matrices and returned to
  /// construct_extended_points.
  void construct_multihop_neighbors(NOMAD::Point &base_point,
				    NOMAD::Signature point_signature,
				    RealMatrixArray::iterator rma_iter,
				    size_t last_cat_index, int num_hops);
};

}  // namespace DAKOTA
