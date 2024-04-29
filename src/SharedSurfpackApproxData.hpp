/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#ifndef SHARED_SURFPACK_APPROX_DATA_H 
#define SHARED_SURFPACK_APPROX_DATA_H 

#include "SharedApproxData.hpp"
#include "DakotaVariables.hpp"
// Pecos headers
#include "SurrogateData.hpp"

class SurfData;
template< typename T > class SurfpackMatrix;

namespace Dakota {


/// Derived approximation class for Surfpack approximation classes.
/// Interface between Surfpack and Dakota.

/** The SharedSurfpackApproxData class is the interface between Dakota
    and Surfpack.  Based on the information in the ProblemDescDB that
    is passed in through the constructor, SharedSurfpackApproxData builds
    a Surfpack Surface object that corresponds to one of the following
    data-fitting techniques: polynomial regression, kriging,
    artificial neural networks, radial basis function network, or
    multivariate adaptaive regression splines (MARS). */

class SharedSurfpackApproxData: public SharedApproxData
{
  //
  //- Heading: Friends
  //

  friend class SurfpackApproximation;
    
  friend class VPSApproximation;  // Mohamed Ebeida

  friend class SurrogatesBaseApprox;
  friend class SurrogatesGPApprox;
  friend class SurrogatesPolyApprox;
  friend class SurrogatesPythonApprox;

public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SharedSurfpackApproxData();
  /// alternate constructor
  SharedSurfpackApproxData(const String& approx_type,
			   const UShortArray& approx_order, size_t num_vars,
			   short data_order, short output_level);
  /// standard constructor: Surfpack surface of appropriate type will be created
  SharedSurfpackApproxData(ProblemDescDB& problem_db, size_t num_vars);
  /// destructor
  ~SharedSurfpackApproxData();

protected:

private:

  //
  //- Heading: Convenience functions
  //

  /// add Pecos::SurrogateData::SurrogateData{Vars,Resp} to SurfData,
  /// accounting for buildDataOrder available
  void add_sd_to_surfdata(const Pecos::SurrogateDataVars& sdv,
			  const Pecos::SurrogateDataResp& sdr, short fail_code,
			  SurfData& surf_data);

  /// copy RealSymMatrix to SurfpackMatrix (Real type only)
  void copy_matrix(const RealSymMatrix& rsm,
		   SurfpackMatrix<Real>& surfpack_matrix);

  /// merge cv, div, and drv vectors into a single ra array
  template<typename RealArrayType>
  void merge_variable_arrays(const RealVector& cv,  const IntVector& div,
			     const RealVector& drv, RealArrayType& ra);
  /// aggregate {continuous,discrete int,discrete real} variables 
  /// from SurrogateDataVars into ra
  template<typename RealArrayType>
  void sdv_to_realarray(const Pecos::SurrogateDataVars& sdv, RealArrayType& ra);
  /// aggregate {active,all} {continuous,discrete int,discrete real}
  /// variables into *pre-sized* array
  template<typename RealArrayType>
  void vars_to_realarray(const Variables& vars, RealArrayType& ra);
  /// aggregate active {continuous,discrete int,discrete real}
  /// variables into *pre-sized* array
  template<typename RealArrayType>
  void active_vars_to_realarray(const Variables& vars, RealArrayType& ra);
  /// aggregate all {continuous,discrete int,discrete real}
  /// variables into *pre-sized* array
  template<typename RealArrayType>
  void all_vars_to_realarray(const Variables& vars, RealArrayType& ra);

  /// retrieve the active or all labels over which the surrogate was built
  StringArray variable_labels(const Variables& vars) const;

  /// validate metric names and cross validation options
  void validate_metrics(const std::set<std::string>& allowed_metrics);

  /// compute number of folds from numFols/percentFold
  unsigned compute_folds();


  /// validate imported labels and initialize map if needed
  void map_variable_labels(const Variables& dfsm_vars,
			   const StringArray& approx_labels);

  /// when importing, take all view of vars and permute as needed
  template<typename RealArrayType>
  RealArrayType imported_eval_vars(const Variables& vars);

