/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef MF_SOLUTION_DATA_H
#define MF_SOLUTION_DATA_H

#include "DataMethod.hpp"


namespace Dakota {

/// Container class for the numerical solution for a given configuration
/// (e.g. ensemble + DAG)

/** Used for caching the optimization solution data that is associated with
    one configuration among multiple possibilities, e.g. ensemble membership
    and directed acyclic graph (DAG) of paired control variates. */

class MFSolutionData
{

public:

  //
  //- Heading: Constructors / destructor
  //

  /// default constructor
  MFSolutionData();
  // full constructor
  MFSolutionData(const RealVector& soln_vars, const RealVector& est_var,
		 const RealVector& est_var_ratios, Real equiv_hf);
  /// copy constructor
  MFSolutionData(const MFSolutionData& sd);
  /// destructor
  ~MFSolutionData();

  /// assignment operator
  MFSolutionData& operator=(const MFSolutionData&);

  //
  //- Heading: Accessors (supporting conversions)
  //

  const RealVector& solution_variables() const;
  Real solution_variable(size_t i) const;
  void solution_variables(const RealVector& soln_vars);
  void solution_variables(const SizetArray& samples);

  std::pair<RealVector, Real> anchored_solution_ratios() const;
  void anchored_solution_ratios(const RealVector& soln_ratios, Real soln_ref);

  RealVector solution_ratios() const;
  Real solution_reference() const;
  
  void initialize_estimator_variances(size_t num_fns);
  void initialize_estimator_variances(size_t num_fns, Real val);
  void estimator_variances(const RealVector& est_var);
  const RealVector& estimator_variances() const;
  void estimator_variance(Real est_var, size_t i);
  Real estimator_variance(size_t i) const;

  static void update_estimator_variance_metric(short metric_type,
					       Real norm_order,
					       const RealVector& est_var_ratios,
					       const RealVector& est_var,
					       Real& est_var_metric,
					       size_t& metric_index);
  static void update_estimator_variance_metric(short metric_type,
					       Real norm_order,
					       const RealVector& est_var,
					       Real& est_var_metric);
  void update_estimator_variance_metric(short metric_type,
					Real norm_order = 2.);
  Real estimator_variance_metric() const;
  void estimator_variance_metric(Real estvar_metric);
  size_t estimator_variance_metric_index() const;

  void initialize_estimator_variance_ratios(size_t num_fns);
  void initialize_estimator_variance_ratios(size_t num_fns, Real val);
  void estimator_variance_ratios(const RealVector& est_var_ratios);
  const RealVector& estimator_variance_ratios() const;
  void estimator_variance_ratio(Real est_var_ratio, size_t i);
  Real estimator_variance_ratio(size_t i) const;

  Real equivalent_hf_allocation() const;
  void equivalent_hf_allocation(Real equiv_hf_alloc);
   
protected:

  //
  //- Heading: Data
  //

  // ***********************
  // optimization variables:
  // ***********************

  // r_i and N_H for model graph:
  //RealVector avgEvalRatios;
  //Real avgHFTarget;

  // N_i and N_H for model graph:
  //RealVector avgApproxSamples;
  //Real        avgTruthSamples;

  // integrated & abstract:
  // > ML BLUE: samples per group (last group is all models)
  // > MFMC_numerical/ACV/GenACV: samples per model (last model is truth)
  RealVector solutionVars;

  // *********************
  // optimization results:
  // *********************
  // Note: could be subclassed for analytic (vector estvar) vs. numerical
  // solutions (averaged estvar)

  /// estimator variances for each QoI for a given model graph
  RealVector estVariances;
  /// estimator variance ratios (1 - R^2) for each QoI for a given model
  /// graph: the ratio of final estimator variances (optimization results)
  /// and final MC estimator variance using final N_H samples (not equivHF)
  RealVector estVarRatios;

  /// scalar metric computed from estVariances or estVarRatios according
  /// to incoming metric type
  Real estVarMetric;
  /// index when estVarMetric involves a selection (e.g. max over QoI)
  /// rather than reduction (e.g. average over QoI)
  size_t estVarMetricIndex;

