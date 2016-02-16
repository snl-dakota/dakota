/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ParamStudy
//- Description: Parameter study driver program.  This class iterates a
//-              Model object using simple rules, i.e. evaluating
//-              a variety of specified points in the design space.   
//- Owner:       Mike Eldred
//- Version: $Id: ParamStudy.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef PARAM_STUDY_H
#define PARAM_STUDY_H

#include "DakotaPStudyDACE.hpp"
#include "dakota_data_io.hpp"


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

  ParamStudy(ProblemDescDB& problem_db, Model& model); ///< constructor
  ~ParamStudy();                                       ///< destructor
    
  //
  //- Heading: Virtual member function redefinitions
  //
  
  bool resize();
  void pre_run();
  void core_run();
  void post_input();
  void post_run(std::ostream& s);

protected:

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

  /// load list of points from data file and distribute among
  /// listCVPoints, listDIVPoints, listDSVPoints, and listDRVPoints
  bool load_distribute_points(const String& points_filename, 
			      unsigned short tabular_format,
			      bool active_only);

  /// distributes incoming all vector in standard variable ordering among
  /// continuous, discrete int, discrete string, and discrete real vectors
  template <typename OrdinalType,  typename ScalarTypeA, typename ScalarTypeC,
	    typename ScalarTypeDI, typename ScalarTypeDS,typename ScalarTypeDR> 
  bool distribute(
    const Teuchos::SerialDenseVector<OrdinalType, ScalarTypeA>& all_data,
    Teuchos::SerialDenseVector<OrdinalType, ScalarTypeC>& c_data,
    Teuchos::SerialDenseVector<OrdinalType, ScalarTypeDI>& di_data,
    Teuchos::SerialDenseVector<OrdinalType, ScalarTypeDS>& ds_data,
    Teuchos::SerialDenseVector<OrdinalType, ScalarTypeDR>& dr_data);

  /// distributes incoming all array in standard variable ordering among
  /// continuous, discrete int, discrete string, and discrete real arrays
  template <typename ScalarType> 
  bool distribute(const std::vector<ScalarType>& all_data,
		  std::vector<ScalarType>& c_data,
		  std::vector<ScalarType>& di_data,
		  std::vector<ScalarType>& ds_data,
		  std::vector<ScalarType>& dr_data);

  /// distributes list_of_pts coming from user spec among
  /// listCVPoints, listDIVPoints, listDSVPoints, and listDRVPoints
  bool distribute_list_of_points(const RealVector& list_of_pts);
  /// compute step vectors from finalPoint, initial points, and numSteps
  void final_point_to_step_vector();
  /// compute step vectors from {cont,discInt,discString,discReal}VarPartitions
  /// and global bounds
  void distribute_partitions();

  /// perform error checks on numSteps
  bool check_num_steps(int num_steps);
  /// perform error checks on numSteps
  bool check_step_vector(const RealVector& step_vector);
  /// perform error checks on finalPoint
  bool check_final_point(const RealVector& final_pt);
  /// perform error checks on stepsPerVariable
  bool check_steps_per_variable(const IntVector& steps_per_var);
  /// perform error checks on variable partitions
  bool check_variable_partitions(const UShortArray& partitions);
  /// check for finite variable bounds within iteratedModel,
  /// as required for computing partitions of finite ranges
  bool check_finite_bounds();
  /// sanity check for vector parameter study
  bool check_ranges_sets(int num_steps);
  /// sanity check for centered parameter study
  bool check_ranges_sets(const IntVector& c_steps,  const IntVector& di_steps,
			 const IntVector& ds_steps, const IntVector& dr_steps);
  /// sanity check for increments along int/real set dimensions
  bool check_sets(const IntVector& c_steps,  const IntVector& di_steps,
		  const IntVector& ds_steps, const IntVector& dr_steps);

  /// check for integer remainder and return step
  int integer_step(int range, int num_steps) const;
  /// check for out of bounds and index remainder and return step
  int index_step(size_t start, size_t end, int num_steps) const;

  /// helper function for performing a continuous step in one variable
  void c_step(size_t c_index, int increment, Variables& vars);
  /// helper function for performing a discrete step in an integer
  /// range variable
  void dri_step(size_t di_index, int increment,	Variables& vars);
  /// helper function for performing a discrete step in an integer set variable
  void dsi_step(size_t di_index, int increment, const IntSet& values,
		Variables& vars);
  /// helper function for performing a discrete step in an string set variable
  void dss_step(size_t ds_index, int increment, const StringSet& values,
		Variables& vars);
  /// helper function for performing a discrete step in a real set variable
  void dsr_step(size_t dr_index, int increment, const RealSet& values,
		Variables& vars);

  /// reset vars to initial point (center)
  void reset(Variables& vars);
  /// store a centered parameter study header within allHeaders
  void centered_header(const String& type, size_t var_index, int step,
		       size_t hdr_index);

  //
  //- Heading: Data
  //

  /// total number of parameter study evaluations computed from specification
  size_t numEvals;

  /// array of continuous evaluation points for the list_parameter_study
  RealVectorArray listCVPoints;
  /// array of discrete int evaluation points for the list_parameter_study
  IntVectorArray listDIVPoints;
  /// array of discrete string evaluation points for the list_parameter_study
  StringMulti2DArray listDSVPoints;
  /// array of discrete real evaluation points for the list_parameter_study
  RealVectorArray listDRVPoints;

  /// the continuous start point for vector and centered parameter studies
  RealVector initialCVPoint;
  /// the discrete int start point for vector and centered parameter studies
  IntVector initialDIVPoint;
  /// the discrete string start point for vector and centered parameter studies
  StringMultiArray initialDSVPoint;
  /// the discrete real start point for vector and centered parameter studies
  RealVector initialDRVPoint;

  /// the continuous ending point for vector_parameter_study
  RealVector finalCVPoint;
  /// the discrete int range value or set index ending point for
  /// vector_parameter_study
  IntVector finalDIVPoint;
  /// the discrete string set index ending point for vector_parameter_study
  IntVector finalDSVPoint;
  /// the discrete real set index ending point for vector_parameter_study
  IntVector finalDRVPoint;

  /// the n-dimensional continuous increment
  RealVector contStepVector;
  /// the n-dimensional discrete integer range value or set index increment
  IntVector discIntStepVector;
  /// the n-dimensional discrete string set index increment
  IntVector discStringStepVector;
  /// the n-dimensional discrete real set index increment
  IntVector discRealStepVector;

  /// the number of times continuous/discrete step vectors are applied
  /// for vector_parameter_study (a specification option)
  int numSteps;

  /// number of offsets in the plus and the minus direction for each
  /// continuous variable in a centered_parameter_study
  IntVector contStepsPerVariable;
  /// number of offsets in the plus and the minus direction for each
  /// discrete integer variable in a centered_parameter_study
  IntVector discIntStepsPerVariable;
  /// number of offsets in the plus and the minus direction for each
  /// discrete string variable in a centered_parameter_study
  IntVector discStringStepsPerVariable;
  /// number of offsets in the plus and the minus direction for each
  /// discrete real variable in a centered_parameter_study
  IntVector discRealStepsPerVariable;

  /// number of partitions for each continuous variable in a
  /// multidim_parameter_study
  UShortArray contVarPartitions;
  /// number of partitions for each discrete integer variable in a
  /// multidim_parameter_study
  UShortArray discIntVarPartitions;
  /// number of partitions for each discrete string variable in a
  /// multidim_parameter_study
  UShortArray discStringVarPartitions;
  /// number of partitions for each discrete real variable in a
  /// multidim_parameter_study
  UShortArray discRealVarPartitions;
};


