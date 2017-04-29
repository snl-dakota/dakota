/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Minimizer
//- Description:  Base class for the optimizer and least squares branches
//-               of the iterator hierarchy.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaMinimizer.hpp 7018 2010-10-12 02:25:22Z mseldre $

#ifndef DAKOTA_MINIMIZER_H
#define DAKOTA_MINIMIZER_H

#include "DakotaIterator.hpp"
#include "DakotaResponse.hpp"
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

  //
  //- Heading: Virtual member function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  Minimizer();
  /// standard constructor
  Minimizer(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for "on the fly" instantiations
  Minimizer(unsigned short method_name, Model& model);
  /// alternate constructor for "on the fly" instantiations
  Minimizer(unsigned short method_name, size_t num_lin_ineq, size_t num_lin_eq,
	    size_t num_nln_ineq, size_t num_nln_eq);

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
  Model original_model(unsigned short recasts_left = 0); 

  /// Wrap iteratedModel in a RecastModel that subtracts provided
  /// observed data from the primary response functions (variables and
  /// secondary responses are unchanged)
  void data_transform_model();

  /// Wrap iteratedModel in a RecastModel that performs variable
  /// and/or response scaling
  void scale_model();

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

  /// allocate results arrays and labels for multipoint storage
  void archive_allocate_best(size_t num_points);

  /// archive the best point into the results array
  void archive_best(size_t index, 
		    const Variables& best_vars, const Response& best_resp);
 
  /// Safely resize the best variables array to newsize taking into
  /// account the envelope-letter design pattern and any recasting.
  void resize_best_vars_array(size_t newsize);

  /// Safely resize the best response array to newsize taking into
  /// account the envelope-letter design pattern and any recasting.
  void resize_best_resp_array(size_t newsize);

  /// infers MOO/NLS solution from the solution of a single-objective optimizer
  void local_recast_retrieve(const Variables& vars, Response& response) const;

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

private:

  //
  //- Heading: Convenience/Helper functions
  //

  //
  //- Heading: Data
  //

};


inline Minimizer::Minimizer(): 
  calibrationDataFlag(false), scaleFlag(false)
{ }


inline Minimizer::~Minimizer() { }


inline void Minimizer::constraint_tolerance(Real constr_tol)
{ constraintTol = constr_tol; }


inline Real Minimizer::constraint_tolerance() const
{ return constraintTol; }


/** default definition that gets redefined in selected derived Minimizers */
inline const Model& Minimizer::algorithm_space_model() const
{ return iteratedModel; }


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
