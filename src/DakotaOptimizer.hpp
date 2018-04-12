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

/** Adapter for copying initial continuous variables values from a Dakota Model
   into TPL vectors */

template <typename VecT>
void get_initial_values( const Model & model,
                               VecT  & values)
{
  const RealVector& initial_points = model.continuous_variables();

  for(int i=0; i<model.cv(); ++i)
    values[i] = initial_points[i];
}

/** Adapter for copying continuous variables data from Dakota RealVector
   into TPL vectors */

//PDH: At some point, need to get rid of big_real_bound_size.  Was no_value
//specific to APPS?  I think it was.  If so, we should look at how general
//that is across the other TPLs.  It might make more sense to push it back
//down to APPS.

template <typename VecT>
bool get_bounds( const RealVector  & lower_source,
                 const RealVector  & upper_source,
                       VecT        & lower_target,
                       VecT        & upper_target,
                       Real          big_real_bound_size,
                       Real          no_value)
{
  bool allSet = true;

  int len = lower_source.length();
  for (int i=0; i<len; i++) {
    if (lower_source[i] > -big_real_bound_size)
      lower_target[i] = lower_source[i];
    else {
      lower_target[i] = no_value;
      allSet = false;
    }
    if (upper_source[i] < big_real_bound_size)
      upper_target[i] = upper_source[i];
    else {
      upper_target[i] = no_value;
      allSet = false;
    }
  }

  return allSet;
}

/** Adapter for copying continuous variables data from a Dakota Model
   into TPL vectors */

template <typename VecT>
void get_bounds( const Model & model,
                       VecT  & lower_target,
                       VecT  & upper_target)
{
  const RealVector& c_l_bnds = model.continuous_lower_bounds();
  const RealVector& c_u_bnds = model.continuous_upper_bounds();

  for( int i=0; i<c_l_bnds.length(); ++i )
  {
    lower_target[i] = c_l_bnds[i];
    upper_target[i] = c_u_bnds[i];
  }
}

/** Adapter originating from (and somewhat specialized based on)
   APPSOptimizer for copying discrete variables from a set-based Dakota
   container into TPL vectors */

//PDH: I think the target_offset can be eliminated from the argument list
//by using a trait specifying expected order.

template <typename SetT, typename VecT>
void get_bounds( const SetT & source_set,
                       VecT & lower_target,
                       VecT & upper_target,
                       int    target_offset)
{
  for (size_t i=0; i<source_set.size(); ++i) {
    lower_target[i+target_offset] = 0;
    upper_target[i+target_offset] = source_set[i].size() - 1;
  }
}

/** Adapter originating from (and somewhat specialized based on)
   APPSOptimizer for copying discrete integer variables data
   with bit masking from Dakota into TPL vectors */

//PDH: Same comments as above for bigBoundSize, no_value, target_offset.

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

/** Adapter originating from (and somewhat specialized based on)
    APPSOptimizer for copying heterogeneous bounded data from 
    Dakota::Variables into concatenated TPL vectors */

//PDH: Same for big_real_bound_size, big_int_bound_size.

