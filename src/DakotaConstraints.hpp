/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Dakota::Constraints
//- Description: Container class of variables and bounds, each with fundamental
//-              arrays (from specification) and derived arrays (for special 
//-              cases).
//- Owner:       Mike Eldred
//- Version: $Id: DakotaConstraints.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef DAKOTA_CONSTRAINTS_H
#define DAKOTA_CONSTRAINTS_H

#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"  // for BaseConstructor
#include "SharedVariablesData.hpp"


namespace Dakota {

class ProblemDescDB;


/// Base class for the variable constraints class hierarchy.

/** The Constraints class is the base class for the class hierarchy
    managing bound, linear, and nonlinear constraints.  Using the
    variable lower and upper bounds arrays from the input
    specification, different derived classes define different views of
    this data.  The linear and nonlinear constraint data is consistent
    in all views and is managed at the base class level.  For memory
    efficiency and enhanced polymorphism, the variable constraints
    hierarchy employs the "letter/envelope idiom" (see Coplien
    "Advanced C++", p. 133), for which the base class (Constraints)
    serves as the envelope and one of the derived classes (selected in
    Constraints::get_constraints()) serves as the letter. */

class Constraints
{
public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// default constructor
  Constraints();
  /// standard constructor
  Constraints(const ProblemDescDB& prob_db, const SharedVariablesData& svd);
  // alternate constructor for minimal instantiations on the fly (reshape reqd)
  //Constraints(const std::pair<short,short>& view);
  /// alternate constructor for instantiations on the fly
  Constraints(const SharedVariablesData& svd);
  /// copy constructor
  Constraints(const Constraints& con);

  /// destructor
  virtual ~Constraints();

  /// assignment operator
  Constraints operator=(const Constraints& con);

  //
  //- Heading: Virtual functions
  //

  /// write a variable constraints object to an std::ostream
  virtual void write(std::ostream& s) const;
  /// read a variable constraints object from an std::istream
  virtual void read(std::istream& s);

  //
  //- Heading: Member functions
  //

  // ACTIVE VARIABLES

  /// return the active continuous variable lower bounds
  const RealVector& continuous_lower_bounds() const;
  /// return an active continuous variable lower bound
  Real continuous_lower_bound(size_t i) const;
  /// set the active continuous variable lower bounds
  void continuous_lower_bounds(const RealVector& cl_bnds);
  /// set an active continuous variable lower bound
  void continuous_lower_bound(Real cl_bnd, size_t i);
  /// return the active continuous variable upper bounds
  const RealVector& continuous_upper_bounds() const;
  /// return an active continuous variable upper bound
  Real continuous_upper_bound(size_t i) const;
  /// set the active continuous variable upper bounds
  void continuous_upper_bounds(const RealVector& cu_bnds);
  /// set an active continuous variable upper bound
  void continuous_upper_bound(Real cu_bnd, size_t i);

  /// return the active discrete variable lower bounds
  const IntVector& discrete_int_lower_bounds() const;
  /// return an active discrete variable lower bound
  int discrete_int_lower_bound(size_t i) const;
  /// set the active discrete variable lower bounds
  void discrete_int_lower_bounds(const IntVector& dil_bnds);
  /// set an active discrete variable lower bound
  void discrete_int_lower_bound(int dil_bnd, size_t i);
  /// return the active discrete variable upper bounds
  const IntVector& discrete_int_upper_bounds() const;
  /// return an active discrete variable upper bound
  int discrete_int_upper_bound(size_t i) const;
  /// set the active discrete variable upper bounds
  void discrete_int_upper_bounds(const IntVector& diu_bnds);
  /// set an active discrete variable upper bound
  void discrete_int_upper_bound(int diu_bnd, size_t i);

  /// return the active discrete variable lower bounds
  const RealVector& discrete_real_lower_bounds() const;
  /// return an active discrete variable lower bound
  Real discrete_real_lower_bound(size_t i) const;
  /// set the active discrete variable lower bounds
  void discrete_real_lower_bounds(const RealVector& drl_bnds);
  /// set an active discrete variable lower bound
  void discrete_real_lower_bound(Real drl_bnd, size_t i);
  /// return the active discrete variable upper bounds
  const RealVector& discrete_real_upper_bounds() const;
  /// return an active discrete variable upper bound
  Real discrete_real_upper_bound(size_t i) const;
  /// set the active discrete variable upper bounds
  void discrete_real_upper_bounds(const RealVector& dru_bnds);
  /// set an active discrete variable upper bound
  void discrete_real_upper_bound(Real dru_bnd, size_t i);

