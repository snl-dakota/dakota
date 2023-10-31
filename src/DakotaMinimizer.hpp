/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_MINIMIZER_H
#define DAKOTA_MINIMIZER_H

#include "DakotaIterator.hpp"
#include "DakotaResponse.hpp"
#include "DakotaTPLDataTransfer.hpp"
#include "ExperimentData.hpp"

namespace Dakota {

/// Base class for the optimizer and least squares branches of the
/// iterator hierarchy.

/** The Minimizer class provides common data and functionality
    for Optimizer and LeastSq. */

class Minimizer: public Iterator
{
  //
  //- Heading: Friends
  //

  /// the SOLBase class is not derived the iterator hierarchy but still needs
  /// access to iterator hierarchy data (to avoid attribute replication)
  friend class SOLBase;
  /// the SNLLBase class is not derived the iterator hierarchy but still needs
  /// access to iterator hierarchy data (to avoid attribute replication)
  friend class SNLLBase;

public:

  //
  //- Heading: Member functions
  //

  /// set the method constraint tolerance (constraintTol)
  void constraint_tolerance(Real constr_tol);
  /// return the method constraint tolerance (constraintTol)
  Real constraint_tolerance() const;

  /// return weighted sum of squared residuals
  static Real sum_squared_residuals(size_t num_pri_fns,
                                    const RealVector& residuals,
                                    const RealVector& weights);

  /// print num_terms residuals and misfit for final results
  static void print_residuals(size_t num_terms, const RealVector& best_terms,
                              const RealVector& weights,
                              size_t num_best, size_t best_index,
                              std::ostream& s);

  /// print the original user model resp in the case of data transformations
  static void print_model_resp(size_t num_pri_fns, const RealVector& best_fns,
                               size_t num_best, size_t best_index,
                               std::ostream& s);

  /// print best evaluation matching vars and set, or partial matches
  /// with matching variables only.
  static void print_best_eval_ids(const String& interface_id,
				  const Variables& best_vars,
				  const ActiveSet& active_set,
				  std::ostream& s);

  // Accessor for data transfer helper/adapters
  std::shared_ptr<TPLDataTransfer> get_data_transfer_helper() const
    { return dataTransferHandler; }

  //
  //- Heading: Virtual member function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  Minimizer(std::shared_ptr<TraitsBase> traits = 
      std::shared_ptr<TraitsBase>(new TraitsBase()));
  /// standard constructor
  Minimizer(ProblemDescDB& problem_db, Model& model, 
      std::shared_ptr<TraitsBase> traits =
      std::shared_ptr<TraitsBase>(new TraitsBase()));

  /// alternate constructor for "on the fly" instantiations
  Minimizer(unsigned short method_name, Model& model, 
      std::shared_ptr<TraitsBase> traits = 
      std::shared_ptr<TraitsBase>(new TraitsBase()));
  /// alternate constructor for "on the fly" instantiations
  Minimizer(unsigned short method_name, size_t num_lin_ineq, size_t num_lin_eq,
	    size_t num_nln_ineq, size_t num_nln_eq, 
            std::shared_ptr<TraitsBase> traits = 
            std::shared_ptr<TraitsBase>(new TraitsBase()));
  /// alternate constructor for "on the fly" instantiations
  Minimizer(Model& model, size_t max_iter, size_t max_eval, Real conv_tol,
	    std::shared_ptr<TraitsBase> traits = 
	    std::shared_ptr<TraitsBase>(new TraitsBase()));

  /// destructor
  ~Minimizer();

  //
  //- Heading: Virtual member function redefinitions
  //

  void update_from_model(const Model& model);

  void initialize_run();
  void post_run(std::ostream& s);
  void finalize_run();

  const Model& algorithm_space_model() const;

  //
  //- Heading: New virtual functions
  //

  /*
  /// initialize the iterator about to be executed within a parallel iterator
  /// scheduling function (serve_iterators() or static_schedule_iterators())
  void initialize_iterator(int index);
  /// pack a send_buffer for assigning an iterator job to a server
  void pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index);
  /// unpack a recv_buffer for accepting an iterator job from the scheduler
  void unpack_parameters_initialize(MPIUnpackBuffer& recv_buffer);
  /// pack a send_buffer for returning iterator results from a server
  void pack_results_buffer(MPIPackBuffer& send_buffer, int job_index);
  /// unpack a recv_buffer for accepting iterator results from a server
  void unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index);
  /// update local PRP results arrays with current iteration results
  void update_local_results(int job_index);
  */

  //
  //- Heading: Convenience/Helper functions
  //

  /// Return a shallow copy of the original model this Iterator was
  /// originally passed, optionally leaving recasts_left on top of it
  Model original_model(unsigned short recasts_left = 0) const; 