inline ParamStudy::~ParamStudy() { }


template <typename OrdinalType,  typename ScalarTypeA,  typename ScalarTypeC,
	  typename ScalarTypeDI, typename ScalarTypeDS, typename ScalarTypeDR> 
bool ParamStudy::
distribute(const Teuchos::SerialDenseVector<OrdinalType, ScalarTypeA>& all_data,
	   Teuchos::SerialDenseVector<OrdinalType, ScalarTypeC>&   c_data,
	   Teuchos::SerialDenseVector<OrdinalType, ScalarTypeDI>& di_data,
	   Teuchos::SerialDenseVector<OrdinalType, ScalarTypeDS>& ds_data,
	   Teuchos::SerialDenseVector<OrdinalType, ScalarTypeDR>& dr_data)
{
  size_t num_vars = numContinuousVars     + numDiscreteIntVars
                  + numDiscreteStringVars + numDiscreteRealVars;
  if (all_data.length() != num_vars) {
    Cerr << "\nError: ParamStudy::distribute() input length must be "
	 << num_vars << '.' << std::endl;
    return true;
  }
  c_data.sizeUninitialized(numContinuousVars);
  di_data.sizeUninitialized(numDiscreteIntVars);
  ds_data.sizeUninitialized(numDiscreteStringVars);
  dr_data.sizeUninitialized(numDiscreteRealVars);

  // Extract in order:
  //   cdv/ddiv/ddrv, cauv/dauiv/daurv, ceuv/deuiv/deurv, csv/dsiv/dsrv
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  const SizetArray& active_totals = svd.active_components_totals();
  size_t i,
    num_cdv   = active_totals[TOTAL_CDV],  num_ddiv = active_totals[TOTAL_DDIV],
    num_ddsv  = active_totals[TOTAL_DDSV], num_ddrv = active_totals[TOTAL_DDRV],
    num_cauv  = active_totals[TOTAL_CAUV],
    num_dauiv = active_totals[TOTAL_DAUIV],
    num_dausv = active_totals[TOTAL_DAUSV],
    num_daurv = active_totals[TOTAL_DAURV],
    num_ceuv  = active_totals[TOTAL_CEUV],
    num_deuiv = active_totals[TOTAL_DEUIV],
    num_deusv = active_totals[TOTAL_DEUSV],
    num_deurv = active_totals[TOTAL_DEURV],
    num_csv   = active_totals[TOTAL_CSV],  num_dsiv = active_totals[TOTAL_DSIV],
    num_dssv  = active_totals[TOTAL_DSSV], num_dsrv = active_totals[TOTAL_DSRV],
    s_cntr = 0, c_cntr = 0, di_cntr = 0, ds_cntr = 0, dr_cntr = 0;
  for (i=0; i<num_cdv; ++i, ++s_cntr, ++c_cntr)
    c_data[c_cntr]   = static_cast<ScalarTypeC>(all_data[s_cntr]);
  for (i=0; i<num_ddiv; ++i, ++s_cntr, ++di_cntr)
    di_data[di_cntr] = static_cast<ScalarTypeDI>(all_data[s_cntr]);
  for (i=0; i<num_ddsv; ++i, ++s_cntr, ++ds_cntr)
    ds_data[ds_cntr] = static_cast<ScalarTypeDS>(all_data[s_cntr]);
  for (i=0; i<num_ddrv; ++i, ++s_cntr, ++dr_cntr)
    dr_data[dr_cntr] = static_cast<ScalarTypeDR>(all_data[s_cntr]);
  for (i=0; i<num_cauv; ++i, ++s_cntr, ++c_cntr)
    c_data[c_cntr]   = static_cast<ScalarTypeC>(all_data[s_cntr]);
  for (i=0; i<num_dauiv; ++i, ++s_cntr, ++di_cntr)
    di_data[di_cntr] = static_cast<ScalarTypeDI>(all_data[s_cntr]);
  for (i=0; i<num_dausv; ++i, ++s_cntr, ++ds_cntr)
    ds_data[ds_cntr] = static_cast<ScalarTypeDS>(all_data[s_cntr]);
  for (i=0; i<num_daurv; ++i, ++s_cntr, ++dr_cntr)
    dr_data[dr_cntr] = static_cast<ScalarTypeDR>(all_data[s_cntr]);
  for (i=0; i<num_ceuv; ++i, ++s_cntr, ++c_cntr)
    c_data[c_cntr]   = static_cast<ScalarTypeC>(all_data[s_cntr]);
  for (i=0; i<num_deuiv; ++i, ++s_cntr, ++di_cntr)
    di_data[di_cntr] = static_cast<ScalarTypeDI>(all_data[s_cntr]);
  for (i=0; i<num_deusv; ++i, ++s_cntr, ++ds_cntr)
    ds_data[ds_cntr] = static_cast<ScalarTypeDS>(all_data[s_cntr]);
  for (i=0; i<num_deurv; ++i, ++s_cntr, ++dr_cntr)
    dr_data[dr_cntr] = static_cast<ScalarTypeDR>(all_data[s_cntr]);
  for (i=0; i<num_csv; ++i, ++s_cntr, ++c_cntr)
    c_data[c_cntr]   = static_cast<ScalarTypeC>(all_data[s_cntr]);
  for (i=0; i<num_dsiv; ++i, ++s_cntr, ++di_cntr)
    di_data[di_cntr] = static_cast<ScalarTypeDI>(all_data[s_cntr]);
  for (i=0; i<num_dssv; ++i, ++s_cntr, ++ds_cntr)
    ds_data[ds_cntr] = static_cast<ScalarTypeDS>(all_data[s_cntr]);
  for (i=0; i<num_dsrv; ++i, ++s_cntr, ++dr_cntr)
    dr_data[dr_cntr] = static_cast<ScalarTypeDR>(all_data[s_cntr]);

#ifdef DEBUG
  Cout << "distribute():\n";
  if (numContinuousVars) {
    Cout << "continuous vector:\n";
    write_data(Cout, c_data);
  }
  if (numDiscreteIntVars) {
    Cout << "discrete int vector:\n";
    write_data(Cout, di_data);
  }
  if (numDiscreteStringVars) {
    Cout << "discrete string vector:\n";
    write_data(Cout, ds_data);
  }
  if (numDiscreteRealVars) {
    Cout << "discrete real vector:\n";
    write_data(Cout, dr_data);
  }
#endif // DEBUG

  return false;
}


