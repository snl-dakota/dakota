/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ParamStudy
//- Description: Parameter study driver program.  This class iterates a
//-              Model object using simple rules, i.e. evaluating
//-              a variety of specified points in the design space.   
//- Owner:       Mike Eldred
//- Version: $Id: ParamStudy.H 7024 2010-10-16 01:24:42Z mseldre $

#ifndef PARAM_STUDY_H
#define PARAM_STUDY_H

#include "DakotaPStudyDACE.H"
#include "data_io.h"


namespace Dakota {

/// Class for vector, list, centered, and multidimensional parameter studies.

/** The ParamStudy class contains several algorithms for performing
    parameter studies of different types.  The vector parameter study
    steps along an n-dimensional vector from an arbitrary initial
    point to an arbitrary final point in a specified number of steps.
    The centered parameter study performs a number of plus and minus
    offsets in each coordinate direction around a center point.  A
    multidimensional parameter study fills an n-dimensional hypercube
    based on bounds and a specified number of partitions for each
    dimension.  And the list parameter study provides for a user
    specification of a list of points to evaluate, which allows
    general parameter investigations not fitting the structure of
    vector, centered, or multidim parameter studies. */

class ParamStudy: public PStudyDACE
{
public:

  //
  //- Heading: Constructors and destructors
  //

  ParamStudy(Model& model); ///< constructor
  ~ParamStudy();            ///< destructor
    
  //
  //- Heading: Virtual member function redefinitions
  //
  
  void pre_run();
  void extract_trends();
  void post_input();
  void post_run(std::ostream& s);

private:

  //
  //- Heading: Convenience/internal member functions
  //

  /// performs the parameter study by sampling from a list of points
  void sample();
  /// performs the parameter study by sampling along a vector, starting from
  /// an initial point followed by numSteps increments along continous/discrete
  /// step vectors
  void vector_loop();
  /// performs a number of plus and minus offsets for each parameter
  /// centered about an initial point
  void centered_loop();
  /// performs a full factorial combination for all intersections
  /// defined by a set of multidimensional partitions
  void multidim_loop();

  /// distributes incoming list_of_pts among listCVPoints, listDIVPoints,
  /// and listDRVPoints
  bool distribute_list_of_points(const RealVector& list_of_pts);
  /// distributes incoming step_vector among contStepVector and discStepVector
  bool distribute_step_vector(const RealVector& step_vector);
  /// compute step vectors from finalPoint, initial points, and numSteps
  void final_point_to_step_vector();
  /// compute step vectors from variablePartitions and global bounds
  void distribute_partitions();

  /// perform error checks on numSteps
  bool check_num_steps(int num_steps);
  /// perform error checks on finalPoint
  bool check_final_point(const RealVector& final_pt);
  /// perform error checks on stepsPerVariable
  bool check_steps_per_variable(const IntVector& steps_per_var);
  /// perform error checks on variablePartitions
  bool check_variable_partitions(const UShortArray& partitions);
  /// check for finite variable bounds within iteratedModel,
  /// as required for computing partitions of finite ranges
  bool check_finite_bounds();
  /// sanity check for vector parameter study
  bool check_ranges_sets(int num_steps);
  /// sanity check for centered parameter study
  bool check_ranges_sets(const IntVector& steps);
  /// sanity check for increments along int/real set dimensions
  bool check_sets(const IntVector& steps);

  /// cast Real to int and ensure no resulting change in value
  int truncate(const Real& value) const;
  /// check for integer remainder and return step
  int integer_step(int range, int num_steps) const;
  /// check for out of bounds and index remainder and return step
  int index_step(size_t start, size_t end, int num_steps) const;

  /// reorder CV/DIV/DRV into standard output order
  void write_ordered(std::ostream& s, const RealVector& c_vector,
		     const IntVector& di_vector, const RealVector& dr_vector);
  /// reorder CV/DV into standard output order
  void write_ordered(std::ostream& s, const RealVector& c_vector,
		     const IntVector& d_vector);