  // INACTIVE VARIABLES

  /// return the inactive continuous lower bounds
  const RealVector& inactive_continuous_lower_bounds() const;
  /// set the inactive continuous lower bounds
  void inactive_continuous_lower_bounds(const RealVector& icl_bnds);
  /// return the inactive continuous upper bounds
  const RealVector& inactive_continuous_upper_bounds() const;
  /// set the inactive continuous upper bounds
  void inactive_continuous_upper_bounds(const RealVector& icu_bnds);

  /// return the inactive discrete lower bounds
  const IntVector& inactive_discrete_int_lower_bounds() const;
  /// set the inactive discrete lower bounds
  void inactive_discrete_int_lower_bounds(const IntVector& idil_bnds);
  /// return the inactive discrete upper bounds
  const IntVector& inactive_discrete_int_upper_bounds() const;
  /// set the inactive discrete upper bounds
  void inactive_discrete_int_upper_bounds(const IntVector& idiu_bnds);

  /// return the inactive discrete lower bounds
  const RealVector& inactive_discrete_real_lower_bounds() const;
  /// set the inactive discrete lower bounds
  void inactive_discrete_real_lower_bounds(const RealVector& idrl_bnds);
  /// return the inactive discrete upper bounds
  const RealVector& inactive_discrete_real_upper_bounds() const;
  /// set the inactive discrete upper bounds
  void inactive_discrete_real_upper_bounds(const RealVector& idru_bnds);

  // ALL VARIABLES

  /// returns a single array with all continuous lower bounds
  const RealVector& all_continuous_lower_bounds() const;
  /// sets all continuous lower bounds using a single array
  void all_continuous_lower_bounds(const RealVector& acl_bnds);
  /// set a lower bound within the all continuous lower bounds array
  void all_continuous_lower_bound(Real acl_bnd, size_t i);
  /// returns a single array with all continuous upper bounds
  const RealVector& all_continuous_upper_bounds() const;
  /// sets all continuous upper bounds using a single array
  void all_continuous_upper_bounds(const RealVector& acu_bnds);
  /// set an upper bound within the all continuous upper bounds array
  void all_continuous_upper_bound(Real acu_bnd, size_t i);

  /// returns a single array with all discrete lower bounds
  const IntVector& all_discrete_int_lower_bounds() const;
  /// sets all discrete lower bounds using a single array
  void all_discrete_int_lower_bounds(const IntVector& adil_bnds);
  /// set a lower bound within the all discrete lower bounds array
  void all_discrete_int_lower_bound(int adil_bnd, size_t i);
  /// returns a single array with all discrete upper bounds
  const IntVector& all_discrete_int_upper_bounds() const;
  /// sets all discrete upper bounds using a single array
  void all_discrete_int_upper_bounds(const IntVector& adiu_bnds);
  /// set an upper bound within the all discrete upper bounds array
  void all_discrete_int_upper_bound(int adiu_bnd, size_t i);

  /// returns a single array with all discrete lower bounds
  const RealVector& all_discrete_real_lower_bounds() const;
  /// sets all discrete lower bounds using a single array
  void all_discrete_real_lower_bounds(const RealVector& adrl_bnds);
  /// set a lower bound within the all discrete lower bounds array
  void all_discrete_real_lower_bound(Real adrl_bnd, size_t i);
  /// returns a single array with all discrete upper bounds
  const RealVector& all_discrete_real_upper_bounds() const;
  /// sets all discrete upper bounds using a single array
  void all_discrete_real_upper_bounds(const RealVector& adru_bnds);
  /// set an upper bound within the all discrete upper bounds array
  void all_discrete_real_upper_bound(Real adru_bnd, size_t i);

  // LINEAR CONSTRAINTS

