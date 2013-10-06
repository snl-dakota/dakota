/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDPOFDarts
//- Description: Class for the Probability of Failure DARTS approach
//- Owner:	 Mohamed Ebeida and Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_POFDARTS_H
#define NOND_POFDARTS_H


#include "dakota_data_types.hpp"
#include "DakotaNonD.hpp"

namespace Dakota {


/// Base class for POF Dart methods within DAKOTA/UQ

/** The NonDPOFDart class implements the calculation of a failure 
    probability for a specified threshold for a specified response
    function using the concepts developed by Mohamed Ebeida. 
    The approach works by throwing down a number of Poisson disk samples 
    of varying radii, and identifying each disk as either in the  
    failure or safe region. The center of each disk represents a "true" 
    function evaluation. kd-darts are used to place additional 
    points, in such a way to target the failure region.  When the 
    disks cover the space sufficiently, Monte Carlo methods or a 
    box volume approach is used to calculate both the lower and 
    upper bounds on the failure probability. */ 

class NonDPOFDarts: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDPOFDarts(Model& model); ///< constructor
  ~NonDPOFDarts();            ///< destructor

  //
  //- Heading: Member functions
  //

  /// perform POFDart analysis and return probability of failure 
  void quantify_uncertainty(); // called by run_iterator

protected: 
  //
  //- Heading: Convenience functions
  //
    
    
    void initiate_random_number_generator(unsigned long x);
    
    double generate_a_random_number();
    
    void init_pof_darts();
    
    void exit_pof_darts();
  
    void execute(size_t kd);
    
    void print_POF_results(double lower, double upper);
    /// print the final statistics
    void print_results(std::ostream& s);
    
    //////////////////////////////////////////////////////////////
    // MPS METHODS
    //////////////////////////////////////////////////////////////
    
    void classical_dart_throwing_games(size_t game_index);
    
    void line_dart_throwing_games(size_t game_index);

    bool valid_dart(double* x);
    
    bool valid_line_flat(size_t flat_dim, double* flat_dart);
    
    void add_point(double* x);
    
    ////////////////////////////////////////////////////////////////
    // OPT / POF METHODS
    ////////////////////////////////////////////////////////////////
    void  shrink_all_spheres(); // shrink all disks by 90% to allow more sampling
    
    void adjust_radius_of_previous_samples_due_to_new_threshold();
    
    void assign_sphere_radius_POF(double* x, size_t isample);
    
    void assign_sphere_radius_OPT(double* x, size_t isample);
    
    void resolve_overlap_POF();
    
    double f_true(double* x);
    
    double f_grad_FD(double* x, double f, double &curv);
    
    double get_dart_radius(double f, double fgrad, double fcurv);
    
    ////////////////////////////////////////////////////////////////
    // VOLUME OF SPHERE UNION METHODS
    ////////////////////////////////////////////////////////////////
    
    void retrieve_POF_bounds(double &lower, double &upper);
    
    double estimate_spheres_volume_0d(double** spheres, size_t num_spheres, size_t num_dim, double* xmin, double* xmax);
    
    double get_sphere_volume(double r, size_t num_dim);
    
    void sample_uniformly_from_unit_sphere(double* dart, size_t num_dim);
    
    double area_triangle(double x1, double y1, double x2, double y2, double x3, double y3);
    
    
    ////////////////////////////////////////////////////////////////
    // OUTPUT METHODS
    ////////////////////////////////////////////////////////////////
    
    void plot_vertices_2d();
    
  //
  //- Heading: Data
  //
    
    // number of samples of true function
    int samples;
    
    // user-specified seed
    int seed;
    
    
    // variables for Random number generator
    double Q[1220];
    int indx;
    double cc;
    double c; /* current CSWB */
    double zc;	/* current SWB `borrow` */
    double zx;	/* SWB seed1 */
    double zy;	/* SWB seed2 */
    size_t qlen;/* length of Q array */
    
    
    // Variables of the POF dart algorithm
    
    size_t _n_dim; // dimension of the problem
    double* _xmin; // lower left corner of the domain
    double* _xmax; // Upper right corner of the domain
    
    bool   _global_optimization;
    double _failure_threshold;
    double _global_minima;
    
    // Input
    double _num_darts;
    double _num_successive_misses_p;
    double _num_successive_misses_m;
    double _max_num_successive_misses;
    
    
    double _max_radius; // maximum radius of a sphere
    double _accepted_void_ratio; // termination criterion for a maximal sample
    
    // Output
    size_t _num_inserted_points, _total_budget;
    double** _sample_points; // store radius as a last coordinate
    
    // Darts
    double* _dart; // a dart for inserting a new sample point
    double* _grad_vec; // Function gradient at a given point
    
    size_t _flat_dim;
    size_t* _line_flat;
    size_t _num_flat_segments;
    double* _line_flat_start;
    double* _line_flat_end;
    double* _line_flat_length;
    

};

} // namespace Dakota

#endif
