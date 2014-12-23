/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 VPSApproximation
//- Description: Class implementation of Voronoi Piecewise Surrogate Approximation
//- Owner:       Mohamed Ebeida
//- Checked by:
//- Version:

#ifndef VPS_APPROXIMATION_H
#define VPS_APPROXIMATION_H

#include "dakota_data_types.hpp"
#include "DakotaApproximation.hpp"
#include "SharedSurfpackApproxData.hpp"

#include "Teuchos_SerialSpdDenseSolver.hpp"

#ifdef HAVE_OPTPP
namespace Teuchos { 
  template<typename OrdinalType, typename ScalarType> class SerialDenseVector;
  template<typename OrdinalType, typename ScalarType> class SerialDenseMatrix;
}
#endif // HAVE_OPTPP


namespace Dakota {

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
  /// alternate constructor
  VPSApproximation(const SharedApproxData& shared_data);
  /// standard constructor
  VPSApproximation(const ProblemDescDB& problem_db,
			 const SharedApproxData& shared_data);
  /// destructor
  ~VPSApproximation();


    //////////////////////////////////////////////////////////////
    // VPS METHODS
    //////////////////////////////////////////////////////////////
    bool VPS_execute();
    double evaluate_surrogate(double* x);

    
    void retrieve_neighbors(size_t ipoint, bool update_point_neighbors);
    void VPS_adjust_extend_neighbors_of_all_points();
    void VPS_extend_neighbors(size_t ipoint);
    void VPS_retrieve_poly_coefficients_for_all_points();
    void VPS_retrieve_poly_coefficients(size_t ipoint);
    void VPS_destroy_global_containers();
    
    void retrieve_permutations(size_t &m, size_t** &perm, size_t num_dim, size_t upper_bound, bool include_origin, bool force_sum_constraint, size_t sum_constraint);
    
    
    void initiate_random_number_generator(unsigned long x);
    double generate_a_random_number();
    
    
    bool trim_line_using_Hyperplane(size_t num_dim,                               // number of dimensions
                                    double* st, double *end,                      // line segmenet end points
                                    double* qH, double* nH);                      // a point on the hyperplane and it normal
    
    double vec_pow_vec(size_t num_dim, double* vec_a, size_t* vec_b);
    bool Cholesky(int n, double** A, double** LD);
    void Cholesky_solver(int n, double** LD, double* b, double* x);
    void GMRES(size_t n, double** A, double* b, double* x, double eps);

    
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
    size_t _num_inserted_points, _total_budget;
    double** _sample_points; // store radius as a last coordinate
    size_t** _sample_neighbors;
    double* _fval;
    
    size_t _vps_order, _vps_num_poly_terms, _num_GMRES;
    double* _vps_dfar; // furthest distance between a seed and its extended neighbors
    size_t**  _vps_t;  // powers of the polynomial expansion
    double** _vps_c;  // polynomial coeffcients per point function
    
    size_t** _vps_ext_neighbors;
    
    double*  _sample_vsize;
    double   _max_vsize; // size of biggest Voronoi cell

};


/** alternate constructor */

inline VPSApproximation::VPSApproximation()
{
}

inline VPSApproximation::
VPSApproximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data)
{
    
    SharedSurfpackApproxData* dat = dynamic_cast<SharedSurfpackApproxData*> (shared_data.data_rep());
    
    //if (dat == 0) std::cout<< "Casting failed"<< std::endl;
    //else std::cout<< "Casting succeeded"<< std::endl;
    
    surrogateOrder = dat->approxOrder;
    
    std::cout << "*** VPS:: Initializing, Surrogate order " << surrogateOrder << std::endl;

}

inline VPSApproximation::~VPSApproximation()
{
    VPS_destroy_global_containers();
}

} // namespace Dakota

#endif
