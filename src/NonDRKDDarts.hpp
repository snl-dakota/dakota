/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDRKDDarts
//- Description: Class for the Recursive k-d DARTS approach
//- Owner:       Mohamed Ebeida and Ahmad Rushdi
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

  // ----------------------
  // - Heading: Constructors and destructor
  // ----------------------
    
    NonDRKDDarts(ProblemDescDB& problem_db, Model& model); ///< constructor

    ~NonDRKDDarts();                                       ///< destructor

    //
    //- Heading: Virtual function redefinitions
    //

    bool resize();
    void core_run();

protected:
    
    //
    //- Heading: Virtual function redefinitions
    //
    
    // together the three run components perform a forward uncertainty
    // propagation to generate a set of parameter samples,
    // performing function evaluations on these parameter samples, and
    // computing statistics on the ensemble of results.
    
    /// generate samples
    void pre_run();
    
    void initiate_random_number_generator(unsigned long x);
    
    double generate_a_random_number();
    
    void init_rkd_darts();
    
    void create_rkd_containers(size_t expected_num_samples);
        
    // -----
    // perform the evaluate parameter sets portion of run
    //void core_run();
    
    void execute();
    
    void create_initial_children(size_t parent);
    
    void create_new_sample(size_t parent, size_t left, size_t right, double position);
    
    void improve_parent_evaluation(size_t parent);
    
    void evaluate_1d_surrogate(size_t parent);
    
    double get_surrogate_interp_error(size_t parent);
    
    double estimate_surrogate_evaluation_err(size_t parent);
    
    void get_children(size_t parent, size_t* children);
    
    void get_neighbors(size_t sample, size_t &num_neighbors, size_t* neighbors);
    
    double interpolate_lagrange(size_t num_data_points, double* data_x, double* data_f, double x);
    
    double integrate_legendre_gauss(double xmin, double xmax, size_t num_data_points, double* data_x, double* data_f, double &err_est);
    
    double f_true(double* x); // for debuging only
    
    // ----------------------
    // Dakota - related
    // ----------------------
    
    void initialize_surrogates();

    void compute_response(double* x);
    
    void add_surrogate_data(const Variables& vars, const Response& resp);

    void build_surrogate();
    
    double eval_surrogate(size_t fn_index, double *vin);
    
    void estimate_rkd_surrogate();
    
    /// -----
    
    /// generate statistics
    
    void post_run(std::ostream& s);
    
    void print_integration_results(std::ostream& s);
    
    void exit_rkd_darts();
    
    void destroy_rkd_containers();
    
    // ----------------------
    // - Heading: Data
    // ----------------------
    
    // number of samples of true function
    int samples;
    
    // user-specified seed
    int seed;
    
    // number of samples on emulator
    int emulatorSamples;
    
    // variables for Random number generator
    double Q[1220];
    int indx;
    double cc;
    double c; /* current CSWB */
    double zc;	/* current SWB `borrow` */
    double zx;	/* SWB seed1 */
    double zy;	/* SWB seed2 */
    size_t qlen;/* length of Q array */
    
private:
    
     // Variables of the RKD algorithm
    
    double* _I_RKD;
    
    bool _eval_error;
    
    size_t _test_function;
    
    size_t _num_inserted_points;

    size_t _num_dim, _num_samples, _max_num_samples, _num_evaluations, _evaluation_budget; // numberof dimensions, size of samples containers, _number of function evaluations
    
    size_t _max_num_neighbors; // maximum number of neighbor to conside in building lagrange interpolant in each cell
    
    double _bounding_box_volume;
    
    double _discont_jump_threshold; // discontinuity jump threshold for disconnecting neighbors across a discontinuity
    
    double* _xmin;
    double* _xmax;
    
    double** _fval;
    
    size_t* _sample_dim;
    size_t* _sample_parent;
    size_t* _sample_first_child;
    size_t* _sample_num_children;
    size_t* _sample_left;
    size_t* _sample_right;
    double* _sample_coord;              // Sample one dimensional coordinate
    double* _sample_value;              // Sample value (either through integration of children or direct function evaluation)
    double* _sample_left_interp_err;
    double* _sample_right_interp_err;
    double* _sample_left_ev_err;
    double* _sample_right_ev_err;
    
   };

} // namespace Dakota

#endif
