/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EffGlobalMinimizer
//- Description: Implementation of Efficient Global Optimization
//- Owner:       Barron J Bichon, Vanderbilt University
//- Checked by:
//- Version:

//- Edited by:   Anh Tran

#ifndef EGO_MINIMIZER_H
#define EGO_MINIMIZER_H

#include "SurrBasedMinimizer.hpp"


namespace Dakota {


/// Implementation of Efficient Global Optimization/Least Squares algorithms


/**
 * \brief A version of TraitsBase specialized for efficient global minimizer
 *
 */

class EffGlobalTraits: public TraitsBase
{
    public:

    /// default constructor
    EffGlobalTraits() { }

    /// destructor
    virtual ~EffGlobalTraits() { }

    /// A temporary query used in the refactor
    virtual bool is_derived() { return true; }

    /// Return the flag indicating whether method supports continuous variables
    bool supports_continuous_variables() { return true; }

    /// Return the flag indicating whether method supports nonlinear equalities
    bool supports_nonlinear_equality() { return true; }

    /// Return the flag indicating whether method supports nonlinear inequalities
    bool supports_nonlinear_inequality() { return true; }
};


/** The EffGlobalMinimizer class provides an implementation of the
    Efficient Global Optimization algorithm developed by Jones, Schonlau,
    & Welch as well as adaptation of the concept to nonlinear least squares. */

class EffGlobalMinimizer: public SurrBasedMinimizer
{
public:

    //
    //- Heading: Constructors and destructor
    //

    /// standard constructor
    EffGlobalMinimizer(ProblemDescDB& problem_db, Model& model);
    /// alternate constructor for instantiations "on the fly"
    //EffGlobalMinimizer(Model& model, int max_iterations, int max_fn_evals);
    ~EffGlobalMinimizer(); /// destructor

    //
    //- Heading: Virtual function redefinitions
    //

    // void derived_init_communicators(ParLevLIter pl_iter);
    // void derived_set_communicators(ParLevLIter pl_iter);
    // void derived_free_communicators(ParLevLIter pl_iter);

    void core_run();

    const Model& algorithm_space_model() const;

    void declare_sources();

private:

    //
    //- Heading: Convenience member functions
    //

    /// called by minimize_surrogates for setUpType == "model"
    void minimize_surrogates_on_model();
    /// called by minimize_surrogates for setUpType == "user_functions"
    // void minimize_surrogates_on_user_functions();

    /// determine best solution from among sample data for expected
    /// imporovement function
    void get_best_sample();

    /// initialize
    /// build initial GP responses
    void build_gp();

    /// for parallel EGO
    /// construct the acquisition batch
    void construct_batch_acquisition(int BatchSizeAcquisition);

    /// for parallel EGO
    /// delete liar responses in fHatModel
    void delete_liar_responses(int BatchSizeAcquisition);

    /// for parallel EGO
    /// query/update responses/update constraints
    void query_batch(int BatchSizeAcquisition);

    /// augmented Lagrangian
    /// augmented Lagrangian
    Real get_augmented_lagrangian(const RealVector& mean,
                                  const RealVector& c_vars,
                                  const Real& eif_star);

    /// check convergence
    /// if EGO has converged
    void check_convergence(const Real& eif_star,
                          const RealVector& c_vars,
                          RealVector prev_cv_star,
                          unsigned short eif_convergence_cntr,
                          unsigned short dist_convergence_cntr);

    /// print mean and variance if debug flag is ON
    void debug_print_values();

    /// print counter if debug flag is ON
    void debug_print_counters(unsigned short globalIterCount,
                              const Real& eif_star,
                              Real distCStar,
                              unsigned short dist_convergence_cntr);

    // DEBUG - output set of samples used to build the GP
    // If problem is 2d, output a grid of points on the GP
    //   and truth (if requested)
    void debug_plots();

    /// expected improvement function for the GP
    Real expected_improvement(const RealVector& means,
  			                     const RealVector& variances);

    /// expected violation function for the constraint functions
    RealVector expected_violation(const RealVector& means,
      				                    const RealVector& variances);

    /// initialize and update the penaltyParameter
    void update_penalty();

    //
    //- Heading: Objective/constraint evaluators passed to RecastModel
    //

    /// static function used as the objective function in the
    /// Expected Improvement (EIF) problem formulation for PMA
    static void EIF_objective_eval(const Variables& sub_model_vars,
  				                        const Variables& recast_vars,
                                  const Response& sub_model_response,
                                  Response& recast_response);

    //
    //- Heading: Data
    //

    /// pointer to the active object instance used within the static evaluator
    /// functions in order to avoid the need for static data
    static EffGlobalMinimizer* effGlobalInstance;
    // static EffGlobalMinimizer* prev_instance;

    /// controls iteration mode: "model" (normal usage) or "user_functions"
    /// (user-supplied functions mode for "on the fly" instantiations).
    String setUpType;

    /// convergence tolerance on distance
    /// between predicted best points
    Real distanceTol;

    /// convergence tolerances
    /// in objectives
    Real convergenceTol;

    /// GP model of response, one approximation per response function
    Model fHatModel;
    /// recast model which assimilates mean and variance to solve the
    /// max(EIF) sub-problem
    Model eifModel;

    /// minimum penalized response from among true function evaluations
    Real meritFnStar;
    /// true function values corresponding to the minimum penalized response
    RealVector truthFnStar;
    /// point that corresponds to the optimal value meritFnStar
    RealVector varStar;

    /// order of the data used for surrogate construction, in ActiveSet
    /// request vector 3-bit format; user may override responses spec
    short dataOrder;

    /// declare check convergence variables
    /// relative distance change in input measured in L2
    Real distCStar;

    /// declare batch size for BatchSizeAcquisition
    /// sampling point located at maximum acquisition function
    int BatchSizeAcquisition;

    /// declare batch size for BatchSizeExploration
    /// sampling point located at maximum posterior variance
    int BatchSizeExploration;

    /// placeholder for batch inputs
    /// before evaluating a batch
    VariablesArray varsArrayBatchAcq;

    /// declar parallel_flag
    /// if model can support asynchronously parallel evaluation
    bool parallelFlag;

    /// best-so-far variables and their associates
    /// for evaluating EI acquisition
    Variables  varsStar;
    RealVector cVars;
    Response   respStar;
    Real       eifStar;

};


inline const Model& EffGlobalMinimizer::algorithm_space_model() const { return fHatModel; }

} // namespace Dakota

#endif