template <typename AdapterT>
bool get_variable_bounds( Model &                   model, // would like to make const but cannot due to discrete_int_sets below
                          Real                      big_real_bound_size,
                          int                       big_int_bound_size,
                          typename AdapterT::VecT & lower,
                          typename AdapterT::VecT & upper)
{
  const RealVector& lower_bnds_cont = model.continuous_lower_bounds();
  const RealVector& upper_bnds_cont = model.continuous_upper_bounds();

  const IntVector& lower_bnds_int = model.discrete_int_lower_bounds();
  const IntVector& upper_bnds_int = model.discrete_int_upper_bounds();

  const RealVector& lower_bnds_real = model.discrete_real_lower_bounds();
  const RealVector& upper_bnds_real = model.discrete_real_upper_bounds();

  const BitArray& int_set_bits = model.discrete_int_sets(); // appears to be able to modify the model object ...
  const IntSetArray& init_pt_set_int = model.discrete_set_int_values();
  const RealSetArray& init_pt_set_real = model.discrete_set_real_values();
  const StringSetArray& init_pt_set_string = model.discrete_set_string_values();

  // Sanity checks ?

  bool allSet = get_bounds(lower_bnds_cont,
                           upper_bnds_cont,
                           lower,
                           upper,
                           big_real_bound_size,
                           AdapterT::noValue());

  int offset = model.cv();
  allSet = allSet && 
           get_mixed_bounds( 
                   int_set_bits,
                   init_pt_set_int,
                   lower_bnds_int,
                   upper_bnds_int,
                   lower,
                   upper,
                   big_int_bound_size,
                   (int)AdapterT::noValue(),
                   offset);

  offset += model.div();
  get_bounds(init_pt_set_real, lower, upper, offset);

  offset += model.drv();
  get_bounds(init_pt_set_string, lower, upper, offset);

  return allSet;
}

/** Adapter for configuring inequality constraint maps used when
   transferring data between Dakota and a TPL */

//PDH: Yes, scaling should be tied to a trait.  And get rid of big_real...

template <typename RVecT, typename IVecT>
int configure_inequality_constraint_maps(
                               const Model & model,
                               Real big_real_bound_size,
                               CONSTRAINT_TYPE ctype,
                               IVecT & map_indices,
                               RVecT & map_multipliers,
                               RVecT & map_offsets,
                               Real scaling = 1.0 /* should this be tied to a trait ? RWH */)
{
  const RealVector& ineq_lwr_bnds = ( ctype == CONSTRAINT_TYPE::NONLINEAR ) ?
                                        model.nonlinear_ineq_constraint_lower_bounds() :
                                        model.linear_ineq_constraint_lower_bounds();
  const RealVector& ineq_upr_bnds = ( ctype == CONSTRAINT_TYPE::NONLINEAR ) ?
                                        model.nonlinear_ineq_constraint_upper_bounds() :
                                        model.linear_ineq_constraint_upper_bounds();
  int num_ineq_constr             = ( ctype == CONSTRAINT_TYPE::NONLINEAR ) ?
                                        model.num_nonlinear_ineq_constraints() :
                                        model.num_linear_ineq_constraints();

  int num_added = 0;

  for (int i=0; i<num_ineq_constr; i++) {
    if (ineq_lwr_bnds[i] > -big_real_bound_size) {
      num_added++;
      map_indices.push_back(i);
      map_multipliers.push_back(scaling);
      map_offsets.push_back(-scaling*ineq_lwr_bnds[i]);
    }
    if (ineq_upr_bnds[i] < big_real_bound_size) {
      num_added++;
      map_indices.push_back(i);
      map_multipliers.push_back(-scaling);
      map_offsets.push_back(scaling*ineq_upr_bnds[i]);
    }
  }
  return num_added;
}

/** Adapter for configuring equality constraint maps used when
   transferring data between Dakota and a TPL */

//PDH: I'll have to remind myself what the call chain is.  Ultimately,
//make_one_sided should be tied to a trait somewhere along the line.
//Same for the map-related vectors (e.g., multipliers).