  /// helper function for performing a continuous step in one variable
  void c_step(size_t c_index, int increment, Variables& vars);
  /// helper function for performing a discrete step in an integer
  /// range variable
  void dri_step(size_t d_index, size_t di_index, int increment,
		Variables& vars);
  /// helper function for performing a discrete step in an integer set variable
  void dsi_step(size_t d_index, size_t di_index, int increment,
		const IntSet& values, Variables& vars);
  /// helper function for performing a discrete step in an uncertain
  /// integer set variable
  void dsi_step(size_t d_index, size_t di_index, int increment,
		const IntRealMap& vals_probs, Variables& vars);
  /// helper function for performing a discrete step in a real set variable
  void dsr_step(size_t d_index, size_t dr_index, int increment,
		const RealSet& values, Variables& vars);
  /// helper function for performing a discrete step in an uncertain
  /// real set variable
  void dsr_step(size_t d_index, size_t dr_index, int increment,
		const RealRealMap& vals_probs, Variables& vars);

  //
  //- Heading: Data
  //

  /// internal code for parameter study type: LIST, VECTOR_SV, VECTOR_FP,
  /// CENTERED, or MULTIDIM
  short pStudyType;
  /// total number of parameter study evaluations computed from specification
  size_t numEvals;

  /// array of continuous evaluation points for the list_parameter_study
  RealVectorArray listCVPoints;
  /// array of discrete int evaluation points for the list_parameter_study
  IntVectorArray listDIVPoints;
  /// array of discrete real evaluation points for the list_parameter_study
  RealVectorArray listDRVPoints;

  /// the continuous starting point for vector and centered parameter studies
  RealVector initialCVPoint;
  /// the continuous starting point for vector and centered parameter studies
  IntVector initialDIVPoint;
  /// the continuous starting point for vector and centered parameter studies
  RealVector initialDRVPoint;

  /// the ending point for vector_parameter_study (a specification option)
  RealVector finalPoint;
  /// the n-dimensional continuous increment in vector_parameter_study
  RealVector contStepVector;
  /// the n-dimensional discrete increment in vector_parameter_study
  IntVector discStepVector;
  /// the number of times stepVector is applied in vector_parameter_study
  int numSteps;

  /// number of offsets in the plus and the minus direction for each
  /// variable in a centered_parameter_study
  IntVector stepsPerVariable;