  /// return the number of linear inequality constraints
  size_t num_linear_ineq_constraints() const;
  /// return the number of linear equality constraints
  size_t num_linear_eq_constraints() const;

  /// return the linear inequality constraint coefficients
  const RealMatrix& linear_ineq_constraint_coeffs() const;
  /// set the linear inequality constraint coefficients
  void linear_ineq_constraint_coeffs(const RealMatrix& lin_ineq_coeffs);
  /// return the linear inequality constraint lower bounds
  const RealVector& linear_ineq_constraint_lower_bounds() const;
  /// set the linear inequality constraint lower bounds
  void linear_ineq_constraint_lower_bounds(
    const RealVector& lin_ineq_l_bnds);
  /// return the linear inequality constraint upper bounds
  const RealVector& linear_ineq_constraint_upper_bounds() const;
  /// set the linear inequality constraint upper bounds
  void linear_ineq_constraint_upper_bounds(
    const RealVector& lin_ineq_u_bnds);
  /// return the linear equality constraint coefficients
  const RealMatrix& linear_eq_constraint_coeffs() const;
  /// set the linear equality constraint coefficients
  void linear_eq_constraint_coeffs(const RealMatrix& lin_eq_coeffs);
  /// return the linear equality constraint targets
  const RealVector& linear_eq_constraint_targets() const;
  /// set the linear equality constraint targets
  void linear_eq_constraint_targets(const RealVector& lin_eq_targets);

  // NONLINEAR CONSTRAINTS

  /// return the number of nonlinear inequality constraints
  size_t num_nonlinear_ineq_constraints() const;
  /// return the number of nonlinear equality constraints
  size_t num_nonlinear_eq_constraints() const;

  /// return the nonlinear inequality constraint lower bounds
  const RealVector& nonlinear_ineq_constraint_lower_bounds() const;
  /// set the nonlinear inequality constraint lower bounds
  void nonlinear_ineq_constraint_lower_bounds(
    const RealVector& nln_ineq_l_bnds);
  /// return the nonlinear inequality constraint upper bounds
  const RealVector& nonlinear_ineq_constraint_upper_bounds() const;
  /// set the nonlinear inequality constraint upper bounds
  void nonlinear_ineq_constraint_upper_bounds(
    const RealVector& nln_ineq_u_bnds);
  /// return the nonlinear equality constraint targets
  const RealVector& nonlinear_eq_constraint_targets() const;
  /// set the nonlinear equality constraint targets
  void nonlinear_eq_constraint_targets(const RealVector& nln_eq_targets);

  /// for use when a deep copy is needed (the representation is _not_ shared)
  Constraints copy() const;

  /// shape the lower/upper bound arrays based on sharedVarsData
  void shape();
  /// reshape the linear/nonlinear/bound constraint arrays arrays and
  /// the lower/upper bound arrays
  void reshape(size_t num_nln_ineq_cons, size_t num_nln_eq_cons,
	       size_t num_lin_ineq_cons, size_t num_lin_eq_cons,
	       const SharedVariablesData& svd);
  /// reshape the lower/upper bound arrays based on sharedVarsData
  void reshape();
  /// reshape the linear/nonlinear constraint arrays
  void reshape(size_t num_nln_ineq_cons, size_t num_nln_eq_cons,
	       size_t num_lin_ineq_cons, size_t num_lin_eq_cons);

  /// sets the inactive view based on higher level (nested) context
  void inactive_view(short view2);

  /// function to check constraintsRep (does this envelope contain a letter)
  bool is_null() const;

protected:

  //
  //- Heading: Constructors
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Constraints(BaseConstructor, const ProblemDescDB& problem_db,
	      const SharedVariablesData& svd);
  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  Constraints(BaseConstructor, const SharedVariablesData& svd);

  //
  //- Heading: Member functions
  //

  /// construct active/inactive views of all variables arrays
  void build_views();
  /// construct active views of all variables bounds arrays
  void build_active_views();
  /// construct inactive views of all variables bounds arrays
  void build_inactive_views();

  /// perform checks on user input, convert linear constraint
  /// coefficient input to matrices, and assign defaults
  void manage_linear_constraints(const ProblemDescDB& problem_db);

  //
  //- Heading: Data
  //