  /// Wrap iteratedModel in a RecastModel that subtracts provided
  /// observed data from the primary response functions (variables and
  /// secondary responses are unchanged)
  void data_transform_model();

  /// Wrap iteratedModel in a RecastModel that performs variable
  /// and/or response scaling
  void scale_model();

  /// ensure iteratedModel is null when using function callbacks for evaluation
  void enforce_null_model();

  /// compute a composite objective value from one or more primary functions
  Real objective(const RealVector& fn_vals, const BoolDeque& max_sense,
		 const RealVector& primary_wts) const;

  /// compute a composite objective with specified number of source
  /// primary functions, instead of userPrimaryFns
  Real objective(const RealVector& fn_vals, size_t num_fns,
		 const BoolDeque& max_sense,
		 const RealVector& primary_wts) const;

  /// compute the gradient of the composite objective function
  void objective_gradient(const RealVector& fn_vals, const RealMatrix& fn_grads,
			  const BoolDeque& max_sense,
			  const RealVector& primary_wts,
			  RealVector& obj_grad) const;
  /// compute the gradient of the composite objective function
  void objective_gradient(const RealVector& fn_vals, size_t num_fns,
			  const RealMatrix& fn_grads,
			  const BoolDeque& max_sense,
			  const RealVector& primary_wts,
			  RealVector& obj_grad) const;

  /// compute the Hessian of the composite objective function
  void objective_hessian(const RealVector& fn_vals, const RealMatrix& fn_grads,
			 const RealSymMatrixArray& fn_hessians,
			 const BoolDeque& max_sense,
			 const RealVector& primary_wts,
			 RealSymMatrix& obj_hess) const;
  /// compute the Hessian of the composite objective function
  void objective_hessian(const RealVector& fn_vals, size_t num_fns,
			 const RealMatrix& fn_grads,
			 const RealSymMatrixArray& fn_hessians,
			 const BoolDeque& max_sense,
			 const RealVector& primary_wts,
			 RealSymMatrix& obj_hess) const;

  /// top-level archival method
  virtual void archive_best_results();

  /// archive best variables for the index'th final solution
  void archive_best_variables(const bool active_only = false) const;

  /// archive the index'th set of objective functions
  void archive_best_objective_functions() const;

  /// archive the index'th set of constraints
  void archive_best_constraints() const;
  
  /// Archive residuals when calibration terms are used
  void archive_best_residuals() const;

  /// Safely resize the best variables array to newsize taking into
  /// account the envelope-letter design pattern and any recasting.
  void resize_best_vars_array(size_t newsize);

  /// Safely resize the best response array to newsize taking into
  /// account the envelope-letter design pattern and any recasting.
  void resize_best_resp_array(size_t newsize);

  /// infers MOO/NLS solution from the solution of a single-objective
  /// optimizer and returns true if lookup succeeds
  bool local_recast_retrieve(const Variables& vars, Response& response) const;

  /// reshape input/output sizes within best{Variables,Response}Array
  void reshape_best(size_t num_cv, size_t num_fns);

  //
  //- Heading: Data
  //

  // Isolate complexity by letting Model::currentVariables/currentResponse
  // manage details.  Then Iterator only needs the following:
  size_t numFunctions;          ///< number of response functions
  size_t numContinuousVars;     ///< number of active continuous vars
  size_t numDiscreteIntVars;    ///< number of active discrete integer vars
  size_t numDiscreteStringVars; ///< number of active discrete string vars
  size_t numDiscreteRealVars;   ///< number of active discrete real vars

  Real constraintTol;   ///< optimizer/least squares constraint tolerance

  /// cutoff value for inequality constraint and continuous variable bounds
  Real bigRealBoundSize;
  /// cutoff value for discrete variable bounds
  int bigIntBoundSize;

  /// number of nonlinear inequality constraints
  size_t numNonlinearIneqConstraints;
  /// number of nonlinear equality constraints
  size_t numNonlinearEqConstraints;

  /// number of linear inequality constraints
  size_t numLinearIneqConstraints;
  /// number of linear equality constraints
  size_t numLinearEqConstraints;

  /// total number of nonlinear constraints
  size_t numNonlinearConstraints;
  /// total number of linear constraints
  size_t numLinearConstraints;
  /// total number of linear and nonlinear constraints
  size_t numConstraints;

  /// flag for use where optimization and NLS must be distinguished
  bool optimizationFlag;
  /// number of objective functions or least squares terms in the
  /// inbound model; always initialize at Minimizer, even if
  /// overridden later
  size_t numUserPrimaryFns;
  /// number of objective functions or least squares terms in
  /// iterator's view, after transformations; always initialize at
  /// Minimizer, even if overridden later
  size_t numIterPrimaryFns;