template <typename RVecT, typename IVecT>
void configure_equality_constraint_maps(
                               Model & model,
                               CONSTRAINT_TYPE ctype,
                               IVecT & indices,
                               size_t index_offset,
                               RVecT & multipliers,
                               RVecT & values,
                               bool make_one_sided)
{
  const RealVector& eq_targets = ( ctype == CONSTRAINT_TYPE::NONLINEAR ) ?
                                     model.nonlinear_eq_constraint_targets() :
                                     model.linear_eq_constraint_targets();
  int num_eq                   = ( ctype == CONSTRAINT_TYPE::NONLINEAR ) ?
                                     model.num_nonlinear_eq_constraints() :
                                     model.num_linear_eq_constraints();

  if( make_one_sided )
  {
    for (int i=0; i<num_eq; i++) {
      indices.push_back(i+index_offset);
      multipliers.push_back(-1.0);
      values.push_back(eq_targets[i]);
      indices.push_back(i+index_offset);
      multipliers.push_back(1.0);
      values.push_back(-eq_targets[i]);
    }
  }
  else // leave as two-sided
  {
    for (int i=0; i<num_eq; i++) {
      indices.push_back(i+index_offset);
      multipliers.push_back(1.0);
      values.push_back(-eq_targets[i]);
    }
  }
}

/** Adapter based initially on APPSOptimizer for linear constraint
   maps and including matrix and bounds data;
       * bundles a few steps together which could (should?) be broken
         into two or more adapters */

template <typename AdapterT>
void get_linear_constraints( Model & model,
                             Real big_real_bound_size,
                             typename AdapterT::VecT & lin_ineq_lower_bnds,
                             typename AdapterT::VecT & lin_ineq_upper_bnds,
                             typename AdapterT::VecT & lin_eq_targets,
                             typename AdapterT::MatT & lin_ineq_coeffs,
                             typename AdapterT::MatT & lin_eq_coeffs)
{
  const RealMatrix& linear_ineq_coeffs     = model.linear_ineq_constraint_coeffs();
  const RealVector& linear_ineq_lower_bnds = model.linear_ineq_constraint_lower_bounds();
  const RealVector& linear_ineq_upper_bnds = model.linear_ineq_constraint_upper_bounds();
  const RealMatrix& linear_eq_coeffs       = model.linear_eq_constraint_coeffs();
  const RealVector& linear_eq_targets      = model.linear_eq_constraint_targets();

  // These are special cases involving matrices which get delegated to the adapter for now
  AdapterT::copy_matrix_data(linear_ineq_coeffs, lin_ineq_coeffs);
  AdapterT::copy_matrix_data(linear_eq_coeffs,   lin_eq_coeffs);

  get_bounds(linear_ineq_lower_bnds,
             linear_ineq_upper_bnds,
             lin_ineq_lower_bnds,
             lin_ineq_upper_bnds,
             big_real_bound_size,
             AdapterT::noValue());

  copy_data(linear_eq_targets, lin_eq_targets);
}

//----------------------------------------------------------------

/** Data adapter to transfer data from Dakota to third-party opt
    packages.  The vector values might contain additional constraints;
    the first entries corresponding to linear constraints are
    populated by apply. */
template <typename VecT>
void apply_linear_constraints( const Model & model,
                               CONSTRAINT_EQUALITY_TYPE etype,
                               const VecT & in_vals,
			       VecT & values,
			       bool adjoint = false)
{
  size_t num_linear_consts      = ( etype == CONSTRAINT_EQUALITY_TYPE::EQUALITY ) ?
                                              model.num_linear_eq_constraints() :
                                              model.num_linear_ineq_constraints();
  const RealMatrix & lin_coeffs = ( etype == CONSTRAINT_EQUALITY_TYPE::EQUALITY ) ?
                                              model.linear_eq_constraint_coeffs() :
                                              model.linear_ineq_constraint_coeffs();

  apply_matrix_partial(lin_coeffs, in_vals, values);

  if( etype == CONSTRAINT_EQUALITY_TYPE::EQUALITY )
  {
    const RealVector & lin_eq_targets = model.linear_eq_constraint_targets();
    for(size_t i=0;i<num_linear_consts;++i)
      values[i] -= lin_eq_targets(i);
  }
}

//----------------------------------------------------------------