  /// If populated, reorder variables when evaluating surrogate
  /// these are indices into the Model's vars so approx_eval[i] = [model_vars[ind[i]]]
  std::vector<size_t> varsMapIndices;


  //
  //- Heading: Data
  //

  /// order of polynomial approximation
  unsigned short approxOrder;

  /// set of diagnostic metrics
  StringArray diagnosticSet;

  /// whether to perform cross validation
  bool crossValidateFlag;
  /// number of folds for CV
  unsigned numFolds;
  /// percentage of data for CV
  Real percentFold;
  /// whether to perform PRESS
  bool pressFlag;
};


inline SharedSurfpackApproxData::SharedSurfpackApproxData()
{ }


inline SharedSurfpackApproxData::~SharedSurfpackApproxData()
{ }


template<typename RealArrayType>
void SharedSurfpackApproxData::
merge_variable_arrays(const RealVector& cv,  const IntVector& div,
		      const RealVector& drv, RealArrayType& ra)
{
  // passed array must be sized due to length/size differences
  size_t num_cv = cv.length(), num_div = div.length(), num_drv = drv.length(),
         num_v  = num_cv + num_div + num_drv;
  if (num_cv)   copy_data_partial(cv,  ra, 0);
  if (num_div) merge_data_partial(div, ra, num_cv);
  if (num_drv)  copy_data_partial(drv, ra, num_cv+num_div);
}


template<typename RealArrayType>
void SharedSurfpackApproxData::
sdv_to_realarray(const Pecos::SurrogateDataVars& sdv, RealArrayType& ra)
{
  // passed array must be sized due to length/size differences

  // check incoming vars for correct length (active or all views)
  const RealVector&  cv = sdv.continuous_variables();
  const IntVector&  div = sdv.discrete_int_variables();
  const RealVector& drv = sdv.discrete_real_variables();
  if (cv.length() + div.length() + drv.length() == numVars)
    merge_variable_arrays(cv, div, drv, ra);
  else {
    Cerr << "Error: bad parameter set length in SharedSurfpackApproxData::"
	 << "sdv_to_realarray(): " << numVars << " != " << cv.length() << " + "
	 << div.length() << " + " << drv.length() << "." << std::endl;
    abort_handler(-1);
  }
}


template<typename RealArrayType>
void SharedSurfpackApproxData::
vars_to_realarray(const Variables& vars, RealArrayType& ra)
{
  // passed array ra must be sized due to length/size differences

  // check incoming vars for correct length (active or all views)
  if (vars.cv() + vars.div() + vars.drv() == numVars)
    active_vars_to_realarray(vars, ra);
  else if (vars.acv() + vars.adiv() + vars.adrv() == numVars)
    all_vars_to_realarray(vars, ra);
  else {
    Cerr << "Error: bad parameter set length in SharedSurfpackApproxData::"
	 << "vars_to_realarray()." << std::endl;
    abort_handler(-1);
  }
}


template<typename RealArrayType>
void SharedSurfpackApproxData::
active_vars_to_realarray(const Variables& vars, RealArrayType& ra)
{
  // passed array ra must be sized due to length/size differences
  merge_variable_arrays(vars.continuous_variables(),
			vars.discrete_int_variables(),
			vars.discrete_real_variables(), ra);
}


template<typename RealArrayType>
void SharedSurfpackApproxData::
all_vars_to_realarray(const Variables& vars, RealArrayType& ra)
{
  // passed array ra must be sized due to length/size differences
  merge_variable_arrays(vars.all_continuous_variables(),
			vars.all_discrete_int_variables(),
			vars.all_discrete_real_variables(), ra);
}


template<typename RealArrayType>
RealArrayType
SharedSurfpackApproxData::imported_eval_vars(const Variables& vars)
{
  RealArrayType all_vars(vars.acv() + vars.adiv() + vars.adrv());
  all_vars_to_realarray(vars, all_vars);

  if (varsMapIndices.empty()) {
    return all_vars;
  }
  else {
    RealArrayType eval_vars(varsMapIndices.size());
    for (size_t i=0; i<varsMapIndices.size(); ++i)
      eval_vars[i] = all_vars[varsMapIndices[i]];
    return eval_vars;
  }
}


} // namespace Dakota
#endif
