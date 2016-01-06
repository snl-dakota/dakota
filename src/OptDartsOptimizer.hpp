/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
       /// Constructor
       OptDartsOptimizer(ProblemDescDB& problem_db, Model& model);

       /// alternate constructor for Iterator instantiations by name
       OptDartsOptimizer(Model& model);
	       
       /// Destructor
       ~OptDartsOptimizer();
	       
	       
         //
         //- Heading: Virtual member function redefinitions
         //
	       
         /// Calls the OptDarts algorithm
         void core_run();
         
	  
     private:
	 
         // Forward Declaration
         // class Evaluator;
	       
	       
         /// Convenience function for Parameter loading.
         void load_parameters(Model &model);

         /// Function evaluation
         double opt_darts_f();

         /// Run the OPT-DARTS method
         void opt_darts_execute(size_t num_dim, size_t budget, double* xmin, double* xmax, double TOL, size_t problem_index, double fw_MC, double fb_MC);
         
         /// Initialize OPT-DARTS
         void opt_darts_initiate(double* xmin, double* xmax);
         
         void opt_darts_reset_convex_hull();
         
         /// Choose the next trial iterate
         size_t opt_darts_pick_candidate(size_t ifunc);
         
         void retrieve_extended_neighbors(size_t icandidate);
         
         void opt_darts_sample_from_candidate_neighborhood(size_t icandidate, size_t ifunc);
         
         void DIRECT_sample_from_candidate_neighborhood(size_t icandidate);
         
         void opt_darts_add_dart();
         
         void opt_darts_update_K_h_approximate_Voronoi(size_t isample);
         
         /// Release memory and exit cleanly
         void opt_darts_terminate();
         
         /// Convenience function for plotting iterates
         void opt_darts_plot_discs_2d(size_t icandidate);
         
         /// Convenience function for plotting convex hull
         void opt_darts_plot_hull_2d(size_t icandidate, size_t ifunc);

         /////////////////////////////////////////////////////////////////////////////////////////
	     /////////// General Methods /////////////////////////////////////////////////////////////
         /////////////////////////////////////////////////////////////////////////////////////////
         
         void initiate_random_generator(unsigned long x);
         
         double generate_a_random_number();
         
         void sample_uniformly_from_unit_sphere_surface(double* dart,      // A double array with size num_dim
                                                        size_t num_dim);   // Number of dimensions
         
         bool trim_line_using_Hyperplane(size_t num_dim,                                   // number of dimensions
                                         double* st, double *end,                          // line segmenet end points
                                         double* qH, double* nH) ;                         // a point on the hyperplane and it normal
         
         // opt darts variables
         double*  _xmin;         // lower left corner of domain
         double*  _xmax;         // upper right corner of domain
         double*  _dart;         // sample candidate
         double* _st;            // start of a line spoke
         double* _end;           // end of a line spoke
         double* _tmp_point;
         double* _qH;            // a point on a hyperplane
         double* _nH;            // normal vector of a hyperplane
         double** _x;            // coordinates of samples
         double** _xc;           // Candidate for each sample
         double** _f;            // function evaluations at samples -- a vector per sample point
         double** _K;            // lower bound estimate for local Lipschitz constant
         double*  _h;            // size of Voronoi cells around sample points;
         double*  _r;            // radius of ball around sample points

         size_t** _neighbors;    // Approximate Delaunay Neighbors
         size_t*  _tmp_neighbors;
         size_t*  _ext_neighbors;
         size_t   _num_ext_neighbors;
         
         bool _use_opt_darts;
         bool _estimate_K;
         
         size_t   _ib;           // index if best sample so far
         size_t   _num_samples;  // number of samples so far
         size_t   _budget;       // number of function evaluations
         size_t   _num_dim;      // number of dimensions
         double   _diag;           // diagonal of bounding box
         size_t   _problem_index;  // specifies which test problem we are using
         
         double  _fb;           // best function evaluations so far
         double  _fw;           // worst function evaluation so far
         double  _fval;         // latest function evaluation
         
         // need to expand to include various function levels
         size_t   _corner_index;
         size_t   _num_corners;      // number of corners
         size_t*  _corners;          // indexec of corner of convex hull
         
         double _epsilon;            // improvement factor
         
         double _fb_MC;
         double _fw_MC;
         
         double** _xm;
         double** _xp;
         
         double* _alpha_Deceptive;

         // variables for Random number generator
         double Q[1220];
         int indx;
         double cc;
         double c; /* current CSWB */
         double zc;	/* current SWB `borrow` */
         double zx;	/* SWB seed1 */
         double zy;	/* SWB seed2 */
         size_t qlen;/* length of Q array */
 
         // flag for DIRECT
         bool use_DIRECT;
         
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