/** Data adapter to transfer data from Dakota to third-party opt packages

    If adjoint = false, (perhaps counter-intuitively) apply the
    Jacobian (transpose of the gradient) to in_vals, which should be
    of size num_continuous_vars: J*x = G'*x, resulting in
    num_nonlinear_const values getting populated (possibly a subset of
    the total constraint vector).

    If adjoint = true, apply the adjoint Jacobian (gradient) to the
    nonlinear constraint portion of in_vals, which should be of size
    at least num_nonlinear_consts: J'*y = G*y, resulting in
    num_continuous_vars values getting populated.
*/
template <typename VecT>
void apply_nonlinear_constraints( const Model & model,
                               CONSTRAINT_EQUALITY_TYPE etype,
                               const VecT & in_vals,
				  VecT & values ,
				  bool adjoint = false)
{
  size_t num_resp = 1; // does this need to be generalized to more than one response value? - RWH

  size_t num_continuous_vars         = model.cv();

  size_t num_linear_consts           = ( etype == CONSTRAINT_EQUALITY_TYPE::EQUALITY ) ?
                                                   model.num_linear_eq_constraints() :
                                                   model.num_linear_ineq_constraints();
  size_t num_nonlinear_consts        = ( etype == CONSTRAINT_EQUALITY_TYPE::EQUALITY ) ?
                                                   model.num_nonlinear_eq_constraints() :
                                                   model.num_nonlinear_ineq_constraints();

  const RealMatrix & gradient_matrix = model.current_response().function_gradients();

  int grad_offset = ( etype == CONSTRAINT_EQUALITY_TYPE::EQUALITY ) ?
                                                   num_resp + model.num_nonlinear_ineq_constraints() :
                                                   num_resp;

  if (adjoint)
    for (size_t i=0; i<num_continuous_vars; ++i) {
      // BMA --> RWH: can't zero in case compounding linear and nonlinear (usability issue)
      // values[i] = 0.0;
      for(size_t j=0; j<num_nonlinear_consts; ++j)
	values[i] += gradient_matrix(i, grad_offset+j) * in_vals[num_linear_consts+j];
    }
  else
    for(size_t j=0; j<num_nonlinear_consts; ++j) {
      values[num_linear_consts+j] = 0.0;
      for (size_t i=0; i<num_continuous_vars; i++)
	values[num_linear_consts+j] += gradient_matrix(i, grad_offset+j) * in_vals[i];
    }
}

//----------------------------------------------------------------

/// Base class for the optimizer branch of the iterator hierarchy.

/** The Optimizer class provides common data and functionality for
    DOTOptimizer, CONMINOptimizer, NPSOLOptimizer, SNLLOptimizer,
    NLPQLPOptimizer, COLINOptimizer, OptDartsOptimizer, NCSUOptimizer,
    NonlinearCGOptimizer, NomadOptimizer, and JEGAOptimizer. */

class Optimizer: public Minimizer
{
public:

  /// Static helper function: third-party opt packages which are not available
  static void not_available(const std::string& package_name);

//----------------------------------------------------------------

  int num_nonlin_ineq_constraints_found() const
    { return numNonlinearIneqConstraintsFound; }

  /** Adapter for transferring variable bounds from Dakota data to TPL data */
  template <typename AdapterT>
    bool get_variable_bounds_from_dakota(
        typename AdapterT::VecT & lower,
        typename AdapterT::VecT & upper)
    {
      return get_variable_bounds<AdapterT>(
                            iteratedModel,
                            bigRealBoundSize,
                            bigIntBoundSize,
                            lower,
                            upper);
    }

  /** Adapter for transferring responses from Dakota data to TPL data */
  template <typename VecT>
    void get_responses_from_dakota(
        const RealVector & dak_fn_vals,
        VecT & funs, 
        VecT & cEqs, 
        VecT & cIneqs)
    {
      return get_responses( iteratedModel,
                            dak_fn_vals, 
                            constraintMapIndices,
                            constraintMapMultipliers,
                            constraintMapOffsets, 
                            funs,
                            cEqs,
                            cIneqs);
    }
protected:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  Optimizer(std::shared_ptr<TraitsBase> traits);
  /// alternate constructor; accepts a model
  Optimizer(ProblemDescDB& problem_db, Model& model, std::shared_ptr<TraitsBase> traits);

