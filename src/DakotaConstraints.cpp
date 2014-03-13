/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DakotaConstraints
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "dakota_system_defs.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaConstraints.hpp"
#include "RelaxedVarConstraints.hpp"
#include "MixedVarConstraints.hpp"
#include "dakota_data_util.hpp"

static const char rcsId[]="@(#) $Id: DakotaConstraints.cpp 7029 2010-10-22 00:17:02Z mseldre $";

namespace Dakota {


/** This constructor is the one which must build the base class data
    for all derived classes.  get_constraints() instantiates a derived
    class letter and the derived constructor selects this base class
    constructor in its initialization list (to avoid recursion in the
    base class constructor calling get_constraints() again).  Since
    the letter IS the representation, its rep pointer is set to NULL
    (an uninitialized pointer causes problems in ~Constraints). */
Constraints::
Constraints(BaseConstructor, const ProblemDescDB& problem_db,
	    const SharedVariablesData& svd):
  sharedVarsData(svd),
  numNonlinearIneqCons(
    problem_db.get_sizet("responses.num_nonlinear_inequality_constraints")),
  nonlinearIneqConLowerBnds(
    problem_db.get_rv("responses.nonlinear_inequality_lower_bounds")),
  nonlinearIneqConUpperBnds(
    problem_db.get_rv("responses.nonlinear_inequality_upper_bounds")),
  numNonlinearEqCons(
    problem_db.get_sizet("responses.num_nonlinear_equality_constraints")),
  nonlinearEqConTargets(
    problem_db.get_rv("responses.nonlinear_equality_targets")), 
  numLinearIneqCons(0), numLinearEqCons(0),
  linearIneqConLowerBnds(
    problem_db.get_rv("method.linear_inequality_lower_bounds")),
  linearIneqConUpperBnds(
    problem_db.get_rv("method.linear_inequality_upper_bounds")),
  linearEqConTargets(
    problem_db.get_rv("method.linear_equality_targets")),
  constraintsRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Constraints::Constraints(BaseConstructor) called to build base "
       << "class data for letter object." << std::endl;
#endif
}


/** This constructor is the one which must build the base class data
    for all derived classes.  get_constraints() instantiates a derived
    class letter and the derived constructor selects this base class
    constructor in its initialization list (to avoid recursion in the
    base class constructor calling get_constraints() again).  Since
    the letter IS the representation, its rep pointer is set to NULL
    (an uninitialized pointer causes problems in ~Constraints). */
Constraints::
Constraints(BaseConstructor, const SharedVariablesData& svd):
  sharedVarsData(svd), numNonlinearIneqCons(0), numNonlinearEqCons(0),
  numLinearIneqCons(0), numLinearEqCons(0), constraintsRep(NULL),
  referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Constraints::Constraints(BaseConstructor) called to build base "
       << "class data for letter object." << std::endl;
#endif
}


/** The default constructor: constraintsRep is NULL in this case (a
    populated problem_db is needed to build a meaningful Constraints
    object).  This makes it necessary to check for NULL in the copy
    constructor, assignment operator, and destructor. */
Constraints::Constraints(): constraintsRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Constraints::Constraints() called to build empty "
       << "variable constraints object." << std::endl;
#endif
}


/** The envelope constructor only needs to extract enough data to
    properly execute get_constraints, since the constructor
    overloaded with BaseConstructor builds the actual base class data
    inherited by the derived classes. */
Constraints::
Constraints(const ProblemDescDB& problem_db, const SharedVariablesData& svd):
  referenceCount(1) // not used since this is the envelope, not the letter
{
#ifdef REFCOUNT_DEBUG
  Cout << "Constraints::Constraints(ProblemDescDB&,cnst std:pair<short,short>&)"
       << " called to instantiate envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate variable constraints type
  constraintsRep = get_constraints(problem_db, svd);
  if (!constraintsRep) // bad type or insufficient memory
    abort_handler(-1);
}


