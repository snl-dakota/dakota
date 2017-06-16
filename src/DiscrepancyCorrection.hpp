/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DiscrepancyCorrection
//- Description: A model which provides a discrepancy for a truth model.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: DiscrepancyCorrection.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef DISCREPANCY_CORRECTION_H
#define DISCREPANCY_CORRECTION_H

#include "SharedApproxData.hpp"
#include "DakotaApproximation.hpp"
#include "DakotaModel.hpp"
#include "DataModel.hpp"


namespace Dakota {


/// Base class for discrepancy corrections

/** The DiscrepancyCorrection class provides common functions for
    computing and applying corrections to approximations. */

class DiscrepancyCorrection
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  DiscrepancyCorrection();
  /// standard constructor
  DiscrepancyCorrection(Model& surr_model, const IntSet& surr_fn_indices,
			short corr_type, short corr_order);
  /// alternate constructor
  DiscrepancyCorrection(const IntSet& surr_fn_indices, size_t num_fns,
			size_t num_vars, short corr_type, short corr_order);
  /// destructor
  ~DiscrepancyCorrection();

  //
  //- Heading: Member functions
  //

  /// initialize the DiscrepancyCorrection data
  void initialize(Model& surr_model, const IntSet& surr_fn_indices,
		  short corr_type, short corr_order);
  /// initialize the DiscrepancyCorrection data
  void initialize(const IntSet& surr_fn_indices, size_t num_fns,
		  size_t num_vars, short corr_type, short corr_order);
  /// initialize the DiscrepancyCorrection data
  void initialize(const IntSet& surr_fn_indices, size_t num_fns,
		  size_t num_vars, short corr_type, short corr_order,
		  const String& approx_type);

  /// compute the correction required to bring approx_response into
  /// agreement with truth_response and store in {add,mult}Corrections
  void compute(const Variables& vars, const Response& truth_response, 
	       const Response& approx_response, bool quiet_flag = false);
  /// compute the correction required to bring approx_response into
  /// agreement with truth_response and store in discrepancy_response
  void compute(//const Variables& vars,
	       const Response& truth_response, const Response& approx_response,
	       Response& discrepancy_response, bool quiet_flag = false);
  /// compute the correction required to bring approx_response into
  /// agreement with truth_response as a function of the variables
  /// and store in {add,mult}Corrections
  void compute(const VariablesArray& vars_array, const ResponseArray& 
               truth_response_array, const ResponseArray& approx_response, 
	       bool quiet_flag = false);

  /// apply the correction computed in compute() to approx_response
  void apply(const Variables& vars, Response& approx_response,
	     bool quiet_flag = false);

  /// compute the variance of approx_response
  void compute_variance(const VariablesArray& vars_array, RealMatrix& 
      			approx_variance, bool quiet_flag = false); 

  /// return correctionType
  short correction_type() const;
  /// return correctionOrder
  short correction_order() const;
  /// return dataOrder
  short data_order() const;
  /// return correctionComputed
  bool computed() const;
  /// return initializedFlag
  bool initialized() const;

protected:

  //
  //- Heading: Data
  //

  /// for mixed response sets, this array specifies the response function
  /// subset that is approximated
  IntSet surrogateFnIndices;

  /// indicates that discrepancy correction instance has been
  /// initialized following construction
  bool initializedFlag;

  /// approximation correction approach to be used: NO_CORRECTION,
  /// ADDITIVE_CORRECTION, MULTIPLICATIVE_CORRECTION, or COMBINED_CORRECTION.
  short correctionType;
  /// approximation correction order to be used: 0, 1, or 2
  short correctionOrder;
  /// order of correction data in 3-bit format: overlay of 1 (value),
  /// 2 (gradient), and 4 (Hessian)
  short dataOrder;

  /// flag indicating whether or not a correction has been computed
  /// and is available for application
  bool correctionComputed;

  /// total number of response functions (of which surrogateFnIndices
  /// may define a subset)
  size_t numFns;
  /// number of continuous variables active in the correction
  size_t numVars;

private:

  //
  //- Heading: Convenience functions
  //

  /// internal convenience function shared by overloaded initialize() variants
  void initialize_corrections();
  
  /// define badScalingFlag
  bool check_scaling(const RealVector& truth_fns, const RealVector& approx_fns);

  /// internal convenience function for computing additive corrections
  /// between truth and approximate responses
  void compute_additive(const Response& truth_response,
			const Response& approx_response, int index,
			Real& discrep_fn, RealVector& discrep_grad,
			RealSymMatrix& discrep_hess);
  /// internal convenience function for computing multiplicative corrections
  /// between truth and approximate responses
  void compute_multiplicative(const Response& truth_response,
			      const Response& approx_response, int index,
			      Real& discrep_fn, RealVector& discrep_grad,
			      RealSymMatrix& discrep_hess);