template <typename ScalarType> bool ParamStudy::
distribute(const std::vector<ScalarType>& all_data,
	   std::vector<ScalarType>& c_data,
	   std::vector<ScalarType>& di_data,
	   std::vector<ScalarType>& ds_data,
	   std::vector<ScalarType>& dr_data)
{
  size_t num_vars = numContinuousVars + numDiscreteIntVars
                  + numDiscreteStringVars + numDiscreteRealVars;
  if (all_data.size() != num_vars) {
    Cerr << "\nError: ParamStudy::distribute() input length must be "
	 << num_vars << '.' << std::endl;
    return true;
  }
  c_data.resize(numContinuousVars);
  di_data.resize(numDiscreteIntVars);
  ds_data.resize(numDiscreteStringVars);
  dr_data.resize(numDiscreteRealVars);

  // Extract in order:
  //   cdv/ddiv/ddrv, cauv/dauiv/daurv, ceuv/deuiv/deurv, csv/dsiv/dsrv
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  const SizetArray& active_totals = svd.active_components_totals();
  size_t i,
    num_cdv   = active_totals[TOTAL_CDV],  num_ddiv = active_totals[TOTAL_DDIV],
    num_ddsv  = active_totals[TOTAL_DDSV], num_ddrv = active_totals[TOTAL_DDRV],
    num_cauv  = active_totals[TOTAL_CAUV],
    num_dauiv = active_totals[TOTAL_DAUIV],
    num_dausv = active_totals[TOTAL_DAUSV],
    num_daurv = active_totals[TOTAL_DAURV],
    num_ceuv  = active_totals[TOTAL_CEUV],
    num_deuiv = active_totals[TOTAL_DEUIV],
    num_deusv = active_totals[TOTAL_DEUSV],
    num_deurv = active_totals[TOTAL_DEURV],
    num_csv   = active_totals[TOTAL_CSV],  num_dsiv = active_totals[TOTAL_DSIV],
    num_dssv  = active_totals[TOTAL_DSSV], num_dsrv = active_totals[TOTAL_DSRV],
    s_cntr = 0, c_cntr = 0, di_cntr = 0, ds_cntr = 0, dr_cntr = 0;
  for (i=0; i<num_cdv; ++i, ++s_cntr, ++c_cntr)
    c_data[c_cntr]   = all_data[s_cntr];
  for (i=0; i<num_ddiv; ++i, ++s_cntr, ++di_cntr)
    di_data[di_cntr] = all_data[s_cntr];
  for (i=0; i<num_ddsv; ++i, ++s_cntr, ++ds_cntr)
    ds_data[ds_cntr] = all_data[s_cntr];
  for (i=0; i<num_ddrv; ++i, ++s_cntr, ++dr_cntr)
    dr_data[dr_cntr] = all_data[s_cntr];
  for (i=0; i<num_cauv; ++i, ++s_cntr, ++c_cntr)
    c_data[c_cntr]   = all_data[s_cntr];
  for (i=0; i<num_dauiv; ++i, ++s_cntr, ++di_cntr)
    di_data[di_cntr] = all_data[s_cntr];
  for (i=0; i<num_dausv; ++i, ++s_cntr, ++ds_cntr)
    ds_data[ds_cntr] = all_data[s_cntr];
  for (i=0; i<num_daurv; ++i, ++s_cntr, ++dr_cntr)
    dr_data[dr_cntr] = all_data[s_cntr];
  for (i=0; i<num_ceuv; ++i, ++s_cntr, ++c_cntr)
    c_data[c_cntr]   = all_data[s_cntr];
  for (i=0; i<num_deuiv; ++i, ++s_cntr, ++di_cntr)
    di_data[di_cntr] = all_data[s_cntr];
  for (i=0; i<num_deusv; ++i, ++s_cntr, ++ds_cntr)
    ds_data[ds_cntr] = all_data[s_cntr];
  for (i=0; i<num_deurv; ++i, ++s_cntr, ++dr_cntr)
    dr_data[dr_cntr] = all_data[s_cntr];
  for (i=0; i<num_csv; ++i, ++s_cntr, ++c_cntr)
    c_data[c_cntr]   = all_data[s_cntr];
  for (i=0; i<num_dsiv; ++i, ++s_cntr, ++di_cntr)
    di_data[di_cntr] = all_data[s_cntr];
  for (i=0; i<num_dssv; ++i, ++s_cntr, ++ds_cntr)
    ds_data[ds_cntr] = all_data[s_cntr];
  for (i=0; i<num_dsrv; ++i, ++s_cntr, ++dr_cntr)
    dr_data[dr_cntr] = all_data[s_cntr];

#ifdef DEBUG
  Cout << "distribute():\n";
  if (numContinuousVars) {
    Cout << "continuous array:\n";
    write_data(Cout, c_data);
  }
  if (numDiscreteIntVars) {
    Cout << "discrete int array:\n";
    write_data(Cout, di_data);
  }
  if (numDiscreteStringVars) {
    Cout << "discrete string array:\n";
    write_data(Cout, ds_data);
  }
  if (numDiscreteRealVars) {
    Cout << "discrete real array:\n";
    write_data(Cout, dr_data);
  }
#endif // DEBUG

  return false;
}