  /// alternate constructor for "on the fly" instantiations
  Optimizer(unsigned short method_name, Model& model, std::shared_ptr<TraitsBase> traits);
  /// alternate constructor for "on the fly" instantiations
  Optimizer(unsigned short method_name, size_t num_cv, size_t num_div,
	    size_t num_dsv, size_t num_drv, size_t num_lin_ineq,
	    size_t num_lin_eq, size_t num_nln_ineq, size_t num_nln_eq, std::shared_ptr<TraitsBase> traits);

  /// destructor
  ~Optimizer();

  //
  //- Heading: Virtual member function redefinitions
  //

  void initialize_run();
  void post_run(std::ostream& s);
  void finalize_run();
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  // helper/adapter methods
  void configure_constraint_maps();
  //void mapped_function_values(const RealVector& function_vals); // use constraints and traits for format
  //const Real& mapped_function_value(size_t i) const;
  
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

  /// number of nonlinear ineq constraints actually used (based on conditional and bigRealBoundSize
  int numNonlinearIneqConstraintsFound;

  /// map from Dakota constraint number to APPS constraint number
  std::vector<int> constraintMapIndices;

  /// multipliers for constraint transformations
  std::vector<double> constraintMapMultipliers;

  /// offsets for constraint transformations
  std::vector<double> constraintMapOffsets;

//----------------------------------------------------------------

  int configure_inequality_constraints( CONSTRAINT_TYPE ctype )
  {
    Real scaling = 1.0;
    if( ctype == CONSTRAINT_TYPE::NONLINEAR )
      scaling = (traits()->nonlinear_inequality_format() == NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_LOWER)
        ? 1.0 : -1.0;
    else if( ctype == CONSTRAINT_TYPE::LINEAR )
      scaling = (traits()->linear_inequality_format() == LINEAR_INEQUALITY_FORMAT::ONE_SIDED_LOWER)
        ? 1.0 : -1.0;
    else
    {
      Cerr << "\nError: inconsistent format for CONSTRAINT_TYPE in configure_inequality_constraints adapter." << std::endl;
      abort_handler(-1);
    }

    return configure_inequality_constraint_maps(
        iteratedModel,
        bigRealBoundSize,
        ctype,
        constraintMapIndices,
        constraintMapMultipliers,
        constraintMapOffsets,
        scaling);
  }

//----------------------------------------------------------------

  void configure_equality_constraints( CONSTRAINT_TYPE ctype, size_t index_offset)
  {
    bool split_into_one_sided = true;
    if( (ctype == CONSTRAINT_TYPE::NONLINEAR) &&
        (traits()->nonlinear_equality_format() == NONLINEAR_EQUALITY_FORMAT::TPL_MANAGED) )
      split_into_one_sided = false;

    return configure_equality_constraint_maps(
        iteratedModel,
        ctype,
        constraintMapIndices,
        index_offset,
        constraintMapMultipliers,
        constraintMapOffsets,
        split_into_one_sided);
  }

//----------------------------------------------------------------

