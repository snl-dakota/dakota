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

  /// Wrap iteratedModel in a RecastModel that subtracts provided
  /// observed data from the primary response functions (variables and
  /// secondary responses are unchanged)
  bool data_transform_model(bool weight_flag = false);

  /// Wrap iteratedModel in a RecastModel that performs variable
  /// and/or response scaling
  void scale_model();

  /// conversion of request vector values for the Gauss-Newton Hessian
  /// approximation
  static void gnewton_set_recast(const Variables& recast_vars,
				 const ActiveSet& recast_set,
				 ActiveSet& sub_model_set);

  /// conversion of request vector values for Least Squares
  static void replicate_set_recast(const Variables& recast_vars,
				  const ActiveSet& recast_set,
				  ActiveSet& sub_model_set);

  /// copy the partial response for secondary functions when needed
  /// (data and reduction transforms)
  static void secondary_resp_copier(const Variables& input_vars,
				    const Variables& output_vars,
				    const Response& input_response,
				    Response& output_response);

  /// determine if response transformation is needed due to variable
  /// transformations
  bool need_resp_trans_byvars(const ShortArray& asv, int start_index, 
			      int num_resp);

  /// general RealVector mapping from scaled to native variables (and values) 
  RealVector modify_s2n(const RealVector& scaled_vars,
			     const IntArray& scale_types,
			     const RealVector& multipliers,
			     const RealVector& offsets) const;

  /// map responses from scaled to native space
  void response_modify_s2n(const Variables& native_vars,
			   const Response& scaled_response,
			   Response& native_response,
			   int start_offset, int num_responses) const;

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

  /// number of RecastModels locally (in Minimizer or derived classes)
  /// layered over the initially passed in Model
  unsigned short minimizerRecasts;
  /// flag for use where optimization and NLS must be distinguished
  bool optimizationFlag;
  /// number of objective functions or least squares terms in the user's model
  /// always initialize at Minimizer, even if overridden later
  size_t numUserPrimaryFns;
  /// number of objective functions or least squares terms in iterator's view
  /// always initialize at Minimizer, even if overridden later
  size_t numIterPrimaryFns;

  /// convenience flag for denoting the presence of user-specified bound
  /// constraints.  Used for method selection and error checking.
  bool boundConstraintFlag;

  bool speculativeFlag; ///< flag for speculative gradient evaluations

  // TODO: Update from expData to scalarData and handle field data

  /// filename from which to read observed data
  String obsDataFilename;
  /// flag indicating whether user-supplied data is active
  bool obsDataFlag;
  /// Container for experimental data to which to calibrate model 
  /// using least squares or other formulations which minimize SSE
  ExperimentData expData;
  /// number of experiments
  size_t numExperiments; 
  /// number of total rows of data since we are allowing varying 
  /// numbers of experiments and replicates per experiment 
  size_t numRowsExpData; 

  // scaling data follow 
  bool       scaleFlag;              ///< flag for overall scaling status
  bool       varsScaleFlag;          ///< flag for variables scaling
  bool       primaryRespScaleFlag;   ///< flag for primary response scaling
  bool       secondaryRespScaleFlag; ///< flag for secondary response scaling

  IntArray   cvScaleTypes;           ///< scale flags for continuous vars.
  RealVector cvScaleMultipliers;     ///< scales for continuous variables
  RealVector cvScaleOffsets;         ///< offsets for continuous variables

  IntArray   responseScaleTypes;         ///< scale flags for all responses
  RealVector responseScaleMultipliers;   ///< scales for all responses
  RealVector responseScaleOffsets;       ///< offsets for all responses (zero
                                         ///< for functions, not for nonlin con)

  IntArray   linearIneqScaleTypes;       ///< scale flags for linear ineq
  RealVector linearIneqScaleMultipliers; ///< scales for linear ineq constrs.
  RealVector linearIneqScaleOffsets;     ///< offsets for linear ineq constrs.

  IntArray   linearEqScaleTypes;         ///< scale flags for linear eq.
  RealVector linearEqScaleMultipliers;   ///< scales for linear constraints
  RealVector linearEqScaleOffsets;       ///< offsets for linear constraints

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

  /// Recast callback function to difference residuals with observed data
  static void primary_resp_differencer(const Variables& raw_vars, 
				       const Variables& residual_vars,
				       const Response& raw_response, 
				       Response& residual_response);

  /// Core of data difference, which doesn't perform any output
  bool data_difference_core(const Response& raw_response, 
			    Response& residual_response);

  //  scaling initialization helper functions:

  /// initialize scaling types, multipliers, and offsets; perform error
  /// checking
  void initialize_scaling();
  /// general helper function for initializing scaling types and factors on a 
  /// vector of variables, functions, constraints, etc.
  void compute_scaling(int object_type, int auto_type, int num_vars,
		       RealVector& lbs, RealVector& ubs,
		       RealVector& targets,
		       const StringArray& scale_strings, 
		       const RealVector& scales,
		       IntArray& scale_types, RealVector& scale_mults,
		       RealVector& scale_offsets);
  /// automatically compute a single scaling factor -- bounds case
  bool compute_scale_factor(const Real lower_bound, const Real upper_bound,
			    Real *multiplier, Real *offset);
  /// automatically compute a single scaling factor -- target case
  bool compute_scale_factor(const Real target, Real *multiplier);
 
  /// RecastModel callback for variables scaling: transform variables
  /// from scaled to native (user) space
  static void variables_scaler(const Variables& scaled_vars, 
			       Variables& native_vars);

  /// RecastModel callback for primary response scaling: transform
  /// responses (grads, Hessians) from native (user) to scaled space
  static void primary_resp_scaler(const Variables& native_vars, 
				  const Variables& scaled_vars,
				  const Response& native_response, 
				  Response& iterator_response);

  /// RecastModel callback for secondary response scaling: transform
  /// constraints (grads, Hessians) from native (user) to scaled space
  static void secondary_resp_scaler(const Variables& native_vars,
				    const Variables& scaled_vars,
				    const Response& native_response,
				    Response& scaled_response);

  /// Core of response scaling, which doesn't perform any output
  void response_scaler_core(const Variables& native_vars,
			    const Variables& scaled_vars,
			    const Response& native_response,
			    Response& iterator_response,
			    size_t start_offset, size_t num_responses);

  /// general RealVector mapping from native to scaled variables vectors: 
  RealVector modify_n2s(const RealVector& native_vars,
			     const IntArray& scale_types,
			     const RealVector& multipliers,
			     const RealVector& offsets) const;
  /// map reponses from native to scaled variable space
  void response_modify_n2s(const Variables& scaled_vars,
			   const Response& native_response,
			   Response& scaled_response,
			   int start_offset, int num_responses) const;
  /// general linear coefficients mapping from native to scaled space 
  RealMatrix lin_coeffs_modify_n2s(const RealMatrix& native_coeffs, 
    const RealVector& cv_multipliers, const RealVector& lin_multipliers) const;

  /// print scaling information for a particular response type in tabular form
  void print_scaling(const String& info, const IntArray& scale_types,
		     const RealVector& scale_mults,
		     const RealVector& scale_offsets, 
		     const StringArray& labels);

  //
  //- Heading: Data
  //
};


inline Minimizer::Minimizer(): 
  minimizerRecasts(0), obsDataFlag(false), scaleFlag(false)
{ }


inline Minimizer::~Minimizer() { }


inline void Minimizer::constraint_tolerance(Real constr_tol)
{ constraintTol = constr_tol; }


inline Real Minimizer::constraint_tolerance() const
{ return constraintTol; }


inline void Minimizer::finalize_run()
{
  // Restore previous object instance in case of recursion.
  minimizerInstance = prevMinInstance;
  Iterator::finalize_run(); // included for completeness
}


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
