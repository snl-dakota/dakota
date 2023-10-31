/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef MINIMIZER_ADAPTER_MODEL_H
#define MINIMIZER_ADAPTER_MODEL_H

#include "AdapterModel.hpp"

namespace Dakota {


/// Derived model class which wraps call-back functions for solving
/// minimization sub-problems.
  
/** The MinimizerAdapterModel class uses C-style function pointers to:
    (a) allow use of existing Minimizer constructor APIs that utilize
    an incoming Model to extract sub-problem data, and (b) enable Model
    recursions on top of these call-backs. */

class MinimizerAdapterModel: public AdapterModel
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// standard full constructor with minimizer-specific bounds/targets;
  /// doubles as a partial constructor given default value for response
  /// mapping function pointer
  MinimizerAdapterModel(const RealVector& cv_initial_pt,
			const RealVector& cv_lower_bnds,
			const RealVector& cv_upper_bnds,
			const RealMatrix& lin_ineq_coeffs,
			const RealVector& lin_ineq_lower_bnds,
			const RealVector& lin_ineq_upper_bnds,
			const RealMatrix& lin_eq_coeffs,
			const RealVector& lin_eq_targets,
			const RealVector& nonlin_ineq_lower_bnds,
			const RealVector& nonlin_ineq_upper_bnds,
			const RealVector& nonlin_eq_targets,
			void (*resp_map) (const Variables& vars,
					  const ActiveSet& set,
					  Response& response) = NULL);

  // this alternate constructor can currently be handled with instantations of
  // base AdapterModel (similar to use of RecastModel vs. its specializations)
  //MinimizerAdapterModel(const Variables& initial_pt, const Constraints& cons,
  //			const Response& resp,
  //			void (*resp_map) (const Variables& vars,
  //					  const ActiveSet& set,
  //					  Response& response) = NULL);

  /// alternate partial constructor; constructs response map but
  /// requires subsequent initialize/assign calls
  MinimizerAdapterModel(void (*resp_map) (const Variables& vars,
					  const ActiveSet& set,
					  Response& response));

  /// destructor
  ~MinimizerAdapterModel();

  //
  //- Heading: Member functions
  //

  /// initialize currentVariables
  void initialize_variables(size_t num_cdv);//, num_div, num_drv, ...
  /// initialize userDefinedConstraints
  void initialize_constraints();
  /// initialize currentResponse
  void initialize_response();

  /// assign variable values
  void assign_variables(const RealVector& c_vars);//,di_vars,dr_vars,...
  /// initialize constraint bounds/targets after alternate construction
  void assign_constraints(const RealVector& cv_lower_bnds,
			  const RealVector& cv_upper_bnds,
			  const RealMatrix& lin_ineq_coeffs,
			  const RealVector& lin_ineq_lower_bnds,
			  const RealVector& lin_ineq_upper_bnds,
			  const RealMatrix& lin_eq_coeffs,
			  const RealVector& lin_eq_targets,
			  const RealVector& nln_ineq_lower_bnds,
			  const RealVector& nln_ineq_upper_bnds,
			  const RealVector& nln_eq_targets);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //
  //- Heading: Data members
  //

private:

  //
  //- Heading: Convenience member functions
  //

  //
  //- Heading: Data members
  //

};


inline MinimizerAdapterModel::~MinimizerAdapterModel()
{ }


inline void MinimizerAdapterModel::
initialize_variables(size_t num_cdv)//, num_div, num_drv, ...
{
  ShortShortPair view(MIXED_DESIGN, EMPTY_VIEW);
  SizetArray vc_totals;  vc_totals.assign(NUM_VC_TOTALS, 0);
  vc_totals[TOTAL_CDV] = num_cdv;
  //vc_totals[TOTAL_*DV] = num_*dv;
  SharedVariablesData svd(view, vc_totals);
  currentVariables = Variables(svd);
  numDerivVars = num_cdv;
}


inline void MinimizerAdapterModel::initialize_constraints()
{ userDefinedConstraints = Constraints(currentVariables.shared_data()); }


inline void MinimizerAdapterModel::initialize_response()
{
  // must follow initialize_variables() and assign_constraints()

  ActiveSet set(numFns, numDerivVars); // order dependency
  SharedResponseData srd(set);
  currentResponse = Response(srd, set);
}


inline void MinimizerAdapterModel::
assign_variables(const RealVector& c_vars)//, di_vars, dr_vars, ...
{
  if (currentVariables.is_null())
    initialize_variables(c_vars.length());// .length(), .length(), ...

  currentVariables.continuous_variables(c_vars);
  //currentVariables.discrete_int_variables(di_vars);
  //currentVariables.discrete_real_variables(dr_vars);
}


inline void MinimizerAdapterModel::
assign_constraints(const RealVector& cv_lower_bnds,
		   const RealVector& cv_upper_bnds,
		   const RealMatrix& lin_ineq_coeffs,
		   const RealVector& lin_ineq_lower_bnds,
		   const RealVector& lin_ineq_upper_bnds,
		   const RealMatrix& lin_eq_coeffs,
		   const RealVector& lin_eq_targets,
		   const RealVector& nln_ineq_lower_bnds,
		   const RealVector& nln_ineq_upper_bnds,
		   const RealVector& nln_eq_targets)
{
  if (userDefinedConstraints.is_null())
    initialize_constraints();

  userDefinedConstraints.continuous_lower_bounds(cv_lower_bnds);
  userDefinedConstraints.continuous_upper_bounds(cv_upper_bnds);

  userDefinedConstraints.linear_ineq_constraint_coeffs(lin_ineq_coeffs);
  userDefinedConstraints.
    linear_ineq_constraint_lower_bounds(lin_ineq_lower_bnds);
  userDefinedConstraints.
    linear_ineq_constraint_upper_bounds(lin_ineq_upper_bnds);
  userDefinedConstraints.linear_eq_constraint_coeffs(lin_eq_coeffs);
  userDefinedConstraints.linear_eq_constraint_targets(lin_eq_targets);

  userDefinedConstraints.
    nonlinear_ineq_constraint_lower_bounds(nln_ineq_lower_bnds);
  userDefinedConstraints.
    nonlinear_ineq_constraint_upper_bounds(nln_ineq_upper_bnds);
  userDefinedConstraints.nonlinear_eq_constraint_targets(nln_eq_targets);

  numFns = 1 + nln_ineq_lower_bnds.length() + nln_eq_targets.length();
}

} // namespace Dakota

#endif
