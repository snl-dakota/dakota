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

    // ----------------------
    // - Heading: Member functions
    // ----------------------

    // perform analysis
    void quantify_uncertainty();

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
    
    /// -----
    /// perform the evaluate parameter sets portion of run
    /// void quantify_uncertainty();
    
    double execute(size_t num_darts);
    
    double evaluate_function(double* x);
    
    /// -----
    
    /// generate statistics
    void post_run(std::ostream& s);
    
    void print_integration_results();
    
    void exit_rkd_darts();
    
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
    
    // Variables of the RKD algorithm
    size_t _num_dim; // dimension of the problem
    double* _xmin; // lower left corner of the domain
    double* _xmax; // Upper right corner of the domain
    
    double _I_RKD; // Numerical Integration value
    
   };

} // namespace Dakota

#endif