  /// number of partitions for each variable in a multidim_parameter_study
  UShortArray variablePartitions;
};


inline ParamStudy::~ParamStudy() { }


inline bool ParamStudy::check_num_steps(int num_steps)
{
  // basic num_steps checks only, check_vector() performs integrated checks
  bool err = false;
  if (num_steps < 0) {
    Cerr << "\nError: num_steps must be nonnegative in "
	 << "vector_parameter_study." << std::endl;
    err = true;
  }
  numSteps = num_steps;
  numEvals = numSteps + 1;
  return err;
}


inline bool ParamStudy::check_final_point(const RealVector& final_pt)
{
  // basic final_point checks only, check_vector() performs integrated checks
  size_t num_vars
    = numContinuousVars + numDiscreteIntVars + numDiscreteRealVars;
  if (final_pt.length() != num_vars) {
    Cerr << "\nError: final_point must be of dimension " << num_vars
	 << " in vector_parameter_study." << std::endl;
    return true;
  }
  finalPoint = final_pt;
  return false;
}


inline bool ParamStudy::check_steps_per_variable(const IntVector& steps_per_var)
{
  size_t spv_len = steps_per_var.length(),
    num_vars = numContinuousVars + numDiscreteIntVars + numDiscreteRealVars;
  // allow spv_len of 1 or num_vars
  if (spv_len == num_vars)
    stepsPerVariable = steps_per_var;
  else if (spv_len == 1) {
    stepsPerVariable.sizeUninitialized(num_vars);
    stepsPerVariable = steps_per_var[0];
  }
  else {
    Cerr << "\nError: steps_per_variable must be of length 1 or " << num_vars
	 << " in centered_parameter_study." << std::endl;
    return true;
  }
  int spv_sum = 0;
  for (size_t i=0; i<num_vars; ++i)
    spv_sum += std::abs(stepsPerVariable[i]);
  numEvals = 2*spv_sum + 1;
  return false;
}


inline bool ParamStudy::check_variable_partitions(const UShortArray& partitions)
{
  size_t vp_len = partitions.size(),
    num_vars = numContinuousVars + numDiscreteIntVars + numDiscreteRealVars;
  // allow vp_len of 1 or num_vars
  if (vp_len == num_vars)
    variablePartitions = partitions;
  else if (vp_len == 1) {
    variablePartitions.resize(num_vars);
    variablePartitions.assign(num_vars, partitions[0]);
  }
  else {
    Cerr << "\nError: partitions must be of length 1 or " << num_vars 
	 << " in multidim_parameter_study." << std::endl;
    return true;
  }
  numEvals = 1;
  for (size_t i=0; i<num_vars; ++i)
    numEvals *= variablePartitions[i] + 1;
  return false;
}


inline bool ParamStudy::check_finite_bounds()
{
  bool bnds_err = false;
  // Finite bounds required for partitioning: check for case of default bounds
  // (upper/lower = +/- type limits)
  size_t i;
  if (numContinuousVars) {
    const RealVector& c_l_bnds = iteratedModel.continuous_lower_bounds();
    const RealVector& c_u_bnds = iteratedModel.continuous_upper_bounds();
    for (i=0; i<numContinuousVars; ++i)
      if (c_l_bnds[i] <= -DBL_MAX || c_u_bnds[i] >= DBL_MAX)
	{ bnds_err = true; break; }
  }
  if (numDiscreteIntVars) {
    const IntVector& di_l_bnds = iteratedModel.discrete_int_lower_bounds();
    const IntVector& di_u_bnds = iteratedModel.discrete_int_upper_bounds();
    for (i=0; i<numDiscreteIntVars; ++i)
      if (di_l_bnds[i] <= INT_MIN || di_u_bnds[i] >= INT_MAX)
	{ bnds_err = true; break; }
  }
  if (numDiscreteRealVars) {
    const RealVector& dr_l_bnds = iteratedModel.discrete_real_lower_bounds();
    const RealVector& dr_u_bnds = iteratedModel.discrete_real_upper_bounds();
    for (i=0; i<numDiscreteRealVars; ++i)
      if (dr_l_bnds[i] <= -DBL_MAX || dr_u_bnds[i] >= DBL_MAX)
	{ bnds_err = true; break; }
  }
  if (bnds_err)
    Cerr << "\nError: multidim_parameter_study requires specification of "
	 << "variable bounds." << std::endl;
  return bnds_err;
}


inline bool ParamStudy::check_ranges_sets(int num_steps)
{
  // convert scalar to a single vector
  IntVector
    steps(numContinuousVars + numDiscreteIntVars + numDiscreteRealVars, false);
  steps = num_steps;
  return check_sets(steps);
}


inline bool ParamStudy::check_ranges_sets(const IntVector& steps_per_var)
{
  // convert vector to plus and minus vector steps
  size_t i,
    num_vars = numContinuousVars + numDiscreteIntVars + numDiscreteRealVars;
  IntVector steps(steps_per_var);
  bool err = check_sets(steps); // positive offsets
  steps.scale(-1);
  if (check_sets(steps))        // negative offsets
    err = true;
  return err;
}


inline int ParamStudy::truncate(const Real& value) const
{
  int truncated_value = (int)value;
  if (std::abs((Real)truncated_value - value) > DBL_EPSILON) {
    Cerr << "\nError: truncation of Real to discrete integer resulted in "
	 << "change in value.  Verify that input specification for discrete "
	 << "types is integral." << std::endl;
    abort_handler(-1);
  }
  return truncated_value;
}


inline int ParamStudy::integer_step(int range, int num_steps) const
{
  if (range % num_steps) {
    Cerr << "\nError: numSteps results in nonintegral division of integer "
	 << "range defined by start and final points." << std::endl;
    abort_handler(-1);
  }
  return range / num_steps;
}


inline int ParamStudy::index_step(size_t start, size_t end, int num_steps) const
{
  if (start == _NPOS) {
    Cerr << "\nError: specified start value not found in set." << std::endl;
    abort_handler(-1);
  }
  else if (end == _NPOS) {
    Cerr << "\nError: specified final value not found in set." << std::endl;
    abort_handler(-1);
  }
  int range = (int)end - (int)start; // can be negative
  return integer_step(range, num_steps);
}


inline void ParamStudy::
write_ordered(std::ostream& s, const RealVector& c_vector,
	      const IntVector& di_vector, const RealVector& dr_vector)
{
  const SizetArray& vc_totals
    = iteratedModel.current_variables().variables_components_totals();
  size_t i, j, num_cdv = vc_totals[0], num_ddiv = vc_totals[1],
    num_ddrv  = vc_totals[2],  num_cauv  = vc_totals[3],
    num_dauiv = vc_totals[4],  num_daurv = vc_totals[5],
    num_ceuv  = vc_totals[6],  num_deuiv = vc_totals[7],
    num_deurv = vc_totals[8],  num_csv   = vc_totals[9],
    num_dsiv  = vc_totals[10], num_dsrv  = vc_totals[11];
  write_data_partial(s, 0, num_cdv, c_vector);
  write_data_partial(s, 0, num_ddiv, di_vector);
  write_data_partial(s, 0, num_ddrv, dr_vector);
  write_data_partial(s, num_cdv, num_cauv, c_vector);
  write_data_partial(s, num_ddiv, num_dauiv, di_vector);
  write_data_partial(s, num_ddrv, num_daurv, dr_vector);
  write_data_partial(s, num_cdv+num_cauv, num_ceuv, c_vector);
  //write_data_partial(s, num_ddiv+num_dauiv, num_deuiv, di_vector);
  //write_data_partial(s, num_ddrv+num_daurv, num_deurv, dr_vector);
  write_data_partial(s, num_cdv+num_cauv+num_ceuv, num_csv, c_vector);
  write_data_partial(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv, di_vector);
  write_data_partial(s, num_ddrv+num_daurv+num_deurv, num_dsrv, dr_vector);
}


inline void ParamStudy::
write_ordered(std::ostream& s, const RealVector& c_vector,
	      const IntVector& d_vector)
{
  const SizetArray& vc_totals
    = iteratedModel.current_variables().variables_components_totals();
  size_t i, j, num_cdv = vc_totals[0],
    num_ddv  = vc_totals[1] + vc_totals[2], num_cauv = vc_totals[3],
    num_dauv = vc_totals[4] + vc_totals[5], num_ceuv = vc_totals[6],
    num_deuv = vc_totals[7] + vc_totals[8], num_csv  = vc_totals[9],
    num_dsv  = vc_totals[10] + vc_totals[11];
  write_data_partial(s, 0, num_cdv, c_vector);
  write_data_partial(s, 0, num_ddv, d_vector);
  write_data_partial(s, num_cdv, num_cauv, c_vector);
  write_data_partial(s, num_ddv, num_dauv, d_vector);
  write_data_partial(s, num_cdv+num_cauv, num_ceuv, c_vector);
  //write_data_partial(s, num_ddv+num_dauv, num_deuv, d_vector);
  write_data_partial(s, num_cdv+num_cauv+num_ceuv, num_csv, c_vector);
  write_data_partial(s, num_ddv+num_dauv+num_deuv, num_dsv, d_vector);
}


inline void ParamStudy::c_step(size_t c_index, int increment, Variables& vars)
{
  // bounds currently ignored for range types
  Real c_var = initialCVPoint[c_index] + increment * contStepVector[c_index];
  vars.continuous_variable(c_var, c_index);
}


inline void ParamStudy::
dri_step(size_t d_index, size_t di_index, int increment, Variables& vars)
{
  // bounds currently ignored for range types
  int di_var = initialDIVPoint[di_index] + increment * discStepVector[d_index];
  vars.discrete_int_variable(di_var, di_index);
}


inline void ParamStudy::
dsi_step(size_t d_index, size_t di_index, int increment, const IntSet& values,
	 Variables& vars)
{
  // valid values and indices are checked for set types
  size_t index0 = set_value_to_index(initialDIVPoint[di_index], values);
  if (index0 == _NPOS) {
    Cerr << "\nError: value " << initialDIVPoint[di_index] << " does not exist "
	 << "within discrete integer set in ParamStudy::dsi_step()."<<std::endl;
    abort_handler(-1);
  }
  int index = index0 + increment * discStepVector[d_index]; // +/- increments
  if (index >= 0 && index < values.size())
    vars.discrete_int_variable(set_index_to_value(index, values), di_index);
  else {
    Cerr << "\nError: index " << index << " out of range within discrete "
	 << "integer set in ParamStudy::dsi_step()." << std::endl;
    abort_handler(-1);
  }
}


inline void ParamStudy::
dsi_step(size_t d_index, size_t di_index, int increment,
	 const IntRealMap& vals_probs, Variables& vars)
{
  // valid values and indices are checked for set types
  size_t index0 = map_key_to_index(initialDIVPoint[di_index], vals_probs);
  if (index0 == _NPOS) {
    Cerr << "\nError: value " << initialDIVPoint[di_index] << " does not exist "
	 << "within discrete integer set in ParamStudy::dsi_step()."<<std::endl;
    abort_handler(-1);
  }
  int index = index0 + increment * discStepVector[d_index]; // +/- increments
  if (index >= 0 && index < vals_probs.size())
    vars.discrete_int_variable(map_index_to_key(index, vals_probs), di_index);
  else {
    Cerr << "\nError: index " << index << " out of range within discrete "
	 << "integer set in ParamStudy::dsi_step()." << std::endl;
    abort_handler(-1);
  }
}


inline void ParamStudy::
dsr_step(size_t d_index, size_t dr_index, int increment,
	 const RealSet& values, Variables& vars)
{
  // valid values and indices are checked for set types
  size_t index0 = set_value_to_index(initialDRVPoint[dr_index], values);
  if (index0 == _NPOS) {
    Cerr << "\nError: value " << initialDRVPoint[dr_index] << " does not exist "
	 << "within discrete real set in ParamStudy::dsr_step()." << std::endl;
    abort_handler(-1);
  }
  int index = index0 + increment * discStepVector[d_index]; // +/- increments
  if (index >= 0 && index < values.size())
    vars.discrete_real_variable(set_index_to_value(index, values), dr_index);
  else {
    Cerr << "\nError: index " << index << " out of range within discrete "
	 << "real set in ParamStudy::dsr_step()." << std::endl;
    abort_handler(-1);
  }
}


inline void ParamStudy::
dsr_step(size_t d_index, size_t dr_index, int increment,
	 const RealRealMap& vals_probs, Variables& vars)
{
  // valid values and indices are checked for set types
  size_t index0 = map_key_to_index(initialDRVPoint[dr_index], vals_probs);
  if (index0 == _NPOS) {
    Cerr << "\nError: value " << initialDRVPoint[dr_index] << " does not exist "
	 << "within discrete real set in ParamStudy::dsr_step()." << std::endl;
    abort_handler(-1);
  }
  int index = index0 + increment * discStepVector[d_index]; // +/- increments
  if (index >= 0 && index < vals_probs.size())
    vars.discrete_real_variable(map_index_to_key(index, vals_probs), dr_index);
  else {
    Cerr << "\nError: index " << index << " out of range within discrete "
	 << "real set in ParamStudy::dsr_step()." << std::endl;
    abort_handler(-1);
  }
}

} // namespace Dakota

#endif
