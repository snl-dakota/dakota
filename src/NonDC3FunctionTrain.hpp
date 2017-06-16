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
        void compute_expansion();
        // void pre_run();
        /// perform a forward uncertainty propagation using PCE/SC methods
        void core_run();
        /// TODO
        void post_run(std::ostream& s);
        /// print the final statistics
        // void print_results(std::ostream& s);

    protected:

        //
        //- Heading: Virtual function redefinitions
        //

        void resolve_inputs(short& u_space_type, short& data_order);
        void initialize(short u_space_type);

        //
        //- Heading: Member function definitions
        //

        /// user specified import build points file
        String importBuildPointsFile;
        /// user specified import build file format
        unsigned short importBuildFormat;
        /// user specified import build active only
        bool importBuildActiveOnly;


    private:

        /// initialize uSpaceModel with input variable data
        void initialize_u_space_model();
        
        /// OBJFUN in NPSOL manual: computes the value and first derivatives of the
        /// objective function (passed by function pointer to NPSOL).
        static int qoi_eval(size_t num_samp,        // number of evaluations
                            const double* var_sets, // num_vars x num_evals
                            double* qoi_sets,       // num_fns x num_evals
                            void* args);            // optional arguments

        //
        //- Heading: Data
        //

        unsigned int randomSeed;
    
        /// pointer to the active object instance used within the static evaluator
        /// functions in order to avoid the need for static data
        static NonDC3FunctionTrain* c3Instance;

        // other data ...
        /// The number of samples used to evaluate the emulator
        int numSamplesOnEmulator;

        /// user specified import approx. points file
        String importApproxPointsFile;
        /// user specified import approx. file format
        unsigned short importApproxFormat;
        /// user specified import approx. active only
        bool importApproxActiveOnly;
        /// file name from \c export_approx_points_file specification
        String exportPointsFile;


        /// override certain print functions 
        void print_moments(std::ostream& s);
        void print_sobol_indices(std::ostream& s);
        void print_local_sensitivity(std::ostream& s);
    
        void compute_diagonal_variance();
        void compute_off_diagonal_covariance();
        void compute_analytic_statistics();
    };

} // namespace Dakota

#endif
