/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 VPSApproximation
//- Description: Class implementation of Voronoi Piecewise Surrogate Approximation
//- Owner:       Mohamed Ebeida and Ahmad Rushdi
//- Checked by:
//- Version:

#ifndef VPS_APPROXIMATION_H
#define VPS_APPROXIMATION_H

#include "dakota_data_types.hpp"
#include "DakotaVariables.hpp"
#include "DakotaApproximation.hpp"
#include "SharedSurfpackApproxData.hpp"
#include "pecos_data_types.hpp" // to identify SDVArrays and SDRArrays

using namespace Pecos;

namespace Dakota
{
    class ProblemDescDB;
    
    /// Derived approximation class for VPS implementation

    /** The VPSApproximation class provides a set of piecewise surrogate approximations
    each of which is valid within a Voronoi cell.  */

    class VPSApproximation: public Approximation
    {

    public:

        //
        //- Heading: Constructors and destructor
        //

        /// default constructor
        VPSApproximation();

        /// standard constructor (to call VPS from an input deck)
        VPSApproximation(const ProblemDescDB& problem_db,
                         const SharedApproxData& shared_data,
                         const String& approx_label);
    
        /// Alternate constructor (to call VPS from another method like POF-darts)
        VPSApproximation(const SharedApproxData& shared_data);
  
        /// destructor
        ~VPSApproximation();


    
        //////////////////////////////////////////////////////////////
        // VPS METHODS
        //////////////////////////////////////////////////////////////
    
        bool VPS_execute();
        
        void VPS_create_containers();
        
        void VPS_retrieve_neighbors(size_t ipoint, bool update_point_neighbors);
        
        void VPS_adjust_extend_neighbors_of_all_points();
        
        void VPS_extend_neighbors(size_t ipoint);
        
        void VPS_build_local_surrogate(size_t cell_index);
        
        double VPS_evaluate_surrogate(double* x);
        

        void VPS_destroy_global_containers();
        

        //////////////////////////////////////////////////////////////
        // Least Square Sub Surrogate METHODS
        //////////////////////////////////////////////////////////////
        void retrieve_permutations(size_t &m, size_t** &perm, size_t num_dim, size_t upper_bound,
                                   bool force_sum_constraint, size_t sum_constraint);

        void build_radial_basis_function(size_t icell);
        
        void VPS_LS_retrieve_weights(size_t cell_index);
        
        double evaluate_basis_function(double* x, size_t icell, size_t ibasis);

        int constrained_LeastSquare(size_t n, size_t m, double** H, double* w, double* f);
        double vec_dot_vec(size_t n, double* vec_a, double* vec_b);
        double vec_pow_vec(size_t num_dim, double* vec_a, size_t* vec_b);
        bool Cholesky(int n, double** A, double** LD);
        void Cholesky_solver(int n, double** LD, double* b, double* x);
        void GMRES(size_t n, double** A, double* b, double* x, double eps);
        void printMatrix(size_t m, size_t n, double** M);
        
        
                
    
        //////////////////////////////////////////////////////////////
        // General METHODS
        //////////////////////////////////////////////////////////////
        
        void initiate_random_number_generator(unsigned long x);
        double generate_a_random_number();
        
        size_t retrieve_closest_cell(double* x);
        
        // spoke darts
        bool trim_line_using_Hyperplane(size_t num_dim,                               // number of dimensions
                                        double* st, double *end,                      // line segmenet end points
                                        double* qH, double* nH);                      // a point on the hyperplane and it normal

        
        //////////////////////////////////////////////////////////////
        // Debugging METHODS
        //////////////////////////////////////////////////////////////
        
        double f_test(double* x);                       // some test functions for debugging
        
        double* grad_f_test(double* x);
        
        double** hessian_f_test(double* x);
        
        void generate_poisson_disk_sample(double r);
        
        void generate_MC_sample();
    
        void isocontouring(std::string file_name, bool plot_test_function, bool plot_surrogate, std::vector<double> contours);
    
        void isocontouring_solid(std::string file_name, bool plot_test_function, bool plot_surrogate, std::vector<double> contours);
    
        void plot_neighbors();

        
        //////////////////////////////////////////////////////////////////////////////////////////
        ////// Inherited method from parent class
        //////////////////////////////////////////////////////////////////////////////////////////