inline bool ParamStudy::check_num_steps(int num_steps)
{
  // basic num_steps checks only; additional checks occur downstream
  if (num_steps < 0) {
    Cerr << "\nError: num_steps must be nonnegative in "
	 << "vector_parameter_study." << std::endl;
    return true;
  }
  numSteps = num_steps;
  numEvals = numSteps + 1;
  return false;
}


inline bool ParamStudy::check_step_vector(const RealVector& step_vec)
{
  // basic final_point checks only, additional checks occur downstream
  size_t num_vars = numContinuousVars     + numDiscreteIntVars
                  + numDiscreteStringVars + numDiscreteRealVars;
  if (step_vec.length() != num_vars) {
    Cerr << "\nError: step_vector must be of dimension " << num_vars
	 << " in vector_parameter_study." << std::endl;
    return true;
  }
  return distribute(step_vec, contStepVector, discIntStepVector,
		    discStringStepVector, discRealStepVector);
}


inline bool ParamStudy::check_final_point(const RealVector& final_pt)
{
  // basic final_point checks only, additional checks occur downstream
  size_t num_vars = numContinuousVars     + numDiscreteIntVars
                  + numDiscreteStringVars + numDiscreteRealVars;
  if (final_pt.length() != num_vars) {
    Cerr << "\nError: final_point must be of dimension " << num_vars
	 << " in vector_parameter_study." << std::endl;
    return true;
  }
  return distribute(final_pt, finalCVPoint, finalDIVPoint, finalDSVPoint,
		    finalDRVPoint);
}