  /// configuration data shared from a Variables instance
  SharedVariablesData sharedVarsData;

  /// a continuous lower bounds array combining continuous design,
  /// uncertain, and continuous state variable types (all view).
  RealVector allContinuousLowerBnds;
  /// a continuous upper bounds array combining continuous design,
  /// uncertain, and continuous state variable types (all view).
  RealVector allContinuousUpperBnds;
  /// a discrete lower bounds array combining discrete design and
  /// discrete state variable types (all view).
  IntVector  allDiscreteIntLowerBnds;
  /// a discrete upper bounds array combining discrete design and
  /// discrete state variable types (all view).
  IntVector  allDiscreteIntUpperBnds;
  /// a discrete lower bounds array combining discrete design and
  /// discrete state variable types (all view).
  RealVector allDiscreteRealLowerBnds;
  /// a discrete upper bounds array combining discrete design and
  /// discrete state variable types (all view).
  RealVector allDiscreteRealUpperBnds;

  /// number of nonlinear inequality constraints
  size_t numNonlinearIneqCons;
  /// number of nonlinear equality constraints
  size_t numNonlinearEqCons;
  /// nonlinear inequality constraint lower bounds
  RealVector nonlinearIneqConLowerBnds;
  /// nonlinear inequality constraint upper bounds
  RealVector nonlinearIneqConUpperBnds;
  /// nonlinear equality constraint targets
  RealVector nonlinearEqConTargets;

  /// number of linear inequality constraints
  size_t numLinearIneqCons;
  /// number of linear equality constraints
  size_t numLinearEqCons;
  /// linear inequality constraint coefficients
  RealMatrix linearIneqConCoeffs; 
  /// linear equality constraint coefficients
  RealMatrix linearEqConCoeffs;
  /// linear inequality constraint lower bounds
  RealVector linearIneqConLowerBnds;
  /// linear inequality constraint upper bounds
  RealVector linearIneqConUpperBnds;
  /// linear equality constraint targets
  RealVector linearEqConTargets;

  //
  //- Heading: Data views
  //

  /// the active continuous lower bounds array view
  RealVector continuousLowerBnds;
  /// the active continuous upper bounds array view
  RealVector continuousUpperBnds;
  /// the active discrete lower bounds array view
  IntVector discreteIntLowerBnds;
  /// the active discrete upper bounds array view
  IntVector discreteIntUpperBnds;
  /// the active discrete lower bounds array view
  RealVector discreteRealLowerBnds;
  /// the active discrete upper bounds array view
  RealVector discreteRealUpperBnds;

  /// the inactive continuous lower bounds array view
  RealVector inactiveContinuousLowerBnds;
  /// the inactive continuous upper bounds array view
  RealVector inactiveContinuousUpperBnds;
  /// the inactive discrete lower bounds array view
  IntVector inactiveDiscreteIntLowerBnds;
  /// the inactive discrete upper bounds array view
  IntVector inactiveDiscreteIntUpperBnds;
  /// the inactive discrete lower bounds array view
  RealVector inactiveDiscreteRealLowerBnds;
  /// the inactive discrete upper bounds array view
  RealVector inactiveDiscreteRealUpperBnds;

private:

  //
  //- Heading: Member functions
  //

  /// Used only by the constructor to initialize constraintsRep to the 
  /// appropriate derived type.
  Constraints* get_constraints(const ProblemDescDB& problem_db,
			       const SharedVariablesData& svd);
  /// Used by copy() to initialize constraintsRep to the appropriate
  /// derived type.
  Constraints* get_constraints(const SharedVariablesData& svd) const;

  //
  //- Heading: Data
  //

  /// pointer to the letter (initialized only for the envelope)
  Constraints* constraintsRep;
  /// number of objects sharing constraintsRep
  int referenceCount;
};


inline const RealVector& Constraints::continuous_lower_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->continuousLowerBnds : continuousLowerBnds;
}


inline Real Constraints::continuous_lower_bound(size_t i) const
{
  return (constraintsRep) ?
    constraintsRep->continuousLowerBnds[i] : continuousLowerBnds[i];
}


inline void Constraints::continuous_lower_bounds(const RealVector& cl_bnds)
{
  if (constraintsRep) constraintsRep->continuousLowerBnds.assign(cl_bnds);
  else                continuousLowerBnds.assign(cl_bnds);
}


