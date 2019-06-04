/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDC3FunctionTrain
//- Description: Iterator for the 
//- Owner:       Alex Gorodetsky, Sandia National Laboratories

#ifndef NOND_C3_FUNCTION_TRAIN_H
#define NOND_C3_FUNCTION_TRAIN_H

// #include "DakotaNonD.hpp

#include "NonDExpansion.hpp"

namespace Dakota {

/// Nonintrusive uncertainty quantification with the C3 library ...

/** The NonDC3FunctionTrain class uses ... */

  class NonDC3FunctionTrain: public NonDExpansion
  {
  public:

    //
    //- Heading: Constructors and destructor
    //

    /// standard constructor
    NonDC3FunctionTrain(ProblemDescDB& problem_db, Model& model);
    /// destructor
    ~NonDC3FunctionTrain();

    //
    //- Heading: Virtual function redefinitions
    //

    /// TODO
    // void compute_expansion();
    /// perform a forward uncertainty propagation using PCE/SC methods
    // void core_run();
    /// TODO

  protected:

    //
    //- Heading: Virtual function redefinitions
    //

    void resolve_inputs(short& u_space_type, short& data_order);

    //
    //- Heading: Member function definitions
    //

    void initialize_data_fit_surrogate(Model& dfs_model);

    /// Publish options from C3 input specification (not needed if model-driven
    /// specification: already extracted by iteratedModel)
    void push_c3_options();

  private:

    //static int qoi_eval(size_t num_samp,        // number of evaluations
    // 			const double* var_sets, // num_vars x num_evals
    // 			double* qoi_sets,       // num_fns x num_evals
    // 			void* args);            // optional arguments

    //
    //- Heading: Data
    //

    /// pointer to the active object instance used within the static evaluator
    /// functions in order to avoid the need for static data
    static NonDC3FunctionTrain* c3Instance;

    unsigned int randomSeed;

    size_t numSamplesOnModel;
   
    // other data ...
    /// The number of samples used to evaluate the emulator
    //int numSamplesOnEmulator;

    // user specified import build points file
    //String importBuildPointsFile;
    // user specified import build file format
    //unsigned short importBuildFormat;
    // user specified import build active only
    //bool importBuildActiveOnly;

    // user specified import approx. points file
    //String importApproxPointsFile;
    // user specified import approx. file format
    //unsigned short importApproxFormat;
    // user specified import approx. active only
    //bool importApproxActiveOnly;
    // file name from \c export_approx_points_file specification
    //String exportPointsFile;

    /// override certain print functions
    // I should not have to define my own print functions -- AG
    // This suggests there needs to be some refactoring to truly separate
    // computing things and printing things
    // The only thing I should have to print is FT specific results 
    void print_results(std::ostream&);
    void print_moments(std::ostream& s);
    void print_sobol_indices(std::ostream& s);

  };
    
} // namespace Dakota

#endif