inline bool ParamStudy::check_steps_per_variable(const IntVector& steps_per_var)
{
  size_t spv_len = steps_per_var.length(),
    num_vars = numContinuousVars     + numDiscreteIntVars +
               numDiscreteStringVars + numDiscreteRealVars;
  // allow spv_len of 1 or num_vars
  if (spv_len == num_vars)
    distribute(steps_per_var, contStepsPerVariable, discIntStepsPerVariable,
	       discStringStepsPerVariable, discRealStepsPerVariable);
  else if (spv_len == 1) {
    int steps = steps_per_var[0];
    contStepsPerVariable.sizeUninitialized(numContinuousVars);
    contStepsPerVariable = steps;
    discIntStepsPerVariable.sizeUninitialized(numDiscreteIntVars);
    discIntStepsPerVariable = steps;
    discStringStepsPerVariable.sizeUninitialized(numDiscreteStringVars);
    discStringStepsPerVariable = steps;
    discRealStepsPerVariable.sizeUninitialized(numDiscreteRealVars);
    discRealStepsPerVariable = steps;
  }
  else {
    Cerr << "\nError: steps_per_variable must be of length 1 or " << num_vars
	 << " in centered_parameter_study." << std::endl;
    return true;
  }
  size_t i, spv_sum = 0;
  for (i=0; i<numContinuousVars; ++i)
    spv_sum += std::abs(contStepsPerVariable[i]);
  for (i=0; i<numDiscreteIntVars; ++i)
    spv_sum += std::abs(discIntStepsPerVariable[i]);
  for (i=0; i<numDiscreteStringVars; ++i)
    spv_sum += std::abs(discStringStepsPerVariable[i]);
  for (i=0; i<numDiscreteRealVars; ++i)
    spv_sum += std::abs(discRealStepsPerVariable[i]);
  numEvals = 2*spv_sum + 1;
  return false;
}