inline void Constraints::continuous_lower_bound(Real cl_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->continuousLowerBnds[i] = cl_bnd;
  else                continuousLowerBnds[i] = cl_bnd;
}


inline const RealVector& Constraints::continuous_upper_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->continuousUpperBnds : continuousUpperBnds;
}


inline Real Constraints::continuous_upper_bound(size_t i) const
{
  return (constraintsRep) ?
    constraintsRep->continuousUpperBnds[i] : continuousUpperBnds[i];
}


inline void Constraints::continuous_upper_bounds(const RealVector& cu_bnds)
{
  if (constraintsRep) constraintsRep->continuousUpperBnds.assign(cu_bnds);
  else                continuousUpperBnds.assign(cu_bnds);
}


inline void Constraints::continuous_upper_bound(Real cu_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->continuousUpperBnds[i] = cu_bnd;
  else                continuousUpperBnds[i] = cu_bnd;
}


inline const IntVector& Constraints::discrete_int_lower_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->discreteIntLowerBnds : discreteIntLowerBnds;
}


inline int Constraints::discrete_int_lower_bound(size_t i) const
{
  return (constraintsRep) ?
    constraintsRep->discreteIntLowerBnds[i] : discreteIntLowerBnds[i];
}


inline void Constraints::discrete_int_lower_bounds(const IntVector& dil_bnds)
{
  if (constraintsRep) constraintsRep->discreteIntLowerBnds.assign(dil_bnds);
  else                discreteIntLowerBnds.assign(dil_bnds);
}


inline void Constraints::discrete_int_lower_bound(int dil_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->discreteIntLowerBnds[i] = dil_bnd;
  else                discreteIntLowerBnds[i] = dil_bnd;
}


inline const IntVector& Constraints::discrete_int_upper_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->discreteIntUpperBnds : discreteIntUpperBnds;
}


inline int Constraints::discrete_int_upper_bound(size_t i) const
{
  return (constraintsRep) ?
    constraintsRep->discreteIntUpperBnds[i] : discreteIntUpperBnds[i];
}


inline void Constraints::discrete_int_upper_bounds(const IntVector& diu_bnds)
{
  if (constraintsRep) constraintsRep->discreteIntUpperBnds.assign(diu_bnds);
  else                discreteIntUpperBnds.assign(diu_bnds);
}


inline void Constraints::discrete_int_upper_bound(int diu_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->discreteIntUpperBnds[i] = diu_bnd;
  else                discreteIntUpperBnds[i] = diu_bnd;
}


inline const RealVector& Constraints::discrete_real_lower_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->discreteRealLowerBnds : discreteRealLowerBnds;
}


inline Real Constraints::discrete_real_lower_bound(size_t i) const
{
  return (constraintsRep) ?
    constraintsRep->discreteRealLowerBnds[i] : discreteRealLowerBnds[i];
}


inline void Constraints::discrete_real_lower_bounds(const RealVector& drl_bnds)
{
  if (constraintsRep) constraintsRep->discreteRealLowerBnds.assign(drl_bnds);
  else                discreteRealLowerBnds.assign(drl_bnds);
}


inline void Constraints::discrete_real_lower_bound(Real drl_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->discreteRealLowerBnds[i] = drl_bnd;
  else                discreteRealLowerBnds[i] = drl_bnd;
}


inline const RealVector& Constraints::discrete_real_upper_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->discreteRealUpperBnds : discreteRealUpperBnds;
}


inline Real Constraints::discrete_real_upper_bound(size_t i) const
{
  return (constraintsRep) ?
    constraintsRep->discreteRealUpperBnds[i] : discreteRealUpperBnds[i];
}


inline void Constraints::discrete_real_upper_bounds(const RealVector& dru_bnds)
{
  if (constraintsRep) constraintsRep->discreteRealUpperBnds.assign(dru_bnds);
  else                discreteRealUpperBnds.assign(dru_bnds);
}


inline void Constraints::discrete_real_upper_bound(Real dru_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->discreteRealUpperBnds[i] = dru_bnd;
  else                discreteRealUpperBnds[i] = dru_bnd;
}