  template <typename AdapterT>
  void get_linear_constraints_and_bounds(
          typename AdapterT::VecT & lin_ineq_lower_bnds,
          typename AdapterT::VecT & lin_ineq_upper_bnds,
          typename AdapterT::VecT & lin_eq_targets,
          typename AdapterT::MatT & lin_ineq_coeffs,
          typename AdapterT::MatT & lin_eq_coeffs)
  {
    return get_linear_constraints<AdapterT>(
        iteratedModel,
        bigRealBoundSize,
        lin_ineq_lower_bnds,
        lin_ineq_upper_bnds,
        lin_eq_targets,
        lin_ineq_coeffs,
        lin_eq_coeffs);
  }

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


inline Optimizer::Optimizer(std::shared_ptr<TraitsBase> traits): Minimizer(traits), localObjectiveRecast(false)
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

//PDH: How much of everything from here on is used by both APPS and ROL?
//Probably need to make a pass to look for possible redundancies and to
//consolidate if/where necessary.

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

/** Data adapter for use by third-party opt packages to transfer response data to Dakota */
template <typename AdapterT>
void set_best_responses( typename AdapterT::OptT & optimizer,
                         const Model & model,
                         const std::vector<int> constraint_map_indices,
                         const std::vector<double> constraint_map_multipliers,
                         const std::vector<double> constraint_map_offsets,
                               ResponseArray & response_array)
{
  RealVector best_fns(model.num_functions());

  size_t num_nl_eq_constr = model.num_nonlinear_eq_constraints();
  size_t num_nl_ineq_constr = model.num_nonlinear_ineq_constraints();

  // Get best Objective - assumes single objective only for now
  std::vector<double> bestEqs(num_nl_eq_constr);
  std::vector<double> bestIneqs(constraint_map_indices.size()-num_nl_eq_constr);
  const BoolDeque& max_sense = model.primary_response_fn_sense();
  best_fns[0] = (!max_sense.empty() && max_sense[0]) ?  -AdapterT::getBestObj(optimizer) : AdapterT::getBestObj(optimizer);

  // Get best Nonlinear Equality Constraints
  if (num_nl_eq_constr > 0) {
    optimizer.getBestNonlEqs(bestEqs); // we leave this method name the same for now but could generalize depending on other TPLs
    for (size_t i=0; i<num_nl_eq_constr; i++)
      // Need to figure out how best to generalize use of 2 index arrays, 1 value array and the expression - could use lambdas with c++11
      best_fns[constraint_map_indices[i]+1] = (bestEqs[i]-constraint_map_offsets[i]) / constraint_map_multipliers[i];
  }

  // Get best Nonlinear Inequality Constraints
  if (num_nl_ineq_constr > 0) {
    optimizer.getBestNonlIneqs(bestIneqs); // we leave this method name the same for now but could generalize depending on other TPLs
    for (size_t i=0; i<bestIneqs.size(); i++)
      // Need to figure out how best to generalize use of 2 index arrays, 1 value array and the expression - could use lambdas with c++11
      best_fns[constraint_map_indices[i+num_nl_eq_constr]+1] = 
        (bestIneqs[i]-constraint_map_offsets[i+num_nl_eq_constr]) / constraint_map_multipliers[i+num_nl_eq_constr];
  }
  response_array.front().function_values(best_fns);
}

//----------------------------------------------------------------

/** copy appropriate slices of source vector to Dakota::Variables */
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