inline bool ParamStudy::check_variable_partitions(const UShortArray& partitions)
{
  size_t i, vp_len = partitions.size();
  // allow vp_len of 1 or num_vars
  if (vp_len == numContinuousVars     + numDiscreteIntVars +
                numDiscreteStringVars + numDiscreteRealVars)
    distribute(partitions, contVarPartitions, discIntVarPartitions,
	       discStringVarPartitions, discRealVarPartitions);
  else if (vp_len == 1) {
    unsigned short part = partitions[0];
    contVarPartitions.assign(numContinuousVars, part);
    discIntVarPartitions.assign(numDiscreteIntVars, part);
    discStringVarPartitions.assign(numDiscreteStringVars, part);
    discRealVarPartitions.assign(numDiscreteRealVars, part);
  }
  else {
    Cerr << "\nError: partitions must be of length 1 or "
	 << numContinuousVars + numDiscreteIntVars + numDiscreteStringVars +
            numDiscreteRealVars << " in multidim_parameter_study." << std::endl;
    return true;
  }
  numEvals = 1;
  for (i=0; i<numContinuousVars; ++i)
    numEvals *= contVarPartitions[i] + 1;
  for (i=0; i<numDiscreteIntVars; ++i)
    numEvals *= discIntVarPartitions[i] + 1;
  for (i=0; i<numDiscreteStringVars; ++i)
    numEvals *= discStringVarPartitions[i] + 1;
  for (i=0; i<numDiscreteRealVars; ++i)
    numEvals *= discRealVarPartitions[i] + 1;
  return false;
}