inline const RealVector& Constraints::inactive_continuous_lower_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->inactiveContinuousLowerBnds : inactiveContinuousLowerBnds;
}


inline void Constraints::
inactive_continuous_lower_bounds(const RealVector& icl_bnds)
{
  if (constraintsRep)
    constraintsRep->inactiveContinuousLowerBnds.assign(icl_bnds);
  else
    inactiveContinuousLowerBnds.assign(icl_bnds);
}


inline const RealVector& Constraints::inactive_continuous_upper_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->inactiveContinuousUpperBnds : inactiveContinuousUpperBnds;
}


inline void Constraints::
inactive_continuous_upper_bounds(const RealVector& icu_bnds)
{
  if (constraintsRep)
    constraintsRep->inactiveContinuousUpperBnds.assign(icu_bnds);
  else
    inactiveContinuousUpperBnds.assign(icu_bnds);
}


inline const IntVector& Constraints::inactive_discrete_int_lower_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->inactiveDiscreteIntLowerBnds : inactiveDiscreteIntLowerBnds;
}


inline void Constraints::
inactive_discrete_int_lower_bounds(const IntVector& idil_bnds)
{
  if (constraintsRep)
    constraintsRep->inactiveDiscreteIntLowerBnds.assign(idil_bnds);
  else
    inactiveDiscreteIntLowerBnds.assign(idil_bnds);
}


inline const IntVector& Constraints::inactive_discrete_int_upper_bounds() const
{
  return(constraintsRep) ?
    constraintsRep->inactiveDiscreteIntUpperBnds : inactiveDiscreteIntUpperBnds;
}


inline void Constraints::
inactive_discrete_int_upper_bounds(const IntVector& idiu_bnds)
{
  if (constraintsRep)
    constraintsRep->inactiveDiscreteIntUpperBnds.assign(idiu_bnds);
  else
    inactiveDiscreteIntUpperBnds.assign(idiu_bnds);
}


inline const RealVector& Constraints::
inactive_discrete_real_lower_bounds() const
{
  return (constraintsRep) ? constraintsRep->inactiveDiscreteRealLowerBnds :
    inactiveDiscreteRealLowerBnds;
}


inline void Constraints::
inactive_discrete_real_lower_bounds(const RealVector& idrl_bnds)
{
  if (constraintsRep)
    constraintsRep->inactiveDiscreteRealLowerBnds.assign(idrl_bnds);
  else
    inactiveDiscreteRealLowerBnds.assign(idrl_bnds);
}


inline const RealVector& Constraints::
inactive_discrete_real_upper_bounds() const
{
  return(constraintsRep) ? constraintsRep-> inactiveDiscreteRealUpperBnds :
    inactiveDiscreteRealUpperBnds;
}


inline void Constraints::
inactive_discrete_real_upper_bounds(const RealVector& idru_bnds)
{
  if (constraintsRep)
    constraintsRep->inactiveDiscreteRealUpperBnds.assign(idru_bnds);
  else
    inactiveDiscreteRealUpperBnds.assign(idru_bnds);
}


inline const RealVector& Constraints::all_continuous_lower_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->allContinuousLowerBnds : allContinuousLowerBnds;
}


inline void Constraints::
all_continuous_lower_bounds(const RealVector& acl_bnds)
{
  if (constraintsRep) constraintsRep->allContinuousLowerBnds.assign(acl_bnds);
  else                allContinuousLowerBnds.assign(acl_bnds);
}


inline void Constraints::all_continuous_lower_bound(Real acl_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->allContinuousLowerBnds[i] = acl_bnd;
  else                allContinuousLowerBnds[i] = acl_bnd;
}


inline const RealVector& Constraints::all_continuous_upper_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->allContinuousUpperBnds : allContinuousUpperBnds;
}


inline void Constraints::all_continuous_upper_bounds(const RealVector& acu_bnds)
{
  if (constraintsRep) constraintsRep->allContinuousUpperBnds.assign(acu_bnds);
  else                allContinuousUpperBnds.assign(acu_bnds);
}


inline void Constraints::all_continuous_upper_bound(Real acu_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->allContinuousUpperBnds[i] = acu_bnd;
  else                allContinuousUpperBnds[i] = acu_bnd;
}


