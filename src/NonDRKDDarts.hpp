/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDRKDDarts
//- Description: Class for the Recursive k-d DARTS approach
//- Owner:       Mohamed Ebeida, Laura Swiler, and Ahmad Rushdi
//- Checked by:
//- Version:

#ifndef NOND_RKDDARTS_H
#define NOND_RKDDARTS_H


#include "dakota_data_types.hpp"
#include "DakotaNonD.hpp"
#include "DakotaApproximation.hpp"
#include "VPSApproximation.hpp"



namespace Dakota {


/// Base class for the Recursive k-d Dart methods within DAKOTA/UQ

/** The NonDRKDDart class recursively implements the numerical integration of a domain based on k-d flat samples. */

class NonDRKDDarts: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDRKDDarts(ProblemDescDB& problem_db, Model& model); ///< constructor
  ~NonDRKDDarts();                                       ///< destructor

  //
  //- Heading: Member functions
  //

  /// perform analysis
    void quantify_uncertainty();

protected: 
  //
  //- Heading: Convenience functions
  //
    
    
    void initiate_random_number_generator(unsigned long x);
    
    double generate_a_random_number();
    
    void init_rkd_darts();
    
    void exit_rkd_darts();
  
    void execute(size_t kd);
    
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
    
    void compute_response(double* x);
    
    void verify_neighbor_consistency();
    
    bool add_neighbor(size_t ipoint, size_t ineighbor);

    void retrieve_neighbors(size_t ipoint, bool update_point_neighbors);
    
    void sample_furthest_vertex(size_t ipoint, double* fv);

    
    ////////////////////////////////////////////////////////////////
    // RKD METHODS
    ////////////////////////////////////////////////////////////////
    
    void update_global_L();
    
    void assign_sphere_radius_RKD(size_t isample);
    
    void  shrink_big_spheres(); // shrink all disks by 90% to allow more sampling
    
    double area_triangle(double x1, double y1, double x2, double y2, double x3, double y3);
   
    //////////////////////////////////////////////////////////////
    // Surrogate METHODS
    //////////////////////////////////////////////////////////////
    void initialize_surrogates();
    void add_surrogate_data(const Variables& vars, const Response& resp);
    void build_surrogate();
    double eval_surrogate(size_t fn_index, double *vin);
    void estimate_rkd_surrogate();
    
    bool trim_line_using_Hyperplane(size_t num_dim,                               // number of dimensions
                                    double* st, double *end,                      // line segmenet end points
                                    double* qH, double* nH);                      // a point on the hyperplane and it normal
    
    
    ////////////////////////////////////////////////////////////////
    // OUTPUT METHODS
    ////////////////////////////////////////////////////////////////
    
    double f_true(double* x); // for debuging only
    
    void plot_vertices_2d(bool plot_true_function, bool plot_suurogate);
    
    void plot_neighbors();
    
  //
  //- Heading: Data
  //
    
    // number of samples of true function
    int samples;
    
    // user-specified seed
    int seed;
    
    // number of samples on emulator  
    int emulatorSamples;

    // type of estimation for Lipschitz constants
    String lipschitzType;
 
 
    // variables for Random number generator
    double Q[1220];
    int indx;
    double cc;
    double c; /* current CSWB */
    double zc;	/* current SWB `borrow` */
    double zx;	/* SWB seed1 */
    double zy;	/* SWB seed2 */
    size_t qlen;/* length of Q array */
    
    
    // Variables of the RKD darts algorithm
    
    bool _eval_error;
    size_t _test_function;
    
    size_t _n_dim; // dimension of the problem
    double* _xmin; // lower left corner of the domain
    double* _xmax; // Upper right corner of the domain
    double  _diag; // diagonal of the domain
    
    double _failure_threshold;
    
    // Input
    double _num_darts;
    double _num_successive_misses_p;
    double _num_successive_misses_m;
    double _max_num_successive_misses;
    
    
    double _accepted_void_ratio; // termination criterion for a maximal sample
    
    // Output
    size_t _num_inserted_points, _total_budget;
    double** _sample_points; // store radius as a last coordinate
    size_t** _sample_neighbors;
    double*  _sample_vsize;
    double   _max_vsize; // size of biggest Voronoi cell
    
    // Darts
    double* _dart; // a dart for inserting a new sample point
    
    size_t _flat_dim;
    size_t* _line_flat;
    size_t _num_flat_segments;
    double* _line_flat_start;
    double* _line_flat_end;
    double* _line_flat_length;
    
    double _safety_factor;
    double* _Lip;
    double** _fval;
    size_t _active_response_function;
    
    bool _use_local_L;

};

} // namespace Dakota

#endif