/** Initializes constraintsRep to the appropriate derived type, as given
    by the variables view. */
Constraints* Constraints::
get_constraints(const ProblemDescDB& problem_db, const SharedVariablesData& svd)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter in get_constraints(ProblemDescDB&)."
       << std::endl;
#endif

  short active_view = svd.view().first;
  switch (active_view) {
  case MIXED_ALL: case MIXED_DESIGN: case MIXED_ALEATORY_UNCERTAIN:
  case MIXED_EPISTEMIC_UNCERTAIN: case MIXED_UNCERTAIN: case MIXED_STATE:
    return new MixedVarConstraints(problem_db, svd);   break;
  case RELAXED_ALL: case RELAXED_DESIGN: case RELAXED_ALEATORY_UNCERTAIN:
  case RELAXED_EPISTEMIC_UNCERTAIN: case RELAXED_UNCERTAIN: case RELAXED_STATE:
    return new RelaxedVarConstraints(problem_db, svd); break;
  default:
    Cerr << "Constraints active view " << active_view << " not currently "
	 << "supported in derived Constraints classes." << std::endl;
    return NULL;                                       break;
  }
}


/** Envelope constructor for instantiations on the fly.  This
    constructor executes get_constraints(view), which invokes the
    default derived/base constructors, followed by a reshape() based
    on vars_comps. */
Constraints::Constraints(const SharedVariablesData& svd):
  referenceCount(1) // not used since this is the envelope, not the letter
{
#ifdef REFCOUNT_DEBUG
  Cout << "Constraints::Constraints(const std::pair<short,short>&) called "
       << "to instantiate envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate variable constraints type
  constraintsRep = get_constraints(svd);
  if (!constraintsRep)
    abort_handler(-1);
}


/** Initializes constraintsRep to the appropriate derived type, as given by
    the variables view. The default derived class constructors are invoked. */
Constraints* Constraints::get_constraints(const SharedVariablesData& svd) const
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter in get_constraints(pair<short,short>&)"
       << std::endl;
#endif

  short active_view = svd.view().first;
  switch (active_view) {
  case MIXED_ALL: case MIXED_DESIGN: case MIXED_ALEATORY_UNCERTAIN:
  case MIXED_EPISTEMIC_UNCERTAIN: case MIXED_UNCERTAIN: case MIXED_STATE:
    return new MixedVarConstraints(svd);   break;
  case RELAXED_ALL: case RELAXED_DESIGN: case RELAXED_ALEATORY_UNCERTAIN:
  case RELAXED_EPISTEMIC_UNCERTAIN: case RELAXED_UNCERTAIN: case RELAXED_STATE:
    return new RelaxedVarConstraints(svd); break;
  default:
    Cerr << "Constraints active view " << active_view << " not currently "
	 << "supported in derived Constraints classes." << std::endl;
    return NULL;                           break;
  }
}


/** Copy constructor manages sharing of constraintsRep and incrementing
    of referenceCount. */
Constraints::Constraints(const Constraints& con)
{
  // Increment new (no old to decrement)
  constraintsRep = con.constraintsRep;
  if (constraintsRep) // Check for an assignment of NULL
    ++constraintsRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "Constraints::Constraints(Constraints&)" << std::endl;
  if (constraintsRep)
    Cout << "constraintsRep referenceCount = " << constraintsRep->referenceCount
	 << std::endl;
#endif
}


/** Assignment operator decrements referenceCount for old constraintsRep,
    assigns new constraintsRep, and increments referenceCount for new
    constraintsRep. */