inline const IntVector& Constraints::all_discrete_int_lower_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->allDiscreteIntLowerBnds : allDiscreteIntLowerBnds;
}


inline void Constraints::
all_discrete_int_lower_bounds(const IntVector& adil_bnds)
{
  if (constraintsRep) constraintsRep->allDiscreteIntLowerBnds.assign(adil_bnds);
  else                allDiscreteIntLowerBnds.assign(adil_bnds);
}


inline void Constraints::all_discrete_int_lower_bound(int adil_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->allDiscreteIntLowerBnds[i] = adil_bnd;
  else                allDiscreteIntLowerBnds[i] = adil_bnd;
}


inline const IntVector& Constraints::all_discrete_int_upper_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->allDiscreteIntUpperBnds : allDiscreteIntUpperBnds;
}


inline void Constraints::
all_discrete_int_upper_bounds(const IntVector& adiu_bnds)
{
  if (constraintsRep)
    constraintsRep->allDiscreteIntUpperBnds.assign(adiu_bnds);
  else
    allDiscreteIntUpperBnds.assign(adiu_bnds);
}


inline void Constraints::all_discrete_int_upper_bound(int adiu_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->allDiscreteIntUpperBnds[i] = adiu_bnd;
  else                allDiscreteIntUpperBnds[i] = adiu_bnd;
}


inline const RealVector& Constraints::all_discrete_real_lower_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->allDiscreteRealLowerBnds : allDiscreteRealLowerBnds;
}


inline void Constraints::
all_discrete_real_lower_bounds(const RealVector& adrl_bnds)
{
  if (constraintsRep)
    constraintsRep->allDiscreteRealLowerBnds.assign(adrl_bnds);
  else
    allDiscreteRealLowerBnds.assign(adrl_bnds);
}


inline void Constraints::all_discrete_real_lower_bound(Real adrl_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->allDiscreteRealLowerBnds[i] = adrl_bnd;
  else                allDiscreteRealLowerBnds[i] = adrl_bnd;
}


inline const RealVector& Constraints::all_discrete_real_upper_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->allDiscreteRealUpperBnds : allDiscreteRealUpperBnds;
}


inline void Constraints::
all_discrete_real_upper_bounds(const RealVector& adru_bnds)
{
  if (constraintsRep)
    constraintsRep->allDiscreteRealUpperBnds.assign(adru_bnds);
  else
    allDiscreteRealUpperBnds.assign(adru_bnds);
}


inline void Constraints::all_discrete_real_upper_bound(Real adru_bnd, size_t i)
{
  if (constraintsRep) constraintsRep->allDiscreteRealUpperBnds[i] = adru_bnd;
  else                allDiscreteRealUpperBnds[i] = adru_bnd;
}


// nonvirtual functions can access letter attributes directly (only need to fwd
// member function call when the function could be redefined).
inline size_t Constraints::num_linear_ineq_constraints() const
{
  return (constraintsRep) ? 
    constraintsRep->numLinearIneqCons : numLinearIneqCons;
}


inline size_t Constraints::num_linear_eq_constraints() const
{ return (constraintsRep) ? constraintsRep->numLinearEqCons : numLinearEqCons; }


inline const RealMatrix& Constraints::linear_ineq_constraint_coeffs() const
{
  return (constraintsRep) ?
    constraintsRep->linearIneqConCoeffs : linearIneqConCoeffs;
}


inline void Constraints::
linear_ineq_constraint_coeffs(const RealMatrix& lin_ineq_coeffs)
{
  if (constraintsRep) constraintsRep->linearIneqConCoeffs = lin_ineq_coeffs;
  else                linearIneqConCoeffs = lin_ineq_coeffs;
}


inline const RealVector& Constraints::
linear_ineq_constraint_lower_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->linearIneqConLowerBnds : linearIneqConLowerBnds;
}


inline void Constraints::
linear_ineq_constraint_lower_bounds(const RealVector& lin_ineq_l_bnds)
{
  if (constraintsRep) constraintsRep->linearIneqConLowerBnds = lin_ineq_l_bnds;
  else                linearIneqConLowerBnds = lin_ineq_l_bnds;
}


