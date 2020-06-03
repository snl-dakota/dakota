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

//- Edited by:   Anh Tran on 12/21/2019

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

    //void derived_init_communicators(ParLevLIter pl_iter);
    //void derived_set_communicators(ParLevLIter pl_iter);
    //void derived_free_communicators(ParLevLIter pl_iter);

    void core_run();

    const Model& algorithm_space_model() const;

    void declare_sources();

private:

    //
    //- Heading: Convenience member functions
    //

    /// called by minimize_surrogates for setUpType == "model"
    void minimize_surrogates_on_model_deprecated();

    /// determine best solution from among the dataset
    void get_best_sample();

    /// initialize convergence criteria (subjected to change however)
    void initialize_convergence_variables();

    /// build initial GP responses after initial sampling
    void build_gp();

    /// get augmented Lagrangian
    Real get_augmented_lagrangian(const RealVector& mean,
                                  const RealVector& c_vars,
                                  const Real& eif_star);

    /// print mean and variance if debug flag is ON
    void debug_print_values();

    /// print counter if debug flag is ON
    void debug_print_counter(unsigned short globalIterCount,
                             const Real& eif_star,
                             Real distCStar,
                             unsigned short dist_convergence_cntr);

    // DEBUG - output set of samples used to build the GP
    // If problem is 2d, output a grid of points on the GP and truth (if requested)
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

    /// function that checks if model supports asynchronous parallelism
    bool check_parallelism();

    /// sequential EGO implementation: main function
    void serial_ego();

    /// synchronous batch-sequential implementation: main function
    void batch_synchronous_ego();
    /// construct batch acquisition
    void construct_batch_acquisition(int BatchSizeAcquisition, VariablesArray varsArrayBatchAcquisition);
    /// delete liar responses
    void delete_liar_responses(int BatchSizeAcquisition);
    /// evaluate batch
    void evaluate_batch(int BatchSizeAcquisition);

    /// convergence checkers
    /// check convergence if EGO has converged
    void check_convergence_deprecated(const Real& eif_star,
                          const RealVector& c_vars,
                          RealVector prev_cv_star,
                          unsigned short eif_convergence_cntr,
                          unsigned short dist_convergence_cntr);
    /// check convergence if EGO has converged
    bool assess_convergence();

    /// post-processing: print best samples and responses
    void post_process();

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

    /// declare batch sizes
    /// sampling point located at maximum acquisition function for BatchSizeAcquisition
    int BatchSizeAcquisition;
    /// sampling point located at maximum posterior variance for BatchSizeExploration
    int BatchSizeExploration;
    /// number of points in the current GP
    size_t numDataPts;

    /// placeholder for batch input (before querying the batch)
    VariablesArray varsArrayBatchAcquisition;

    /// liar response
    const IntResponsePair respStarLiar;

    /// check model parallelism
    /// bool flag if model supports asynchronous parallelism
    bool parallelFlag;

    /// convergence checkers
    /// tolerance convergence on distance between predicted best-so-far samples
    Real distanceTol;
    /// limit convergence (compared with tolerance) in input measured in L2
    Real distCStar;
    /// tolerance convergence in objectives
    Real convergenceTol;
    /// counter for convergence in EIF
    unsigned short eifConvergenceCntr;
    /// limit convergence (compared with counter) of EIF
    unsigned short eifConvergenceLimit;
    /// counter for distance in input space
    unsigned short distConvergenceCntr;
    /// limit for distance (compared with counter) in input space
    unsigned short distConvergenceLimit;
    /// counter for global iteration
    unsigned short globalIterCount;
    /// bool flag for convergence
    bool approxConverged;
    /// previous best-so-far sample
    RealVector prevCvStar;


};


inline const Model& EffGlobalMinimizer::algorithm_space_model() const
{ return fHatModel; }

} // namespace Dakota

#endif