  copy_data_partial(source, 0, contVars, 0, num_cont_vars);
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

/** copy the various pieces comprising Dakota::Variables into a concatenated TPL vector */
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

/** Data adapter to transfer data from Dakota to third-party opt packages */
template <typename vectorType>
void get_responses( const Model & model,
                    const RealVector & dak_fn_vals,
                    const std::vector<int> constraint_map_indices,
                    const std::vector<double> constraint_map_multipliers,
                    const std::vector<double> constraint_map_offsets,
                    vectorType & f_vec, 
                    vectorType & cEqs_vec, 
                    vectorType & cIneqs_vec)
{
  size_t num_nl_eq_constr = model.num_nonlinear_eq_constraints();

  // Copy Objective - assumes single objective only for now
  f_vec.resize(1);
  const BoolDeque& max_sense = model.primary_response_fn_sense();
  f_vec[0] = (!max_sense.empty() && max_sense[0]) ? -dak_fn_vals[0] : dak_fn_vals[0];

  // Get best Nonlinear Equality Constraints - see comments in set_best_responses 
  cEqs_vec.resize(num_nl_eq_constr);
  for (int i=0; i<cEqs_vec.size(); i++)
    cEqs_vec[i] = constraint_map_offsets[i] +
      constraint_map_multipliers[i]*dak_fn_vals[constraint_map_indices[i]+1];

  // Get best Nonlinear Inequality Constraints - see comments in set_best_responses 
  cIneqs_vec.resize(constraint_map_indices.size()-num_nl_eq_constr);
  for (int i=0; i<cIneqs_vec.size(); i++)
    cIneqs_vec[i] = constraint_map_offsets[i+num_nl_eq_constr] +
      constraint_map_multipliers[i+num_nl_eq_constr] * 
      dak_fn_vals[constraint_map_indices[i+num_nl_eq_constr]+1];
}

//----------------------------------------------------------------

/** Data adapter to transfer data from Dakota to third-party opt packages */
template <typename VecT>
void get_nonlinear_eq_constraints( const Model & model,
                                         VecT & values,
                                         Real scale,
                                         int offset = -1 )
{
  if( -1 == offset )
    offset = model.num_linear_eq_constraints();
  size_t num_nonlinear_ineq        = model.num_nonlinear_ineq_constraints();
  size_t num_nonlinear_eq          = model.num_nonlinear_eq_constraints();
  const RealVector& nln_eq_targets = model.nonlinear_eq_constraint_targets();
  const RealVector& curr_resp_vals = model.current_response().function_values();

  for (int i=0; i<num_nonlinear_eq; i++)
    values[i+offset] = curr_resp_vals[i+1+num_nonlinear_ineq] + scale*nln_eq_targets[i];
}

//----------------------------------------------------------------

/** Data adapter to transfer data from Dakota to third-party opt packages */
template <typename VecT>
void get_nonlinear_eq_constraints( Model & model,
                                   const RealVector & curr_resp_vals,
                                         VecT & values,
                                         Real scale,
                                         int offset = 0 )
{
  const RealVector& nln_eq_targets = model.nonlinear_eq_constraint_targets();
  int num_nl_eq_constr             = model.num_nonlinear_eq_constraints();

  for (int i=0; i<num_nl_eq_constr; i++)
    values[i+offset] = curr_resp_vals[i] + scale*nln_eq_targets[i];
}

//----------------------------------------------------------------

/** Data adapter to transfer data from Dakota to third-party opt packages */
template <typename VecT>
void get_nonlinear_ineq_constraints( const Model & model,
                                           VecT & values)
{
  size_t num_nonlinear_ineq        = model.num_nonlinear_ineq_constraints();
  size_t num_linear_ineq           = model.num_linear_ineq_constraints();
  const RealVector& curr_resp_vals = model.current_response().function_values();

  copy_data_partial(curr_resp_vals, 1, values, num_linear_ineq, num_nonlinear_ineq);
}

//----------------------------------------------------------------

///  Would like to combine the previous adapter with this one (based on APPSOptimizer and COLINOptimizer)
///  and then see how much more generalization is needed to support other TPLs like JEGA

/** Data adapter to transfer data from Dakota to third-party opt packages */
template <typename VecT>
void get_nonlinear_constraints( Model & model,
                                VecT & nonlin_ineq_lower,
                                VecT & nonlin_ineq_upper,
                                VecT & nonlin_eq_targets)
{
  const RealVector& nln_ineq_lwr_bnds = model.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& nln_ineq_upr_bnds = model.nonlinear_ineq_constraint_upper_bounds();
  const RealVector& nln_eq_targets    = model.nonlinear_eq_constraint_targets();

  copy_data(nln_ineq_lwr_bnds, nonlin_ineq_lower);
  copy_data(nln_ineq_upr_bnds, nonlin_ineq_upper);
  copy_data(nln_eq_targets   , nonlin_eq_targets);
}

} // namespace Dakota

#endif