Constraints Constraints::operator=(const Constraints& con)
{
  if (constraintsRep != con.constraintsRep) { // normal case: old != new
    // Decrement old
    if (constraintsRep) // Check for NULL
      if ( --constraintsRep->referenceCount == 0 ) 
	delete constraintsRep;
    // Assign and increment new
    constraintsRep = con.constraintsRep;
    if (constraintsRep) // Check for NULL
      ++constraintsRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "Constraints::operator=(Constraints&)" << std::endl;
  if (constraintsRep)
    Cout << "constraintsRep referenceCount = " << constraintsRep->referenceCount
	 << std::endl;
#endif

  return *this; // calls copy constructor since returned by value
}


/** Destructor decrements referenceCount and only deletes constraintsRep
    when referenceCount reaches zero. */
Constraints::~Constraints()
{ 
  // Check for NULL pointer 
  if (constraintsRep) {
    --constraintsRep->referenceCount;
#ifdef REFCOUNT_DEBUG
    Cout << "constraintsRep referenceCount decremented to " 
         << constraintsRep->referenceCount << std::endl;
#endif
    if (constraintsRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting constraintsRep" << std::endl;
#endif
      delete constraintsRep;
    }
  }
}


void Constraints::build_active_views()
{
  if (constraintsRep)
    constraintsRep->build_active_views(); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual build_active_views"
	 << "() function.\nNo default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Constraints::build_inactive_views()
{
  if (constraintsRep)
    constraintsRep->build_inactive_views(); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual build_inactive_views"
	 << "() function.\nNo default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Constraints::inactive_view(short view2)
{
  if (constraintsRep)
    constraintsRep->inactive_view(view2);
  else {
    short view1 = sharedVarsData.view().first;
    // If active view is {RELAXED,MIXED}_ALL, outer level active view is
    // aggregated in inner loop all view and inactive view remains EMPTY.
    // Disallow assignment of an inactive ALL view.
    if (view1 > MIXED_ALL && view2 > MIXED_ALL) {
      sharedVarsData.inactive_view(view2); // likely redundant with Variables
      //check_view_compatibility(); // performed in Variables
      build_inactive_views();
    }
  }
}


void Constraints::read(std::istream& s)
{
  if (constraintsRep)
    constraintsRep->read(s); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual read function.\n"
         << "No default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Constraints::write(std::ostream& s) const
{
  if (constraintsRep)
    constraintsRep->write(s); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual write function.\n"
         << "No default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


/** Deep copies are used for history mechanisms that catalogue
    permanent copies (should not change as the representation within
    userDefinedConstraints changes). */
Constraints Constraints::copy() const
{
  // the envelope class instantiates a new envelope and a new letter and copies
  // current attributes into the new objects.

#ifdef REFCOUNT_DEBUG
  Cout << "Constraints::copy() called to generate a deep copy with no "
       << "representation sharing." << std::endl;
#endif

  Constraints con; // new envelope: referenceCount=1, constraintsRep=NULL

  if (constraintsRep) {
    // new letter: allocate a constraintsRep
    con.constraintsRep = get_constraints(constraintsRep->sharedVarsData);

    // nonlinear constraints
    con.constraintsRep->numNonlinearIneqCons
      = constraintsRep->numNonlinearIneqCons;
    con.constraintsRep->numNonlinearEqCons = constraintsRep->numNonlinearEqCons;
    con.constraintsRep->nonlinearIneqConLowerBnds
      = constraintsRep->nonlinearIneqConLowerBnds;
    con.constraintsRep->nonlinearIneqConUpperBnds
      = constraintsRep->nonlinearIneqConUpperBnds;
    con.constraintsRep->nonlinearEqConTargets
      = constraintsRep->nonlinearEqConTargets;
    // linear constraints
    con.constraintsRep->numLinearIneqCons  = constraintsRep->numLinearIneqCons;
    con.constraintsRep->numLinearEqCons    = constraintsRep->numLinearEqCons;
    con.constraintsRep->linearIneqConCoeffs
      = constraintsRep->linearIneqConCoeffs;
    con.constraintsRep->linearEqConCoeffs  = constraintsRep->linearEqConCoeffs;
    con.constraintsRep->linearIneqConLowerBnds
      = constraintsRep->linearIneqConLowerBnds;
    con.constraintsRep->linearIneqConUpperBnds
      = constraintsRep->linearIneqConUpperBnds;
    con.constraintsRep->linearEqConTargets = constraintsRep->linearEqConTargets;
    // bounds
    con.constraintsRep->allContinuousLowerBnds
      = constraintsRep->allContinuousLowerBnds;
    con.constraintsRep->allContinuousUpperBnds
      = constraintsRep->allContinuousUpperBnds;
    con.constraintsRep->allDiscreteIntLowerBnds
      = constraintsRep->allDiscreteIntLowerBnds;
    con.constraintsRep->allDiscreteIntUpperBnds
      = constraintsRep->allDiscreteIntUpperBnds;
    con.constraintsRep->allDiscreteRealLowerBnds
      = constraintsRep->allDiscreteRealLowerBnds;
    con.constraintsRep->allDiscreteRealUpperBnds
      = constraintsRep->allDiscreteRealUpperBnds;

    // update active and inactive views
    con.constraintsRep->build_views();
  }

  return con;
}


void Constraints::
reshape(size_t num_nln_ineq_cons, size_t num_nln_eq_cons,
	size_t num_lin_ineq_cons, size_t num_lin_eq_cons,
	const SizetArray& vc_totals)
{
  if (constraintsRep) { // envelope
    constraintsRep->reshape(num_nln_ineq_cons, num_nln_eq_cons,
			    num_lin_ineq_cons, num_lin_eq_cons);
    constraintsRep->reshape(vc_totals);
  }
  else { // base class implementation for letter
    reshape(num_nln_ineq_cons, num_nln_eq_cons, num_lin_ineq_cons,
	    num_lin_eq_cons);
    reshape(vc_totals);
  }
}

/** Resizes the linear and nonlinear constraint arrays at the base
    class.  Does NOT currently resize the derived bounds arrays. */
void Constraints::
reshape(size_t num_nln_ineq_cons, size_t num_nln_eq_cons,
	size_t num_lin_ineq_cons, size_t num_lin_eq_cons)
      //const SizetArray& vc_totals)
{
  if (constraintsRep) // envelope
    constraintsRep->reshape(num_nln_ineq_cons, num_nln_eq_cons,
			    num_lin_ineq_cons, num_lin_eq_cons);
  else { // base class implementation for letter

    // Reshape attributes used by all letters
    numNonlinearIneqCons = num_nln_ineq_cons;
    nonlinearIneqConLowerBnds.resize(num_nln_ineq_cons);
    nonlinearIneqConUpperBnds.resize(num_nln_ineq_cons);

    numNonlinearEqCons = num_nln_eq_cons;
    nonlinearEqConTargets.resize(num_nln_eq_cons);

    numLinearIneqCons = num_lin_ineq_cons;
    linearIneqConLowerBnds.resize(num_lin_ineq_cons);
    linearIneqConUpperBnds.resize(num_lin_ineq_cons);
    //linearIneqConCoeffs.reshape(num_lin_ineq_cons);

    numLinearEqCons = num_lin_eq_cons;
    linearEqConTargets.resize(num_lin_eq_cons);
    //linearEqConCoeffs.reshape(num_lin_eq_cons);

    //size_t num_vars = num_cv + num_dv;
    //linearIneqConCoeffs.reshape(num_lin_ineq_cons, num_vars);
    //linearEqConCoeffs.reshape(num_lin_eq_cons, num_vars);
  }
}


/** Resizes the derived bounds arrays. */
void Constraints::reshape(const SizetArray& vc_totals)
{
  if (constraintsRep) // envelope
    constraintsRep->reshape(vc_totals);
  else { // base class portion invoked by derived class redefinitions
    // reshape base class data that requires number of active variables
    // (follows build_views() call in derived reshape() definitions)
    size_t num_av = continuousLowerBnds.length() +
      discreteIntLowerBnds.length() + discreteRealLowerBnds.length();
    linearIneqConCoeffs.reshape(numLinearIneqCons, num_av);
    linearEqConCoeffs.reshape(numLinearEqCons, num_av);
  }
}


/** Convenience function called from derived class constructors.  The
    number of variables active for applying linear constraints is
    currently defined to be the number of active continuous variables
    plus the number of active discrete variables (the most general
    case), even though very few optimizers can currently support mixed
    variable linear constraints. */
void Constraints::manage_linear_constraints(const ProblemDescDB& problem_db)
{
  const RealVector& linear_ineq_cons
    = problem_db.get_rv("method.linear_inequality_constraints");
  const RealVector& linear_eq_cons
    = problem_db.get_rv("method.linear_equality_constraints");
  size_t lin_ineq_len = linear_ineq_cons.length(),
         lin_eq_len   = linear_eq_cons.length();
  // get number of active variables to which linear constraints are applied.
  size_t num_vars = continuousLowerBnds.length() +
    discreteIntLowerBnds.length() + discreteRealLowerBnds.length();

  if (lin_ineq_len || lin_eq_len) { // check sanity of inputs
    // check on num_vars is embedded so that, if there are no linear
    // constraints, the error is managed downstream within the iterators
    if (num_vars == 0) {
      Cerr << "Error: no active variable bounds in Constraints::"
	   << "manage_linear_constraints()." << std::endl;
      abort_handler(-1);
    }
    else if (lin_ineq_len%num_vars || lin_eq_len%num_vars) {
      Cerr << "Error: number of terms in linear constraint specification not "
	   << "evenly\n       divisible by " << num_vars << " variables."
	   << std::endl;
      abort_handler(-1);
    }
  }

  if (lin_ineq_len) {
    numLinearIneqCons = lin_ineq_len/num_vars;
    copy_data(linear_ineq_cons, linearIneqConCoeffs, (int)numLinearIneqCons,
	      (int)num_vars);

    size_t i, len_lower_bnds = linearIneqConLowerBnds.length(),
      len_upper_bnds = linearIneqConUpperBnds.length();
    if (!len_lower_bnds) {
      linearIneqConLowerBnds.sizeUninitialized(numLinearIneqCons);
      linearIneqConLowerBnds = -DBL_MAX; // default lower bounds
    }
    else if (len_lower_bnds != numLinearIneqCons) {
      Cerr << "Error: length of linear inequality lower bounds specification "
           << "not equal to\n       number of linear inequality constraints."
           << std::endl;
      abort_handler(-1);
    }
    if (!len_upper_bnds) {
      linearIneqConUpperBnds.sizeUninitialized(numLinearIneqCons);
      linearIneqConUpperBnds = 0.0; // default upper bounds
    }
    else if (len_upper_bnds != numLinearIneqCons) {
      Cerr << "Error: length of linear inequality upper bounds specification "
           << "not equal to\n       number of linear inequality constraints."
           << std::endl;
      abort_handler(-1);
    }
    // Sanity check on bounds (prevents subtle specification error resulting
    // from specifying positive lower bounds and relying on the default upper
    // bounds of 0).
    for (i=0; i<numLinearIneqCons; i++) {
      if (linearIneqConLowerBnds[i] > linearIneqConUpperBnds[i]) {
	Cerr << "Error: linear inequality lower bound values must be less than "
	     << "or equal to\n       linear inequality upper bound values."
	     << std::endl;
	abort_handler(-1);
      }
    }
  }
  if (lin_eq_len) {
    numLinearEqCons = lin_eq_len/num_vars;
    copy_data(linear_eq_cons, linearEqConCoeffs, (int)numLinearEqCons,
	      (int)num_vars);

    size_t len_targets = linearEqConTargets.length();
    if (!len_targets) {
      linearEqConTargets.sizeUninitialized(numLinearEqCons);
      linearEqConTargets = 0.0;
    }
    else if (len_targets != numLinearEqCons) {
      Cerr << "Error: length of linear equality targets specification not "
           << "equal to\n       number of linear equality constraints."
	   << std::endl;
      abort_handler(-1);
    }
  }
}

} // namespace Dakota