  /// convenience flag for denoting the presence of user-specified bound
  /// constraints.  Used for method selection and error checking.
  bool boundConstraintFlag;

  bool speculativeFlag; ///< flag for speculative gradient evaluations

  /// flag indicating whether user-supplied calibration data is active
  bool calibrationDataFlag;
  /// Container for experimental data to which to calibrate model 
  /// using least squares or other formulations which minimize SSE
  ExperimentData expData;
  /// number of experiments
  size_t numExperiments; 
  /// number of total calibration terms (sum over experiments of 
  /// number of experimental data per experiment, including field data)
  size_t numTotalCalibTerms;
  /// Shallow copy of the data transformation model, when present
  /// (cached in case further wrapped by other transformations)
  Model dataTransformModel; 

  /// whether Iterator-level scaling is active
  bool scaleFlag;
  /// Shallow copy of the scaling transformation model, when present
  /// (cached in case further wrapped by other transformations)
  Model scalingModel;

  /// pointer to Minimizer used in static member functions
  static Minimizer* minimizerInstance;
  /// pointer containing previous value of minimizerInstance
  Minimizer* prevMinInstance;

  /// convenience flag for gradient_type == numerical && method_source == vendor
  bool vendorNumericalGradFlag;

  /// Emerging helper class for handling data transfers to/from Dakota and the underlying TPL
  std::shared_ptr<TPLDataTransfer> dataTransferHandler;

private:

  //
  //- Heading: Convenience/Helper functions
  //

  //
  //- Heading: Data
  //

};


inline Minimizer::Minimizer(std::shared_ptr<TraitsBase> traits): 
  Iterator(traits), calibrationDataFlag(false), scaleFlag(false)
{ }


inline Minimizer::~Minimizer() { }


inline void Minimizer::constraint_tolerance(Real constr_tol)
{ constraintTol = constr_tol; }


inline Real Minimizer::constraint_tolerance() const
{ return constraintTol; }


/** default definition that gets redefined in selected derived Minimizers */
inline const Model& Minimizer::algorithm_space_model() const
{ return iteratedModel; }


inline void Minimizer::enforce_null_model()
{
  // This function is only for updates in "user functions" mode (NPSOL & OPT++)
  if (!iteratedModel.is_null()) {
    Cerr << "Error: callback updaters should not be used when Model data "
	 << "available." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


inline void Minimizer::reshape_best(size_t num_cv, size_t num_fns)
{
  // This function is only for updates in "user functions" mode (NPSOL & OPT++)
  size_t i, num_best = bestVariablesArray.size();
  if (bestResponseArray.size() != num_best) {
    Cerr << "Error: inconsistent best array sizing in Minimizer::"
	 << "reshape_best()." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // assume single SVD shared among all enries in bestVariablesArray
  const SharedVariablesData& old_svd
    = bestVariablesArray.front().shared_data();
  SizetArray vc_totals = old_svd.components_totals(); // copy
  vc_totals[TOTAL_CDV] = num_cv; // update
  // SVD has a limited API, so rather than copy+reshape, build a new one
  SharedVariablesData new_svd(old_svd.view(), vc_totals,
			      old_svd.all_relaxed_discrete_int(),
			      old_svd.all_relaxed_discrete_real());
  for (i=0; i<num_best; ++i) {
    bestVariablesArray[i].reshape(new_svd); // pulls sizing from new svd
    bestResponseArray[i].reshape(num_fns, num_cv, false, false);
  }
}


//inline void Minimizer::initialize_iterator(int job_index)
//{ } // default = no-op


/** This virtual function redefinition is executed on the dedicated master
    processor for self scheduling.  It is not used for peer partitions. */
//inline void Minimizer::
//pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index)
//{ } // default = no-op


/** This virtual function redefinition is executed on an iterator server for
    dedicated master self scheduling.  It is not used for  peer partitions. */
//inline void Minimizer::
//unpack_parameters_initialize(MPIUnpackBuffer& recv_buffer)
//{ } // default = no-op


/** This virtual function redefinition is executed either on an iterator
    server for dedicated master self scheduling or on peers 2 through n
    for static scheduling. */
//inline void Minimizer::
//pack_results_buffer(MPIPackBuffer& send_buffer, int job_index)
//{ } // default = no-op


/** This virtual function redefinition is executed on an environment master
    (either the dedicated master processor for self scheduling or peer 1
    for static scheduling). */
//inline void Minimizer::
//unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
//{ } // default = no-op


//inline void Minimizer::update_local_results(int job_index)
//{ } // default = no-op

} // namespace Dakota

#endif
