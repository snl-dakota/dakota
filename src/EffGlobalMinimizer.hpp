/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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

    void pre_run();
    void core_run();
    void post_run(std::ostream& s);

    const Model& algorithm_space_model() const;

    void declare_sources();

private:

    //
    //- Heading: Convenience member functions
    //

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

    /// probability improvement (PI) function for the EGO
    /// PI acquisition function implementation
    Real compute_probability_improvement(const RealVector& means, const RealVector& variances);
    /// expected improvement (EI) function for the EGO
    /// EI acquisition function implementation
    Real compute_expected_improvement(const RealVector& means, const RealVector& variances);
    /// lower confidence bound (LCB) function for the EGO
    /// LCB acquisition function implementation
    Real compute_lower_confidence_bound(const RealVector& means, const RealVector& variances);
    /// variance function for the EGO
    /// MSE acquisition implementation
    Real compute_variances(const RealVector& variances);

    /// expected violation function for the constraint functions
    RealVector expected_violation(const RealVector& means,
                                        const RealVector& variances);

    /// initialize and update the penaltyParameter
    void update_penalty();

    //
    //- Heading: Objective/constraint evaluators passed to RecastModel
    //


    /// static function used as the objective function in the
    /// Expected Improvement (EIF) problem formulation for EGO
    static void PIF_objective_eval(const Variables& sub_model_vars,
                                        const Variables& recast_vars,
                                  const Response& sub_model_response,
                                  Response& recast_response);
    /// static function used as the objective function in the
    /// Expected Improvement (EIF) problem formulation for EGO
    static void EIF_objective_eval(const Variables& sub_model_vars,
                                        const Variables& recast_vars,
                                  const Response& sub_model_response,
                                  Response& recast_response);
    /// static function used as the objective function in the
    /// Lower-Confidence Bound (LCB) problem formulation for EGO
    static void LCB_objective_eval(const Variables& sub_model_vars,
                                  const Variables& recast_vars,
                                  const Response& sub_model_response,
                                  Response& recast_response);
    /// Variance formulation for primary
    static void Variances_objective_eval(const Variables& sub_model_vars,
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
    void construct_batch_acquisition();
    /// construct batch exploration
    void construct_batch_exploration();
    /// delete liar responses
    void delete_liar_responses();
    /// evaluate batch
    void evaluate_batch_and_update_constraints();
    /// update convergence counters
    void update_convergence_counters();

    /// convergence checkers
    // check convergence if EGO has converged
    //void check_convergence_deprecated(const Real& eif_star,
    //                       const RealVector& c_vars,
    //                       RealVector prev_cv_star,
    //                       unsigned short eif_convergence_cntr,
    //                       unsigned short dist_convergence_cntr);
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

    /// recast model which assimilates either (a) mean and variance to solve
    /// the max(EIF) sub-problem (used by EIF_objective_eval()) or (b) variance
    /// alone for pure exploration (used by Variances_objective_eval())
    Model approxSubProbModel;
    /* Note: don't need a separate model for EIF vs. exploration since the
       underlying simulation model is the one that evaluates the truth data
       and the recastings are only used for the approximate sub-problem solve.
       So there is no need to segregate processing queues: the aggregate set
       of available {variables,response} updates can be pushed to the GP
       irregardless of acquisition type. */

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
    /// total batch sizes
    int batchSize;
    /// sampling point located at maximum acquisition function for batchSizeAcquisition
    int batchSizeAcquisition;
    /// sampling point located at maximum posterior variance for batchSizeExploration
    int batchSizeExploration;
    /// number of points in the current GP
    size_t numDataPts;

    /// placeholder for batch input (before querying the batch)
    VariablesArray varsArrayBatch;

    /// liar response
    IntResponseMap synchronousRespStarTruth;

    /// check model parallelism
    /// bool flag if model supports asynchronous parallelism
    bool parallelFlag;

    /// convergence checkers
    /// tolerance convergence on distance between predicted best-so-far samples
    Real distanceTol;
    /// limit convergence (compared with tolerance) in input measured in L2
    Real distCStar;
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
