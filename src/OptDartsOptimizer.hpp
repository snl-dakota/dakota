/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2013, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       OptDartsOptimizer
//- Description: Implementation of OptDarts optimizer 
//- Owner:       Mohamed Ebeida and Laura Swiler
//- Checked by:
//- Version: $Id$


#include "DakotaOptimizer.hpp"
#include "ProblemDescDB.hpp"

#include <algorithm>
#include <sstream>

// Forward Declarations
class OptDartsOptimizer;

namespace Dakota {

     /// Wrapper class for OptDarts Optimizer


     class OptDartsOptimizer : public Optimizer
     {
	  public:
               /// standard constructor
	       OptDartsOptimizer(ProblemDescDB& problem_db, Model& model);

               /// alternate constructor for Iterator instantiations by name
               OptDartsOptimizer(Model& model);
	       
	       /// Destructor
               ~OptDartsOptimizer();
	       
	       //
	       //- Heading: Virtual member function redefinitions
	       //
	       
	       /// Calls the OptDarts algorithm 
	       void find_optimum();
	  private:
	       // Forward Declaration
	       //class Evaluator;
	       
	       /// Convenience function for Parameter loading.
	       void load_parameters(Model &model);
	       // Variables for the stuff that must go in
	       // the parameters.
	       // Will be filled by calling 
	       // load_parameters in the constructor,
	       // where we have access to the model.
               int numTotalVars;
	       
	       // Parameters.
	       int randomSeed, maxBlackBoxEvals, maxIterations;
	       
     }; // class OptDartsOptimizer
}