inline const RealVector& Constraints::
linear_ineq_constraint_upper_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->linearIneqConUpperBnds : linearIneqConUpperBnds;
}


inline void Constraints::
linear_ineq_constraint_upper_bounds(const RealVector& lin_ineq_u_bnds)
{
  if (constraintsRep) constraintsRep->linearIneqConUpperBnds = lin_ineq_u_bnds;
  else                linearIneqConUpperBnds = lin_ineq_u_bnds;
}


inline const RealMatrix& Constraints::linear_eq_constraint_coeffs() const
{
  return (constraintsRep) ?
    constraintsRep->linearEqConCoeffs : linearEqConCoeffs;
}


inline void Constraints::
linear_eq_constraint_coeffs(const RealMatrix& lin_eq_coeffs)
{
  if (constraintsRep) constraintsRep->linearEqConCoeffs = lin_eq_coeffs;
  else                linearEqConCoeffs = lin_eq_coeffs;
}


inline const RealVector& Constraints::linear_eq_constraint_targets() const
{
  return (constraintsRep) ?
    constraintsRep->linearEqConTargets : linearEqConTargets;
}


inline void Constraints::
linear_eq_constraint_targets(const RealVector& lin_eq_targets)
{
  if (constraintsRep) constraintsRep->linearEqConTargets = lin_eq_targets;
  else                linearEqConTargets = lin_eq_targets;
}


inline size_t Constraints::num_nonlinear_ineq_constraints() const
{
  return (constraintsRep) ?
    constraintsRep->numNonlinearIneqCons : numNonlinearIneqCons;
}


inline size_t Constraints::num_nonlinear_eq_constraints() const
{
  return (constraintsRep) ?
    constraintsRep->numNonlinearEqCons : numNonlinearEqCons;
}


inline const RealVector& Constraints::
nonlinear_ineq_constraint_lower_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->nonlinearIneqConLowerBnds : nonlinearIneqConLowerBnds;
}


inline void Constraints::
nonlinear_ineq_constraint_lower_bounds(const RealVector& nln_ineq_l_bnds)
{
  if (constraintsRep)
    constraintsRep->nonlinearIneqConLowerBnds = nln_ineq_l_bnds;
  else
    nonlinearIneqConLowerBnds = nln_ineq_l_bnds;
}


inline const RealVector& Constraints::
nonlinear_ineq_constraint_upper_bounds() const
{
  return (constraintsRep) ?
    constraintsRep->nonlinearIneqConUpperBnds : nonlinearIneqConUpperBnds;
}


inline void Constraints::
nonlinear_ineq_constraint_upper_bounds(const RealVector& nln_ineq_u_bnds)
{
  if (constraintsRep)
    constraintsRep->nonlinearIneqConUpperBnds = nln_ineq_u_bnds;
  else
    nonlinearIneqConUpperBnds = nln_ineq_u_bnds;
}


inline const RealVector& Constraints::
nonlinear_eq_constraint_targets() const
{
  return (constraintsRep) ?
    constraintsRep->nonlinearEqConTargets : nonlinearEqConTargets;
}


inline void Constraints::
nonlinear_eq_constraint_targets(const RealVector& nln_eq_targets)
{
  if (constraintsRep) constraintsRep->nonlinearEqConTargets = nln_eq_targets;
  else                nonlinearEqConTargets = nln_eq_targets;
}


inline bool Constraints::is_null() const
{ return (constraintsRep) ? false : true; }


inline void Constraints::build_views()
{
  // called only from letters
  const std::pair<short,short>& view = sharedVarsData.view();
  if (view.first  != EMPTY_VIEW)
    build_active_views();
  if (view.second != EMPTY_VIEW)
    build_inactive_views();
}


// Having overloaded operators call read/write means that the operators need 
// not be a friend to private data because read/write functions are public.

/// std::istream extraction operator for Constraints
inline std::istream& operator>>(std::istream& s, Constraints& con)
{ con.read(s); return s; }


/// std::ostream insertion operator for Constraints
inline std::ostream& operator<<(std::ostream& s, const Constraints& con)
{ con.write(s); return s; }

} // namespace Dakota

#endif
