/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Minimizer
//- Description:  Base class for the optimizer and least squares branches
//-               of the iterator hierarchy.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaMinimizer.H 7018 2010-10-12 02:25:22Z mseldre $

#ifndef DAKOTA_MINIMIZER_H
#define DAKOTA_MINIMIZER_H

#include "DakotaIterator.H"
#include "DakotaResponse.H"


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
  //- Heading: Virtual member function redefinitions
  //

protected:

  //
  //- Heading: Constructors and destructor
  //

  Minimizer();             ///< default constructor
  Minimizer(Model& model); ///< standard constructor

  /// alternate constructor for "on the fly" instantiations
  Minimizer(NoDBBaseConstructor, Model& model);
  /// alternate constructor for "on the fly" instantiations
  Minimizer(NoDBBaseConstructor, size_t num_lin_ineq, size_t num_lin_eq,
	    size_t num_nln_ineq, size_t num_nln_eq);

  ~Minimizer();            ///< destructor

  //
  //- Heading: Virtual member function redefinitions
  //

  void initialize_run();
  void finalize_run();

  //
  //- Heading: Convenience/Helper functions
  //

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

  //  conversion helper functions between native/user and scaled/iterator:

  /// variables conversion map for RecastModel used in scaling: transform
  /// variables from scaled to native (user) space
  static void variables_recast(const Variables& scaled_vars, 
			       Variables& native_vars);
  /// secondary response conversion map for RecastModel used in scaling:
  /// transform constraints (fns, grads, Hessians) from native (user) to 
  // iterator space
  static void secondary_resp_recast(const Variables& native_vars,
				    const Variables& scaled_vars,
				    const Response& native_response,
				    Response& scaled_response);
  /// determine if response transformation is needed due to variable
  /// transformations
  bool need_resp_trans_byvars(const ShortArray& asv, int start_index, 
			      int num_resp);
  /// general RealVector mapping from native to scaled variables vectors: 
  RealVector modify_n2s(const RealVector& native_vars,
			     const IntArray& scale_types,
			     const RealVector& multipliers,
			     const RealVector& offsets) const;
  /// general RealVector mapping from scaled to native variables (and values) 
  RealVector modify_s2n(const RealVector& scaled_vars,
			     const IntArray& scale_types,
			     const RealVector& multipliers,
			     const RealVector& offsets) const;
  /// map reponses from native to scaled variable space
  void response_modify_n2s(const Variables& scaled_vars,
			   const Response& native_response,
			   Response& scaled_response,
			   int native_offset, int recast_offset,
			   int num_responses) const;
  /// map responses from scaled to native variable space
  void response_modify_s2n(const Variables& native_vars,
			   const Response& scaled_response,
			   Response& native_response,
			   int scaled_offset, int native_offset,
			   int num_responses) const;
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

  Real constraintTol; ///< optimizer/least squares constraint tolerance

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
  int numNonlinearConstraints;
  /// total number of linear constraints
  int numLinearConstraints;
  /// total number of linear and nonlinear constraints
  int numConstraints;

  /// convenience flag for denoting the presence of user-specified bound
  /// constraints.  Used for method selection and error checking.
  bool boundConstraintFlag;

  bool speculativeFlag; ///< flag for speculative gradient evaluations

  /// number of objective functions or least squares terms in the user's model
  size_t numUserPrimaryFns;
  /// number of objective functions or least squares terms in iterator's view
  size_t numIterPrimaryFns;

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

  /// convenience flag for gradType == numerical && methodSource == vendor
  bool vendorNumericalGradFlag;

private:

  //
  //- Heading: Data
  //
};


inline Minimizer::Minimizer(): scaleFlag(false)
{ }


inline Minimizer::~Minimizer() { }


inline void Minimizer::finalize_run()
{
  // Restore previous object instance in case of recursion.
  minimizerInstance = prevMinInstance;
  Iterator::finalize_run(); // included for completeness
}

} // namespace Dakota

#endif