  // for accuracy constrained: total allocation for estimator in
  // equivalent HF units (may differ from total actual = equivHFEvals)
  Real equivHFAlloc;
};


inline MFSolutionData::MFSolutionData(): //, avgHFTarget(0.)
  estVarMetric(DBL_MAX), equivHFAlloc(0.)
{ }


inline MFSolutionData::
MFSolutionData(const RealVector& soln_vars, const RealVector& est_var,
	       const RealVector& est_var_ratios, Real equiv_hf):
  estVarMetric(DBL_MAX)
{
  copy_data(soln_vars, solutionVars);       copy_data(est_var, estVariances);
  copy_data(est_var_ratios, estVarRatios);  equivHFAlloc = equiv_hf;
}


inline MFSolutionData::MFSolutionData(const MFSolutionData& sd)
{
  copy_data(sd.solutionVars, solutionVars);
  copy_data(sd.estVariances, estVariances);
  copy_data(sd.estVarRatios, estVarRatios);
  //estVarMetricType = sd.estVarMetricType;
  estVarMetric = sd.estVarMetric;
  equivHFAlloc = sd.equivHFAlloc;
}


inline MFSolutionData::~MFSolutionData()
{ }


inline MFSolutionData& MFSolutionData::operator=(const MFSolutionData& sd)
{
  copy_data(sd.solutionVars, solutionVars);
  copy_data(sd.estVariances, estVariances);
  copy_data(sd.estVarRatios, estVarRatios);
  //estVarMetricType = sd.estVarMetricType;
  estVarMetric = sd.estVarMetric;
  equivHFAlloc = sd.equivHFAlloc;
  return *this;
}


inline const RealVector& MFSolutionData::solution_variables() const
{ return solutionVars; }


inline Real MFSolutionData::solution_variable(size_t i) const
{ return solutionVars[i]; }


inline void MFSolutionData::solution_variables(const RealVector& soln_vars)
{ copy_data(soln_vars, solutionVars); }


inline void MFSolutionData::solution_variables(const SizetArray& samples)
{
  size_t i, len = samples.size();
  if (solutionVars.length() != len) solutionVars.sizeUninitialized(len);
  for (i=0; i<len; ++i)
    solutionVars[i] = (Real)samples[i];
}


inline std::pair<RealVector, Real> MFSolutionData::
anchored_solution_ratios() const
{
  RealVector ratios; Real ratio_ref = 0.; int v_len = solutionVars.length();
  if (v_len) {
    int r_len = v_len - 1;
    ratio_ref = solutionVars[r_len];
    copy_data_partial(solutionVars, 0, r_len, ratios);
    ratios.scale(1./ratio_ref);
  }
  return std::pair<RealVector, Real>(ratios, ratio_ref);
}


inline void MFSolutionData::
anchored_solution_ratios(const RealVector& soln_ratios, Real soln_ref)
{
  int r_len = soln_ratios.length(), v_len = r_len + 1;
  if (solutionVars.length() != v_len)
    solutionVars.sizeUninitialized(v_len);
  for (int i=0; i<r_len; ++i)
    solutionVars[i] = soln_ratios[i] * soln_ref;
  solutionVars[r_len] = soln_ref;
}


inline RealVector MFSolutionData::solution_ratios() const
{
  RealVector ratios; int v_len = solutionVars.length();
  if (v_len) {
    int r_len = v_len - 1;
    copy_data_partial(solutionVars, 0, r_len, ratios);
    ratios.scale(1./solutionVars[r_len]);
  }
  return ratios;
}


inline Real MFSolutionData::solution_reference() const
{
  int sv_len = solutionVars.length();
  return (sv_len) ? solutionVars[sv_len - 1] : 0.;
}


inline void MFSolutionData::
initialize_estimator_variances(size_t num_fns)
{ if (estVariances.length()!=num_fns) estVariances.sizeUninitialized(num_fns); }


inline void MFSolutionData::
initialize_estimator_variances(size_t num_fns, Real val)
{
  if (estVariances.length()!=num_fns) estVariances.sizeUninitialized(num_fns);
  estVariances.putScalar(val);
}


inline void MFSolutionData::estimator_variances(const RealVector& est_var)
{ copy_data(est_var, estVariances); }


inline const RealVector& MFSolutionData::estimator_variances() const
{ return estVariances; } // enforce length == numFunctions?


inline void MFSolutionData::estimator_variance(Real est_var, size_t i)
{ estVariances[i] = est_var; }


inline Real MFSolutionData::estimator_variance(size_t i) const
{ return estVariances[i]; }


inline void MFSolutionData::
update_estimator_variance_metric(short metric_type, Real norm_order,
				 const RealVector& est_var_ratios,
				 const RealVector& est_var,
				 Real& metric, size_t& metric_index)
{
  // Another metric to consider would be to move away from a relative accuracy
  // constraint (convTol * estVarIter0) and allow specification of a mean
  // accuracy based on the confidence interval, e.g. user wants the mean value
  // within +/- 1% --> 95% confidence interval half-width / mean estimate < .01,
  // which would shift from relative to absolute when mean estimate ~ 0.

  // Error checks
  bool err_flag = false;
  switch (metric_type) {
  case DEFAULT_ESTVAR_METRIC: case AVG_ESTVAR_METRIC:
  case NORM_ESTVAR_METRIC:    case MAX_ESTVAR_METRIC:
    if (est_var.empty()) {
      Cerr << "Error: estimator variance undefined in MFSolutionData::"
	   << "update_estimator_variance_metric()" << std::endl;
      err_flag = true;
    }
    break;
  case AVG_ESTVAR_RATIO_METRIC:  case NORM_ESTVAR_RATIO_METRIC:
  case MAX_ESTVAR_RATIO_METRIC:
    // Can only omit N* from variance minimization in case where it is fixed
    // (R_ONLY_LINEAR_CONSTRAINT from truth_fixed_by_pilot specification).
    // In this case, minimizing an estvar ratios metric is equivalent to
    // minimizing estvar, albeit more well scaled across QoI.

    // This error check would need to be performed at a higher level scope
    //if (optSubProblemForm != R_ONLY_LINEAR_CONSTRAINT) { // not in scope
    //  Cerr << "Error: minimization metrics defined from estimator variance "
    //       << "ratios require a R_ONLY_LINEAR_CONSTRAINT formulation."
    // 	     << std::endl;
    //  err_flag = true;
    //}
 
    if (est_var_ratios.empty()) {
      Cerr << "Error: estimator variance ratios undefined in MFSolutionData::"
	   << "update_estimator_variance_metric()" << std::endl;
      err_flag = true;
    }
    break;
  default:
    Cerr << "Error: unsupported metric type in MFSolutionData::"
	 << "update_estimator_variance_metric()" << std::endl;
    err_flag = true;  break;
  }
  if (err_flag)
    abort_handler(METHOD_ERROR);

  switch (metric_type) {
  // absolute metrics:
  case DEFAULT_ESTVAR_METRIC: case AVG_ESTVAR_METRIC:
    metric_index = _NPOS;
    metric = average(est_var);                    break;
  case NORM_ESTVAR_METRIC:
    metric_index = _NPOS;
    metric = p_norm(est_var, norm_order);         break;
  case MAX_ESTVAR_METRIC:
    metric_index = find_max_index(est_var);
    metric = est_var[metric_index];               break;
  // metrics relative to MC estvar (see validity note above):
  case AVG_ESTVAR_RATIO_METRIC:
    metric_index = _NPOS;
    metric = average(est_var_ratios);             break;
  case NORM_ESTVAR_RATIO_METRIC:
    metric_index = _NPOS;
    metric = p_norm(est_var_ratios, norm_order);  break;
  case MAX_ESTVAR_RATIO_METRIC:
    metric_index = find_max_index(est_var_ratios);
    metric = est_var_ratios[metric_index];        break;
  }
}


inline void MFSolutionData::
update_estimator_variance_metric(short metric_type, Real norm_order,
				 const RealVector& est_var, Real& metric)
{
  // In current uses, we restrict to estvar metrics even for a metric_type
  // based on estvar ratios

  switch (metric_type) {
  case DEFAULT_ESTVAR_METRIC: case AVG_ESTVAR_METRIC:
  case AVG_ESTVAR_RATIO_METRIC:
    metric = average(est_var);             break;
  case NORM_ESTVAR_METRIC:    case NORM_ESTVAR_RATIO_METRIC:
    metric = p_norm(est_var, norm_order);  break;
  case MAX_ESTVAR_METRIC:     case MAX_ESTVAR_RATIO_METRIC:
    metric = maximum(est_var);             break;
  default:
    Cerr << "Error: unsupported metric type in MFSolutionData::"
	 << "update_estimator_variance_metric()" << std::endl;
    abort_handler(METHOD_ERROR);        break;
  }
}


inline void MFSolutionData::
update_estimator_variance_metric(short metric_type, Real norm_order)
{
  update_estimator_variance_metric(metric_type, norm_order, estVarRatios,
				   estVariances, estVarMetric,
				   estVarMetricIndex);
}


inline Real MFSolutionData::estimator_variance_metric() const
{ return estVarMetric; }


/** This function must be used with care as it can desynchronize the
    scalar estVarMetric from its corresponding estVariances vector. At
    this time, it is only used during solution comparison in numerical
    solves, where we need to store the final scalar value coming from
    the optimizer.  For the solution that is selected, the estVariances
    vector is then synchronized downstream. */
inline void MFSolutionData::estimator_variance_metric(Real estvar_metric)
{ estVarMetric = estvar_metric; }


inline size_t MFSolutionData::estimator_variance_metric_index() const
{ return estVarMetricIndex; }


inline void MFSolutionData::
initialize_estimator_variance_ratios(size_t num_fns)
{ if (estVarRatios.length()!=num_fns) estVarRatios.sizeUninitialized(num_fns); }


inline void MFSolutionData::
initialize_estimator_variance_ratios(size_t num_fns, Real val)
{
  if (estVarRatios.length()!=num_fns) estVarRatios.sizeUninitialized(num_fns);
  estVarRatios.putScalar(val);
}


inline void MFSolutionData::
estimator_variance_ratios(const RealVector& est_var_ratios)
{ copy_data(est_var_ratios, estVarRatios); }


inline const RealVector& MFSolutionData::estimator_variance_ratios() const
{ return estVarRatios; }


inline void MFSolutionData::
estimator_variance_ratio(Real est_var_ratio, size_t i)
{ estVarRatios[i] = est_var_ratio; }


inline Real MFSolutionData::estimator_variance_ratio(size_t i) const
{ return estVarRatios[i]; }


inline Real MFSolutionData::equivalent_hf_allocation() const
{ return equivHFAlloc; }


inline void MFSolutionData::equivalent_hf_allocation(Real equiv_hf_alloc)
{ equivHFAlloc = equiv_hf_alloc; }

} // namespace Dakota

#endif