    protected:
        //
        //- Heading: Virtual function redefinitions
        //

        /// return the minimum number of samples (unknowns) required to
        /// build the derived class approximation type in numVars dimensions
        int min_coefficients() const;

        /// return the number of constraints to be enforced via an anchor point
        int num_constraints()  const;

        /// builds the approximation from scratch
        void build();

        /// retrieve the predicted function value for a given parameter set
        Real value(const Variables& vars);

        /// retrieve the function gradient at the predicted value
        /// for a given parameter set
        const RealVector& gradient(const Variables& vars);

        /// retrieve the variance of the predicted value for a given parameter set
        Real prediction_variance(const Variables& vars);

private: 

  //
  //- Heading: Member functions
  //

  /// Function to compute coefficients governing the VPS surrogates.
  void VPSmodel_build();

  /// Function returns a response value using the VPS surface.
  /** The response value is computed at the design point specified
      by the RealVector function argument.*/
  void VPSmodel_apply(const RealVector& new_x, bool variance_flag,
		     bool gradients_flag);

        //
        //- Heading: Data
        //

        /// pointer to the active object instance used within the static evaluator
        static VPSApproximation* VPSinstance;

        /// value of the approximation returned by value()
        Real approxValue;
        /// value of the approximation returned by prediction_variance()
        Real approxVariance;
        /// A 2-D array (num sample sites = rows, num vars = columns)
        /// used to create the Gaussian process
        RealMatrix trainPoints;
        /// An array of response values; one response value per sample site
        RealMatrix trainValues;
        /// The number of observations on which the GP surface is built.
        size_t numObs;
        /// The order of the polynomial in each Voronoi cell
        int surrogateOrder;
    
    
        enum subsurrogate{LS, GP};
        enum subsurrogate_basis{polynomial, radial};
        enum testfunction{SmoothHerbie, Herbie, Cone, Cross, UnitSphere, Linear34};
        
        subsurrogate       _vps_subsurrogate;
        subsurrogate_basis _vps_subsurrogate_basis;
        testfunction       _vps_test_function;
        
        // variables for Random number generator
        double Q[1220];
        int indx;
        double cc;
        double c; /* current CSWB */
        double zc;	/* current SWB `borrow` */
        double zx;	/* SWB seed1 */
        double zy;	/* SWB seed2 */
        size_t qlen;/* length of Q array */
    
        size_t _n_dim; // dimension of the problem
        double* _xmin; // lower left corner of the domain
        double* _xmax; // Upper right corner of the domain
        double  _diag; // diagonal of the domain
    
        // variables for VPS
        size_t _num_inserted_points;
        double** _sample_points; // points coordinates
        double* _fval;           // a single function evaluation at each point
        double** _fgrad;         // gradient vectors at each point
        double*** _fhess;        // hessian matrix at each point
        
        size_t** _sample_neighbors;  // cell direct neighbors
        size_t** _vps_ext_neighbors; // cell extended neighbors
    
    
        size_t _vps_order, _num_GMRES;
        size_t* _num_cell_basis_functions; // number of basis functions for each cell
        double* _sample_vsize;  // furthest distance between seed and one of its Voronoi corners
        double* _vps_dfar;       // furthest distance between a seed and its extended neighbors
        double*** _sample_basis;  // centers of rbs for a given cell
        
        double _max_vsize;     // size of biggest Voronoi cell
        double _disc_min_jump; // minimum jump for discontinuity detection
        double _disc_min_grad; // minimum gradient for discontinuity detection
        
        double _f_min, _f_max; //minimum and maximum function values;
        

        ////////////////////////////////////////////////////////////////////////////////
        // Sub surrogate variables
        ///////////////////////////////////////////////////////////////////////////////
        // LS
        
        size_t***  _vps_t; // LS_polynomials powers of the polynomial expansion
        double**   _vps_w; // LS weights per point and basis function

        // GP
        SharedApproxData sharedData;
        std::vector<Approximation> gpApproximations; // One approximation for each cell
        Variables gpEvalVars;
        

        // Debugging variables
        bool _use_derivatives;
        bool _use_gradient;
        bool _use_hessian;


};


} // namespace Dakota

#endif