  /// internal convenience function for applying additive corrections
  /// to an approximate response
  void apply_additive(const Variables& vars, Response& approx_response);
  /// internal convenience function for applying multiplicative
  /// corrections to an approximate response
  void apply_multiplicative(const Variables& vars,
			    Response& approx_response);

  /// internal convenience function for applying additive corrections
  /// to a set of response functions
  void apply_additive(const Variables& vars, RealVector& approx_fns);
  /// internal convenience function for applying multiplicative corrections
  /// to a set of response functions
  void apply_multiplicative(const Variables& vars, RealVector& approx_fns);

  /// search data_pairs for missing approximation data
  const Response& search_db(const  Variables& search_vars,
			    const ShortArray& search_asv);

  //
  //- Heading: Data
  //

  /// flag used to indicate function values near zero for multiplicative
  /// corrections; triggers an automatic switch to additive corrections
  bool badScalingFlag;
  /// flag indicating the need for additive correction calculations
  bool computeAdditive;
  /// flag indicating the need for multiplicative correction calculations
  bool computeMultiplicative;

  /// string indicating the discrepancy approximation type
  String approxType;

  /// data that is shared among all correction Approximations
  SharedApproxData sharedData;
  /// array of additive corrections; surrogate models of a model
  /// discrepancy function (formed from model differences)
  std::vector<Approximation> addCorrections;
  /// array of multiplicative corrections; surrogate models of a model
  /// discrepancy function (formed from model ratios)
  std::vector<Approximation> multCorrections;

  /// shallow copy of the surrogate model instance as returned by
  /// Model::surrogate_model() (the DataFitSurrModel or
  /// HierarchSurrModel::lowFidelityModel instance) 
  Model surrModel;

  /// factors for combining additive and multiplicative corrections.
  /// Each factor is the weighting applied to the additive correction and
  /// 1.-factor is the weighting applied to the multiplicative correction.
  /// The factor value is determined by an additional requirement to match
  /// the high fidelity function value at the previous correction point
  /// (e.g., previous trust region center).  This results in a multipoint
  /// correction instead of a strictly local correction.
  RealVector combineFactors;

  // point in parameter space where the current correction is calculated
  // (often the center of the current trust region)
  //RealVector correctionCenterPt;
  /// copy of center point from the previous correction cycle
  Variables correctionPrevCenterPt;
  /// truth function values at the current correction point
  RealVector truthFnsCenter;
  /// Surrogate function values at the current correction point
  RealVector approxFnsCenter;
  /// Surrogate gradient values at the current correction point
  RealMatrix approxGradsCenter;
  /// copy of truth function values at center of previous correction cycle
  RealVector truthFnsPrevCenter;
  /// copy of approximate function values at center of previous correction cycle
  RealVector approxFnsPrevCenter;
};


inline DiscrepancyCorrection::DiscrepancyCorrection():
  initializedFlag(false), correctionType(NO_CORRECTION), 
  correctionOrder(0), dataOrder(1), correctionComputed(false),
  computeAdditive(false), computeMultiplicative(false)
{ }


inline DiscrepancyCorrection::
DiscrepancyCorrection(Model& surr_model, const IntSet& surr_fn_indices,
		      short corr_type, short corr_order)
{ initialize(surr_model, surr_fn_indices, corr_type, corr_order); }


inline DiscrepancyCorrection::~DiscrepancyCorrection()
{ }


inline short DiscrepancyCorrection::correction_type() const
{ return correctionType; }


inline short DiscrepancyCorrection::correction_order() const
{ return correctionOrder; }


inline short DiscrepancyCorrection::data_order() const
{ return dataOrder; }


inline bool DiscrepancyCorrection::computed() const
{ return correctionComputed; }


inline bool DiscrepancyCorrection::initialized() const
{ return initializedFlag; }


inline void DiscrepancyCorrection::
apply_additive(const Variables& vars, RealVector& approx_fns)
{
  for (ISIter it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
    approx_fns[*it] += addCorrections[*it].value(vars);
}


inline void DiscrepancyCorrection::
apply_multiplicative(const Variables& vars, RealVector& approx_fns)
{
  for (ISIter it=surrogateFnIndices.begin(); it!=surrogateFnIndices.end(); ++it)
    approx_fns[*it] *= multCorrections[*it].value(vars);
}

} // namespace Dakota

#endif