inline bool ParamStudy::check_finite_bounds()
{
  bool bnds_err = false;
  // Finite bounds required for partitioning: check for case of default bounds
  // (upper/lower = +/- type limits)
  size_t i;
  Real dbl_inf = std::numeric_limits<Real>::infinity();
  if (numContinuousVars) {
    const RealVector& c_l_bnds = iteratedModel.continuous_lower_bounds();
    const RealVector& c_u_bnds = iteratedModel.continuous_upper_bounds();
    for (i=0; i<numContinuousVars; ++i)
      if (c_l_bnds[i] == -dbl_inf || c_u_bnds[i] == dbl_inf)
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
      if (dr_l_bnds[i] == -dbl_inf || dr_u_bnds[i] == dbl_inf)
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
  IntVector c_steps_per_var(numContinuousVars,     false),
           di_steps_per_var(numDiscreteIntVars,    false),
           ds_steps_per_var(numDiscreteStringVars, false),
           dr_steps_per_var(numDiscreteRealVars,   false);
  c_steps_per_var  = num_steps;
  di_steps_per_var = num_steps;
  ds_steps_per_var = num_steps;
  dr_steps_per_var = num_steps;
  return check_sets(c_steps_per_var, di_steps_per_var, ds_steps_per_var,
		    dr_steps_per_var);
}


inline bool ParamStudy::
check_ranges_sets(const IntVector& c_steps_per_var,
		  const IntVector& di_steps_per_var,
		  const IntVector& ds_steps_per_var,
		  const IntVector& dr_steps_per_var)
{
  // convert vector to plus and minus vector steps
  IntVector c_steps(c_steps_per_var),  di_steps(di_steps_per_var),
           ds_steps(ds_steps_per_var), dr_steps(dr_steps_per_var);
  bool err = check_sets(c_steps, di_steps, ds_steps, dr_steps); // + offsets
  c_steps.scale(-1); di_steps.scale(-1); dr_steps.scale(-1);
  if (check_sets(c_steps, di_steps, ds_steps, dr_steps))        // - offsets
    err = true;
  return err;
}


inline int ParamStudy::integer_step(int range, int num_steps) const
{
  if (range % num_steps) {
    Cerr << "\nError: numSteps results in nonintegral division of integer/"
	 << "index range defined by start and final points." << std::endl;
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


inline void ParamStudy::c_step(size_t c_index, int increment, Variables& vars)
{
  // bounds currently ignored for range types
  Real c_var = initialCVPoint[c_index] + increment * contStepVector[c_index];
  vars.continuous_variable(c_var, c_index);
}


inline void ParamStudy::
dri_step(size_t di_index, int increment, Variables& vars)
{
  // bounds currently ignored for range types
  int di_var = initialDIVPoint[di_index]
             + increment * discIntStepVector[di_index];
  vars.discrete_int_variable(di_var, di_index);
}


inline void ParamStudy::
dsi_step(size_t di_index, int increment, const IntSet& values, Variables& vars)
{
  // valid values and indices are checked for set types
  size_t index0 = set_value_to_index(initialDIVPoint[di_index], values);
  if (index0 == _NPOS) {
    Cerr << "\nError: value " << initialDIVPoint[di_index] << " does not exist "
	 << "within discrete integer set in ParamStudy::dsi_step()."<<std::endl;
    abort_handler(-1);
  }
  int index = index0 + increment * discIntStepVector[di_index];// +/- increments
  if (index >= 0 && index < values.size())
    vars.discrete_int_variable(set_index_to_value(index, values), di_index);
  else {
    Cerr << "\nError: index " << index << " out of range within discrete "
	 << "integer set in ParamStudy::dsi_step()." << std::endl;
    abort_handler(-1);
  }
}


inline void ParamStudy::
dss_step(size_t ds_index, int increment, const StringSet& values,
	 Variables& vars)
{
  // valid values and indices are checked for set types
  size_t index0 = set_value_to_index(initialDSVPoint[ds_index], values);
  if (index0 == _NPOS) {
    Cerr << "\nError: value " << initialDSVPoint[ds_index] << " does not exist "
	 << "within discrete string set in ParamStudy::dss_step()."<< std::endl;
    abort_handler(-1);
  }
  int index = index0 + increment * discStringStepVector[ds_index]; // +/- steps
  if (index >= 0 && index < values.size())
    vars.discrete_string_variable(set_index_to_value(index, values), ds_index);
  else {
    Cerr << "\nError: index " << index << " out of range within discrete "
	 << "string set in ParamStudy::dsr_step()." << std::endl;
    abort_handler(-1);
  }
}


inline void ParamStudy::
dsr_step(size_t dr_index, int increment, const RealSet& values, Variables& vars)
{
  // valid values and indices are checked for set types
  size_t index0 = set_value_to_index(initialDRVPoint[dr_index], values);
  if (index0 == _NPOS) {
    Cerr << "\nError: value " << initialDRVPoint[dr_index] << " does not exist "
	 << "within discrete real set in ParamStudy::dsr_step()." << std::endl;
    abort_handler(-1);
  }
  int index = index0 + increment * discRealStepVector[dr_index];//+/- increments
  if (index >= 0 && index < values.size())
    vars.discrete_real_variable(set_index_to_value(index, values), dr_index);
  else {
    Cerr << "\nError: index " << index << " out of range within discrete "
	 << "real set in ParamStudy::dsr_step()." << std::endl;
    abort_handler(-1);
  }
}


inline void ParamStudy::reset(Variables& vars)
{
  if (numContinuousVars)     vars.continuous_variables(initialCVPoint);
  if (numDiscreteIntVars)    vars.discrete_int_variables(initialDIVPoint);
  if (numDiscreteStringVars) vars.discrete_string_variables(
    initialDSVPoint[boost::indices[idx_range(0, numDiscreteStringVars)]]);
  if (numDiscreteRealVars)   vars.discrete_real_variables(initialDRVPoint);
}


inline void ParamStudy::
centered_header(const String& type, size_t var_index, int step,
		size_t hdr_index)
{
  String& h_string = allHeaders[hdr_index];
  h_string.clear();
  if (iteratedModel.asynch_flag())
    h_string += "\n\n";
  // This code expanded due to MSVC issue with Dakota::String operator +/+=
  // Can be combined once using std::string everywhere
  h_string += ">>>>> Centered parameter study evaluation for ";
  h_string += type; 
  h_string += "[";
  h_string += boost::lexical_cast<std::string>(var_index+1); 
  h_string += "]";
  if (step < 0) h_string += " - " + boost::lexical_cast<std::string>(-step);
  else          h_string += " + " + boost::lexical_cast<std::string>( step);
  h_string += "delta:\n";
}

} // namespace Dakota

#endif
