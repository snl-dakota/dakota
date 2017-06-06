/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Optimizer
//- Description:  Abstract base class to logically represent a variety
//-               of DAKOTA optimizer objects in a generic fashion.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaOptimizer.hpp 7018 2010-10-12 02:25:22Z mseldre $

#ifndef DAKOTA_OPTIMIZER_H
#define DAKOTA_OPTIMIZER_H

#include "DakotaMinimizer.hpp"

namespace Dakota {


/// Base class for the optimizer branch of the iterator hierarchy.

/** The Optimizer class provides common data and functionality for
    DOTOptimizer, CONMINOptimizer, NPSOLOptimizer, SNLLOptimizer,
    NLPQLPOptimizer, COLINOptimizer, and JEGAOptimizer. */

class Optimizer: public Minimizer
{
public:

  /// Static helper function: third-party opt packages which are not available
  static void not_available(const std::string& package_name);

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  Optimizer();
  /// alternate constructor; accepts a model
  Optimizer(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for "on the fly" instantiations
  Optimizer(unsigned short method_name, Model& model);
  /// alternate constructor for "on the fly" instantiations
  Optimizer(unsigned short method_name, size_t num_cv, size_t num_div,
	    size_t num_dsv, size_t num_drv, size_t num_lin_ineq,
	    size_t num_lin_eq, size_t num_nln_ineq, size_t num_nln_eq);

  /// destructor
  ~Optimizer();

  //
  //- Heading: Virtual member function redefinitions
  //

  void initialize_run();
  void post_run(std::ostream& s);
  void finalize_run();
  void print_results(std::ostream& s);

  //
  //- Heading: Data
  //
  
  /// number of objective functions (iterator view)
  size_t numObjectiveFns;

  /// flag indicating whether local recasting to a single objective is used
  bool localObjectiveRecast;

  /// pointer to Optimizer instance used in static member functions
  static Optimizer* optimizerInstance;
  /// pointer containing previous value of optimizerInstance
  Optimizer* prevOptInstance;

private:

  //
  //- Heading: Convenience/Helper functions
  //

  /// Wrap iteratedModel in a RecastModel that performs (weighted)
  /// multi-objective or sum-of-squared residuals transformation
  void reduce_model(bool local_nls_recast, bool require_hessians);

  /// Recast callback to reduce multiple objectives or residuals to a
  /// single objective, with gradients and Hessians as needed
  static void primary_resp_reducer(const Variables& full_vars,
				   const Variables& reduced_vars,
				   const Response& full_response,
				   Response& reduced_response);

  /// forward mapping: maps multiple primary response functions to a single
  /// weighted objective for single-objective optimizers
  void objective_reduction(const Response& full_response,
			   const BoolDeque& sense, const RealVector& full_wts,
			   Response& reduced_response) const;

  //
  //- Heading: Data
  //
};


inline Optimizer::Optimizer(): localObjectiveRecast(false)
{ }


inline Optimizer::~Optimizer()
{ }


inline void Optimizer::finalize_run()
{
  // Restore previous object instance in case of recursion.
  optimizerInstance = prevOptInstance;

  Minimizer::finalize_run();
}


inline void Optimizer::not_available(const std::string& package_name)
{
  Cerr << package_name << " is not available.\n";
  abort_handler(-1);
}


//----------------------------------------------------------------

// Data utilities supporting Opt TPL refactor which may eventually be promoted
// to a more generally accessible location - RWH
template <typename VectorType1, typename VectorType2, typename SetArray>
void copy_data( const VectorType1 & source, 
                const BitArray & set_bits,
                const SetArray& set_vars, 
                      VectorType2 & dest, 
                      size_t offset,
                      size_t len)
{
  size_t i, index, set_cntr;

  for(i=0, set_cntr=0; i<len; ++i)
  { 
    if (set_bits[i])
    {
      index = set_value_to_index(source[i], set_vars[set_cntr]);
      if (index == _NPOS) {
        Cerr << "\ncopy_data Error: bad index in discrete set lookup." << std::endl;
        abort_handler(-1);
      }
      else
	dest[i+offset] = (int)index;

      ++set_cntr;
    }
    else 
      dest[i+offset] = source[i];
  }
}


//----------------------------------------------------------------


template <typename VectorType1, typename VectorType2, typename SetArray>
void copy_data( const VectorType1 & source, 
                const SetArray& set_vars, 
                      VectorType2 & dest, 
                      size_t offset,
                      size_t len)
{
  size_t i, index;
  for(i=0; i<len; ++i)
  { 
    index = set_value_to_index(source[i], set_vars[i]);
    if (index == _NPOS) {
      Cerr << "\ncopy_data Error: bad index in discrete set lookup." << std::endl;
      abort_handler(-1);
    }
    else
      dest[i+offset] = (int)index;
  }
}

//----------------------------------------------------------------

template <typename VectorType1, typename VectorType2>
void copy_data( const VectorType1 & source, 
                      VectorType2 & dest, 
                const BitArray & int_set_bits, 
                const IntSetArray& set_int_vars, 
                size_t offset,
                size_t len)
{
  size_t i, dsi_cntr;
  for(i=0, dsi_cntr=0; i<len; ++i)
  { 
    // This active discrete int var is a set type
    // Map from index back to value.
    if (int_set_bits[i])
      dest[i] = set_index_to_value(source[i+offset], set_int_vars[dsi_cntr++]);

    // This active discrete int var is a range type
    else
      dest[i] = source[i+offset];
  }
}

//----------------------------------------------------------------

/// Data adapter for use by third-party opt packages to transfer response data to Dakota
template <typename AdapterT>
void set_best_responses( typename AdapterT::OptT & optimizer,
                         const Model & model,
                         const std::vector<int> constraintMapIndices, // need to move this to traits or similar
                         const std::vector<double> constraintMapMultipliers, // need to move this to traits or similar
                         const std::vector<double> constraintMapOffsets, // need to move this to traits or similar
                               ResponseArray & response_array)
{
  RealVector best_fns(model.num_functions());

  size_t numNlEqCons = model.num_nonlinear_eq_constraints();
  size_t numNlIneqCons = model.num_nonlinear_ineq_constraints();

  // Get best Objective - assumes single objective only for now
  std::vector<double> bestEqs(numNlEqCons);
  std::vector<double> bestIneqs(constraintMapIndices.size()-numNlEqCons);
  const BoolDeque& max_sense = model.primary_response_fn_sense();
  best_fns[0] = (!max_sense.empty() && max_sense[0]) ?  -AdapterT::getBestObj(optimizer) : AdapterT::getBestObj(optimizer);

  // Get best Nonlinear Equality Constraints
  if (numNlEqCons > 0) {
    optimizer.getBestNonlEqs(bestEqs); // we leave this method name the same for now but could generalize depending on other TPLs
    for (size_t i=0; i<numNlEqCons; i++)
      // Need to figure out how best to generalize use of 2 index arrays, 1 value array and the expression - could use lambdas with c++11
      best_fns[constraintMapIndices[i]+1] = (bestEqs[i]-constraintMapOffsets[i]) / constraintMapMultipliers[i];
  }

  // Get best Nonlinear Inequality Constraints
  if (numNlIneqCons > 0) {
    optimizer.getBestNonlIneqs(bestIneqs); // we leave this method name the same for now but could generalize depending on other TPLs
    for (size_t i=0; i<bestIneqs.size(); i++)
      // Need to figure out how best to generalize use of 2 index arrays, 1 value array and the expression - could use lambdas with c++11
      best_fns[constraintMapIndices[i+numNlEqCons]+1] = 
        (bestIneqs[i]-constraintMapOffsets[i+numNlEqCons]) / constraintMapMultipliers[i+numNlEqCons];
  }
  response_array.front().function_values(best_fns);
}

//----------------------------------------------------------------

/// copy appropriate slices of source vector to Dakota::Variables
template <typename VectorType>
void set_variables( const VectorType & source,
                          Model & model,
                          Variables & vars)
{
  int num_cont_vars = vars.cv();
  int num_disc_int_vars = vars.div();
  int num_disc_real_vars = vars.drv();
  int num_disc_string_vars = vars.dsv();

  const BitArray& int_set_bits = model.discrete_int_sets();
  const IntSetArray& set_int_vars = model.discrete_set_int_values();
  const RealSetArray& set_real_vars = model.discrete_set_real_values();
  const StringSetArray& set_string_vars = model.discrete_set_string_values();

  RealVector contVars(num_cont_vars);
  IntVector  discIntVars(num_disc_int_vars);
  RealVector discRealVars(num_disc_real_vars);

  size_t i, dsi_cntr;

  copy_data_partial(source, contVars, 0, num_cont_vars);
  vars.continuous_variables(contVars);

  copy_data(source, discIntVars, int_set_bits, set_int_vars, num_cont_vars, num_disc_int_vars);
  vars.discrete_int_variables(discIntVars);

  // Does this work for more than one discrete Real variables set? - RWH
  for (i=0; i<num_disc_real_vars; i++)
    discRealVars[i] = set_index_to_value(source[i+num_cont_vars+num_disc_int_vars], set_real_vars[i]);
  vars.discrete_real_variables(discRealVars);

  for (i=0; i<num_disc_string_vars; i++)
    vars.discrete_string_variable(set_index_to_value(source[i+num_cont_vars+num_disc_int_vars+num_disc_real_vars], set_string_vars[i]), i);
}

//----------------------------------------------------------------

/// copy the various pieces comprising Dakota::Variables into a concatenated TPL vector
template <typename VectorType>
void get_variables( Model & model,
                    VectorType & vec)
{
  const RealVector& cvars = model.continuous_variables();
  const IntVector& divars = model.discrete_int_variables();
  const RealVector& drvars = model.discrete_real_variables();
  const StringMultiArrayConstView dsvars = model.discrete_string_variables();

  // Could do a sanity check ?
  if( (model.cv()  !=  cvars.length()) ||
      (model.div() != divars.length()) ||
      (model.drv() != drvars.length()) ||
      (model.dsv() != dsvars.size())   )
  {
    Cerr << "\nget_variables Error: model variables have inconsistent lengths." << std::endl;
    abort_handler(-1);
  }

  const BitArray& int_set_bits = model.discrete_int_sets();
  const IntSetArray& pt_set_int = model.discrete_set_int_values();
  const RealSetArray& pt_set_real = model.discrete_set_real_values();
  const StringSetArray& pt_set_string = model.discrete_set_string_values();

  int offset = 0;
  copy_data(cvars, vec);

  offset = cvars.length();
  copy_data(divars, int_set_bits, pt_set_int, vec, offset, divars.length());

  offset += divars.length();
  copy_data(drvars, pt_set_real, vec, offset, drvars.length());

  offset = drvars.length();
  copy_data(dsvars, pt_set_string, vec, offset, dsvars.size());
}

//----------------------------------------------------------------

template <typename OrdinalType, typename ScalarType, typename VectorType2, typename MaskType, typename SetArray>
bool get_mixed_bounds( const MaskType& mask_set,
                       const SetArray& source_set,
                       const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& lower_source,
                       const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& upper_source,
                       VectorType2& lower_target,
                       VectorType2& upper_target,
                       ScalarType bigBoundSize,
                       ScalarType no_value,
                       int target_offset = 0)
{
  bool allSet = true;
  size_t i, set_cntr, len = lower_source.length();

  for(i=0, set_cntr=0; i<len; ++i)
  {
    if (mask_set[i]) {
      lower_target[i+target_offset] = 0;
      upper_target[i+target_offset] = source_set[set_cntr].size() - 1;
      ++set_cntr;
    }
    else 
    {
      if (lower_source[i] > -bigBoundSize)
        lower_target[i+target_offset] = lower_source[i];
      else
      {
        lower_target[i] = no_value;
        allSet = false;
      }
      if (upper_source[i] < bigBoundSize)
        upper_target[i+target_offset] = upper_source[i];
      else
      {
        upper_target[i] = no_value;
        allSet = false;
      }
    }
  }

  return allSet;
}

//----------------------------------------------------------------

template <typename OrdinalType, typename ScalarType, typename VectorType2>
bool get_bounds( const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& lower_source,
                 const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& upper_source,
                       VectorType2& lower_target,
                       VectorType2& upper_target,
                       ScalarType bigBoundSize,
                       ScalarType no_value,
                       int target_offset = 0)
{
  bool allSet = true;

  OrdinalType len = lower_source.length();
  for (OrdinalType i=0; i<len; i++) {
    if (lower_source[i] > -bigBoundSize)
      lower_target[i] = lower_source[i];
    else {
      lower_target[i] = no_value;
      allSet = false;
    }
    if (upper_source[i] < bigBoundSize)
      upper_target[i] = upper_source[i];
    else {
      upper_target[i] = no_value;
      allSet = false;
    }
  }

  return allSet;
}

//----------------------------------------------------------------

template <typename SetArray, typename VectorType>
void get_bounds( const SetArray& source_set,
                       VectorType& lower_target,
                       VectorType& upper_target,
                       int target_offset = 0)
{
  for (size_t i=0; i<source_set.size(); ++i) {
    lower_target[i+target_offset] = 0;
    upper_target[i+target_offset] = source_set[i].size() - 1;
  }
}

//----------------------------------------------------------------

/// Data adapter to transfer data from Dakota to third-party opt packages
template <typename vectorType>
void get_responses( const Model & model,
                    const RealVector & dak_fn_vals,
                    const std::vector<int> constraintMapIndices, // need to move this to traits or similar
                    const std::vector<double> constraintMapMultipliers, // need to move this to traits or similar
                    const std::vector<double> constraintMapOffsets, // need to move this to traits or similar
                    vectorType & f_vec, 
                    vectorType & cEqs_vec, 
                    vectorType & cIneqs_vec)
{
  size_t numNlEqCons = model.num_nonlinear_eq_constraints();

  // Copy Objective - assumes single objective only for now
  f_vec.resize(1);
  const BoolDeque& max_sense = model.primary_response_fn_sense();
  f_vec[0] = (!max_sense.empty() && max_sense[0]) ? -dak_fn_vals[0] : dak_fn_vals[0];

  // Get best Nonlinear Equality Constraints - see comments in set_best_responses 
  cEqs_vec.resize(numNlEqCons);
  for (int i=0; i<cEqs_vec.size(); i++)
    cEqs_vec[i] = constraintMapOffsets[i] +
      constraintMapMultipliers[i]*dak_fn_vals[constraintMapIndices[i]+1];

  // Get best Nonlinear Equality Constraints - see comments in set_best_responses 
  cIneqs_vec.resize(constraintMapIndices.size()-numNlEqCons);
  for (int i=0; i<cIneqs_vec.size(); i++)
    cIneqs_vec[i] = constraintMapOffsets[i+numNlEqCons] +
      constraintMapMultipliers[i+numNlEqCons] * 
      dak_fn_vals[constraintMapIndices[i+numNlEqCons]+1];
}

//----------------------------------------------------------------

/// copy the various pieces comprising bounds on Dakota::Variables into concatenated TPL vectors
template <typename AdapterT>
bool get_bounds( Model & model,
                 Real bigRealBoundSize, // It would be nice to clean this up and not need to pass in
                 int bigIntBoundSize, // It would be nice to clean this up and not need to pass in
                 typename AdapterT::VecT & lower,
                 typename AdapterT::VecT & upper)
{
  const RealVector& lower_bnds_cont = model.continuous_lower_bounds();
  const RealVector& upper_bnds_cont = model.continuous_upper_bounds();

  const IntVector& lower_bnds_int = model.discrete_int_lower_bounds();
  const IntVector& upper_bnds_int = model.discrete_int_upper_bounds();

  const RealVector& lower_bnds_real = model.discrete_real_lower_bounds();
  const RealVector& upper_bnds_real = model.discrete_real_upper_bounds();

  const BitArray& int_set_bits = model.discrete_int_sets();
  const IntSetArray& init_pt_set_int = model.discrete_set_int_values();
  const RealSetArray& init_pt_set_real = model.discrete_set_real_values();
  const StringSetArray& init_pt_set_string = model.discrete_set_string_values();

  // Sanity checks ?

  int offset = 0;
  bool allSet = get_bounds(lower_bnds_cont, upper_bnds_cont,
                           lower, upper,
                           bigRealBoundSize,
                           AdapterT::noValue(),
                           offset);

  offset = model.cv();
  allSet = allSet && 
              get_mixed_bounds
                (int_set_bits, init_pt_set_int, lower_bnds_int, upper_bnds_int, 
                 lower, upper, bigIntBoundSize, (int)AdapterT::noValue(), offset);

  offset += model.div();
  get_bounds(init_pt_set_real, lower, upper, offset);

  offset += model.drv();
  get_bounds(init_pt_set_string, lower, upper, offset);

  return allSet;
}

//----------------------------------------------------------------

/// copy the various data associated with linear constraints from Dakota into TPL vectors/matrices
template <typename AdapterT>
void get_linear_constraints( Model & model,
                             Real bigRealBoundSize, // It would be nice to clean this up and not need to pass in
                             typename AdapterT::VecT & lin_ineq_lower_bnds,
                             typename AdapterT::VecT & lin_ineq_upper_bnds,
                             typename AdapterT::VecT & lin_eq_targets,
                             typename AdapterT::MatT & lin_ineq_coeffs,
                             typename AdapterT::MatT & lin_eq_coeffs)
{
  const RealMatrix& linear_ineq_coeffs = model.linear_ineq_constraint_coeffs();
  const RealVector& linear_ineq_lower_bnds = model.linear_ineq_constraint_lower_bounds();
  const RealVector& linear_ineq_upper_bnds = model.linear_ineq_constraint_upper_bounds();
  const RealMatrix& linear_eq_coeffs = model.linear_eq_constraint_coeffs();
  const RealVector& linear_eq_targets = model.linear_eq_constraint_targets();

  // These are special cases involving Matrices which gets delegated to the adapter for now
  AdapterT::copy_data(linear_ineq_coeffs, lin_ineq_coeffs);
  AdapterT::copy_data(linear_eq_coeffs, lin_eq_coeffs);

  get_bounds(linear_ineq_lower_bnds, linear_ineq_upper_bnds,
              lin_ineq_lower_bnds, lin_ineq_upper_bnds,
              bigRealBoundSize, // hard-wired to Real type; is more gneral type needed?
              AdapterT::noValue());

  copy_data(linear_eq_targets, lin_eq_targets);
}

} // namespace Dakota

#endif
