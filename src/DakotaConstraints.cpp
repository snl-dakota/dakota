/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
    the letter IS the representation, its rep pointer is set to NULL. */
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
    problem_db.get_rv("variables.linear_inequality_lower_bounds")),
  linearIneqConUpperBnds(
    problem_db.get_rv("variables.linear_inequality_upper_bounds")),
  linearEqConTargets(
    problem_db.get_rv("variables.linear_equality_targets"))
{
  shape_bounds(); // size all*{Lower,Upper}Bnds arrays
  build_views(); // construct active/inactive views of all arrays
  manage_linear_constraints(problem_db); // manage linear constraints
}


/** This constructor is the one which must build the base class data
    for all derived classes.  get_constraints() instantiates a derived
    class letter and the derived constructor selects this base class
    constructor in its initialization list (to avoid recursion in the
    base class constructor calling get_constraints() again).  Since
    the letter IS the representation, its rep pointer is set to NULL. */
Constraints::
Constraints(BaseConstructor, const SharedVariablesData& svd):
  sharedVarsData(svd), numNonlinearIneqCons(0), numNonlinearEqCons(0),
  numLinearIneqCons(0), numLinearEqCons(0)
{
  shape_bounds(); // size all*{Lower,Upper}Bnds arrays
  build_views(); // construct active/inactive views of all arrays
  // no linear constraints for this lightweight ctor
}


/** The default constructor: constraintsRep is NULL in this case (a
    populated problem_db is needed to build a meaningful Constraints
    object). */
Constraints::Constraints()
{ /* empty ctor */ }


/** The envelope constructor only needs to extract enough data to
    properly execute get_constraints, since the constructor
    overloaded with BaseConstructor builds the actual base class data
    inherited by the derived classes. */
Constraints::
Constraints(const ProblemDescDB& problem_db, const SharedVariablesData& svd):
  // Set the rep pointer to the appropriate variable constraints type
  constraintsRep(get_constraints(problem_db, svd))
{
  if (!constraintsRep) // bad type or insufficient memory
    abort_handler(CONS_ERROR);
}


/** Initializes constraintsRep to the appropriate derived type, as given
    by the variables view. */
std::shared_ptr<Constraints> Constraints::
get_constraints(const ProblemDescDB& problem_db, const SharedVariablesData& svd)
{
  short active_view = svd.view().first;
  switch (active_view) {
  case MIXED_ALL: case MIXED_DESIGN: case MIXED_ALEATORY_UNCERTAIN:
  case MIXED_EPISTEMIC_UNCERTAIN: case MIXED_UNCERTAIN: case MIXED_STATE:
    return std::make_shared<MixedVarConstraints>(problem_db, svd);
    break;
  case RELAXED_ALL: case RELAXED_DESIGN: case RELAXED_ALEATORY_UNCERTAIN:
  case RELAXED_EPISTEMIC_UNCERTAIN: case RELAXED_UNCERTAIN: case RELAXED_STATE:
    return std::make_shared<RelaxedVarConstraints>(problem_db, svd);
    break;
  default:
    Cerr << "Constraints active view " << active_view << " not currently "
	 << "supported in derived Constraints classes." << std::endl;
    return std::shared_ptr<Constraints>();
    break;
  }
}


/** Envelope constructor for instantiations on the fly.  This
    constructor executes get_constraints(view), which invokes the
    default derived/base constructors, followed by a reshape() based
    on vars_comps. */
Constraints::Constraints(const SharedVariablesData& svd):
  // Set the rep pointer to the appropriate variable constraints type
  constraintsRep(get_constraints(svd))
{
  if (!constraintsRep)
    abort_handler(CONS_ERROR);
}


/** Initializes constraintsRep to the appropriate derived type, as given by
    the variables view. The default derived class constructors are invoked. */
std::shared_ptr<Constraints>
Constraints::get_constraints(const SharedVariablesData& svd) const
{
  short active_view = svd.view().first;
  switch (active_view) {
  case MIXED_ALL: case MIXED_DESIGN: case MIXED_ALEATORY_UNCERTAIN:
  case MIXED_EPISTEMIC_UNCERTAIN: case MIXED_UNCERTAIN: case MIXED_STATE:
    return std::make_shared<MixedVarConstraints>(svd);
    break;
  case RELAXED_ALL: case RELAXED_DESIGN: case RELAXED_ALEATORY_UNCERTAIN:
  case RELAXED_EPISTEMIC_UNCERTAIN: case RELAXED_UNCERTAIN: case RELAXED_STATE:
    return std::make_shared<RelaxedVarConstraints>(svd);
    break;
  default:
    Cerr << "Constraints active view " << active_view << " not currently "
	 << "supported in derived Constraints classes." << std::endl;
    return std::shared_ptr<Constraints>();
    break;
  }
}


/** Copy constructor manages sharing of constraintsRep */
Constraints::Constraints(const Constraints& con)
{
  constraintsRep = con.constraintsRep;
}


/** Assignment operator shares the constraintsRep with this envelope. */
Constraints Constraints::operator=(const Constraints& con)
{
  constraintsRep = con.constraintsRep;
  return *this; // calls copy constructor since returned by value
}


Constraints::~Constraints()
{ /* empty dtor */ }


void Constraints::build_active_views()
{
  // Initialize active views
  if (sharedVarsData.view().first == EMPTY_VIEW) {
    Cerr << "Error: active view cannot be EMPTY_VIEW in VarConstraints."
	 << std::endl;
    abort_handler(CONS_ERROR);
  }
  sharedVarsData.initialize_active_start_counts();
  sharedVarsData.initialize_active_components();

  size_t num_cv  = sharedVarsData.cv(),    num_div = sharedVarsData.div(),
       /*num_dsv = sharedVarsData.dsv(),*/ num_drv = sharedVarsData.drv();
  if (num_cv) {
    size_t cv_start = sharedVarsData.cv_start();
    continuousLowerBnds = RealVector(Teuchos::View,
      &allContinuousLowerBnds[cv_start], num_cv);
    continuousUpperBnds = RealVector(Teuchos::View,
      &allContinuousUpperBnds[cv_start], num_cv);
  }
  else // return to default empty initialization
    continuousLowerBnds = continuousUpperBnds = RealVector();
  if (num_div) {
    size_t div_start = sharedVarsData.div_start();
    discreteIntLowerBnds = IntVector(Teuchos::View,
      &allDiscreteIntLowerBnds[div_start], num_div);
    discreteIntUpperBnds = IntVector(Teuchos::View,
      &allDiscreteIntUpperBnds[div_start], num_div);
  }
  else // return to default empty initialization
    discreteIntLowerBnds = discreteIntUpperBnds = IntVector();
  if (num_drv) {
    size_t drv_start = sharedVarsData.drv_start();
    discreteRealLowerBnds = RealVector(Teuchos::View,
      &allDiscreteRealLowerBnds[drv_start], num_drv);
    discreteRealUpperBnds = RealVector(Teuchos::View,
      &allDiscreteRealUpperBnds[drv_start], num_drv);
  }
  else // return to default empty initialization
    discreteRealLowerBnds = discreteRealUpperBnds = RealVector();
}


void Constraints::build_inactive_views()
{
  // Initialize inactive views
  if (sharedVarsData.view().second == MIXED_ALL ||
      sharedVarsData.view().second == RELAXED_ALL) {
    Cerr << "Error: inactive view cannot be ALL in VarConstraints."<< std::endl;
    abort_handler(CONS_ERROR);
  }
  sharedVarsData.initialize_inactive_start_counts();
  sharedVarsData.initialize_inactive_components();

  size_t num_icv  = sharedVarsData.icv(),    num_idiv = sharedVarsData.idiv(),
       /*num_idsv = sharedVarsData.idsv(),*/ num_idrv = sharedVarsData.idrv();
  if (num_icv) {
    size_t icv_start = sharedVarsData.icv_start();
    inactiveContinuousLowerBnds = RealVector(Teuchos::View,
      &allContinuousLowerBnds[icv_start], num_icv);
    inactiveContinuousUpperBnds = RealVector(Teuchos::View,
      &allContinuousUpperBnds[icv_start], num_icv);
  }
  else // return to default empty initialization
    inactiveContinuousLowerBnds = inactiveContinuousUpperBnds = RealVector();
  if (num_idiv) {
    size_t idiv_start = sharedVarsData.idiv_start();
    inactiveDiscreteIntLowerBnds = IntVector(Teuchos::View,
      &allDiscreteIntLowerBnds[idiv_start], num_idiv);
    inactiveDiscreteIntUpperBnds = IntVector(Teuchos::View,
      &allDiscreteIntUpperBnds[idiv_start], num_idiv);
  }
  else // return to default empty initialization
    inactiveDiscreteIntLowerBnds = inactiveDiscreteIntUpperBnds = IntVector();
  if (num_idrv) {
    size_t idrv_start = sharedVarsData.idrv_start();
    inactiveDiscreteRealLowerBnds = RealVector(Teuchos::View,
      &allDiscreteRealLowerBnds[idrv_start], num_idrv);
    inactiveDiscreteRealUpperBnds = RealVector(Teuchos::View,
      &allDiscreteRealUpperBnds[idrv_start], num_idrv);
  }
  else // return to default empty initialization
    inactiveDiscreteRealLowerBnds = inactiveDiscreteRealUpperBnds
      = RealVector();
}


void Constraints::active_view(short view1)
{
  if (constraintsRep)
    constraintsRep->active_view(view1);
  else { // likely redundant with corresponding update from Variables

    sharedVarsData.active_view(view1);
    // Unconditional update: account for case where Variables::active_view()
    // already updated SharedVariablesData, but Constraints has not been updated
    build_active_views();

    if (view1 == RELAXED_ALL || view1 == MIXED_ALL) {
      if (sharedVarsData.view().second)
	sharedVarsData.inactive_view(EMPTY_VIEW);
      build_inactive_views(); // unconditional
    }

    //check_view_compatibility(); // performed in Variables
  }
}


void Constraints::inactive_view(short view2)
{
  if (constraintsRep)
    constraintsRep->inactive_view(view2);
  else {
    short view1 = sharedVarsData.view().first;
    if (view2 == RELAXED_ALL || view2 == MIXED_ALL) {
      Cerr << "Error: Constraints inactive view may not be ALL." << std::endl;
      abort_handler(CONS_ERROR);
    }
    else if ( (view1 == RELAXED_ALL || view1 == MIXED_ALL) && view2) {
      Cerr << "Warning: ignoring non-EMPTY inactive view for active ALL view "
	   << "in Constraints." << std::endl;
      return; //abort_handler(CONS_ERROR);
    }

    sharedVarsData.inactive_view(view2); // likely redundant with Variables
    build_inactive_views(); // unconditional
    //check_view_compatibility(); // performed in Variables
  }
}


void Constraints::read(std::istream& s)
{
  if (constraintsRep)
    constraintsRep->read(s); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual read function.\n"
         << "No default defined at base class." << std::endl;
    abort_handler(CONS_ERROR);
  }
}


void Constraints::write(std::ostream& s) const
{
  if (constraintsRep)
    constraintsRep->write(s); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual write function.\n"
         << "No default defined at base class." << std::endl;
    abort_handler(CONS_ERROR);
  }
}


/** Deep copies are used for history mechanisms that catalogue
    permanent copies (should not change as the representation within
    userDefinedConstraints changes). */
Constraints Constraints::copy() const
{
  // the envelope class instantiates a new envelope and a new letter and copies
  // current attributes into the new objects.
  Constraints con; // new envelope: constraintsRep=NULL

  if (constraintsRep) {
    // new letter: allocate a constraintsRep
    con.constraintsRep = get_constraints(constraintsRep->sharedVarsData);

    con.update(*this);
  }

  return con;
}


void Constraints::update_nonlinear_constraints(const Constraints& cons)
{
  constraintsRep->numNonlinearIneqCons
    = cons.constraintsRep->numNonlinearIneqCons;
  constraintsRep->numNonlinearEqCons = cons.constraintsRep->numNonlinearEqCons;
  copy_data(cons.constraintsRep->nonlinearIneqConLowerBnds,
	    constraintsRep->nonlinearIneqConLowerBnds);
  copy_data(cons.constraintsRep->nonlinearIneqConUpperBnds,
	    constraintsRep->nonlinearIneqConUpperBnds);
  copy_data(cons.constraintsRep->nonlinearEqConTargets,
	    constraintsRep->nonlinearEqConTargets);
}


void Constraints::update_linear_constraints(const Constraints& cons)
{
  constraintsRep->numLinearIneqCons = cons.constraintsRep->numLinearIneqCons;
  constraintsRep->numLinearEqCons   = cons.constraintsRep->numLinearEqCons;
  copy_data(cons.constraintsRep->linearIneqConCoeffs,
	    constraintsRep->linearIneqConCoeffs);
  copy_data(cons.constraintsRep->linearEqConCoeffs,
	    constraintsRep->linearEqConCoeffs);
  copy_data(cons.constraintsRep->linearIneqConLowerBnds,
	    constraintsRep->linearIneqConLowerBnds);
  copy_data(cons.constraintsRep->linearIneqConUpperBnds,
	    constraintsRep->linearIneqConUpperBnds);
  copy_data(cons.constraintsRep->linearEqConTargets,
	    constraintsRep->linearEqConTargets);
}


void Constraints::update_variable_bounds(const Constraints& cons)
{
  copy_data(cons.constraintsRep->allContinuousLowerBnds,
	    constraintsRep->allContinuousLowerBnds);
  copy_data(cons.constraintsRep->allContinuousUpperBnds,
	    constraintsRep->allContinuousUpperBnds);
  copy_data(cons.constraintsRep->allDiscreteIntLowerBnds,
	    constraintsRep->allDiscreteIntLowerBnds);
  copy_data(cons.constraintsRep->allDiscreteIntUpperBnds,
	    constraintsRep->allDiscreteIntUpperBnds);
  copy_data(cons.constraintsRep->allDiscreteRealLowerBnds,
	    constraintsRep->allDiscreteRealLowerBnds);
  copy_data(cons.constraintsRep->allDiscreteRealUpperBnds,
	    constraintsRep->allDiscreteRealUpperBnds);
}


/** Deep copies are used for history mechanisms that catalogue
    permanent copies (should not change as the representation within
    userDefinedConstraints changes). */
void Constraints::update(const Constraints& cons)
{
  update_nonlinear_constraints(cons);
  update_linear_constraints(cons);
  update_variable_bounds(cons);

  // update active and inactive views
  constraintsRep->build_views();
}


/** Resizes the derived bounds arrays. */
void Constraints::shape_bounds()
{
  if (constraintsRep) // envelope
    constraintsRep->shape_bounds();
  else { // base class portion invoked by derived class redefinitions

    size_t num_acv, num_adiv, num_adsv, num_adrv;
    sharedVarsData.all_counts(num_acv, num_adiv, num_adsv, num_adrv);

    allContinuousLowerBnds.sizeUninitialized(num_acv);
    allContinuousUpperBnds.sizeUninitialized(num_acv);
    allDiscreteIntLowerBnds.sizeUninitialized(num_adiv);
    allDiscreteIntUpperBnds.sizeUninitialized(num_adiv);
    //allDiscreteStringLowerBnds.sizeUninitialized(num_adsv);
    //allDiscreteStringUpperBnds.sizeUninitialized(num_adsv);
    allDiscreteRealLowerBnds.sizeUninitialized(num_adrv);
    allDiscreteRealUpperBnds.sizeUninitialized(num_adrv);
  }
}


/* *** TO DO: can this version be retired? ***
void Constraints::
reshape(size_t num_nln_ineq_cons, size_t num_nln_eq_cons,
	size_t num_lin_ineq_cons, size_t num_lin_eq_cons,
	const SharedVariablesData& svd)
{
  if (constraintsRep) // envelope
    constraintsRep->reshape(num_nln_ineq_cons, num_nln_eq_cons,
			    num_lin_ineq_cons, num_lin_eq_cons, svd);
  else { // base class implementation
    sharedVarsData = svd;
    reshape_bounds(); // bounds
    build_views();    // active views of bounds
    reshape_nonlinear(num_nln_ineq_cons, num_nln_eq_cons);
    reshape_linear(num_lin_ineq_cons, num_lin_eq_cons); // simple resize, no mapping
  }
}
*/


void Constraints::
reshape(size_t num_nln_ineq_cons, size_t num_nln_eq_cons,
	const SharedVariablesData& svd)
{
  if (constraintsRep) // envelope
    constraintsRep->reshape(num_nln_ineq_cons, num_nln_eq_cons, svd);
  else { // base class implementation
    reshape_nonlinear(num_nln_ineq_cons, num_nln_eq_cons);
    reshape_update_linear(sharedVarsData, svd);

    sharedVarsData = svd;
    reshape_bounds(); // bounds
    build_views();    // active views of bounds
  }
}


void Constraints::reshape_bounds()
{
  if (constraintsRep) // envelope
    constraintsRep->reshape_bounds();
  else { // base class portion invoked by derived class redefinitions

    size_t num_acv, num_adiv, num_adsv, num_adrv;
    sharedVarsData.all_counts(num_acv, num_adiv, num_adsv, num_adrv);

    // not currently preserving data across all-variables resizes
    allContinuousLowerBnds.resize(num_acv);
    allContinuousUpperBnds.resize(num_acv);
    allDiscreteIntLowerBnds.resize(num_adiv);
    allDiscreteIntUpperBnds.resize(num_adiv);
    //allDiscreteStringLowerBnds.resize(num_adsv);
    //allDiscreteStringUpperBnds.resize(num_adsv);
    allDiscreteRealLowerBnds.resize(num_adrv);
    allDiscreteRealUpperBnds.resize(num_adrv);
  }
}


/** Resizes the linear and nonlinear constraint arrays at the base
    class.  Does NOT currently resize the derived bounds arrays. */
void Constraints::
reshape_nonlinear(size_t num_nln_ineq_cons, size_t num_nln_eq_cons)
{
  if (constraintsRep) // envelope
    constraintsRep->reshape_nonlinear(num_nln_ineq_cons, num_nln_eq_cons);
  else { // reshape nln con attributes used by all letters
    if (numNonlinearIneqCons != num_nln_ineq_cons) {
      numNonlinearIneqCons = num_nln_ineq_cons;
      nonlinearIneqConLowerBnds.resize(num_nln_ineq_cons);
      nonlinearIneqConUpperBnds.resize(num_nln_ineq_cons);
    }
    if (numNonlinearEqCons != num_nln_eq_cons) {
      numNonlinearEqCons = num_nln_eq_cons;
      nonlinearEqConTargets.resize(num_nln_eq_cons);
    }
  }
}


/** Resizes the linear and nonlinear constraint arrays at the base
    class.  Does NOT currently resize the derived bounds arrays. */
void Constraints::
reshape_linear(size_t num_lin_ineq_cons, size_t num_lin_eq_cons)
{
  if (constraintsRep) // envelope
    constraintsRep->reshape_linear(num_lin_ineq_cons, num_lin_eq_cons);
  else { // reshape # of linear constraints while preserving # of active vars
    if (numLinearIneqCons != num_lin_ineq_cons) {
      size_t num_av = linearIneqConCoeffs.numCols();
      if (num_av == 0)
	num_av = continuousLowerBnds.length() + discreteIntLowerBnds.length()
	       + discreteRealLowerBnds.length();
      numLinearIneqCons = num_lin_ineq_cons;
      linearIneqConLowerBnds.resize(num_lin_ineq_cons);
      linearIneqConUpperBnds.resize(num_lin_ineq_cons);
      linearIneqConCoeffs.reshape(num_lin_ineq_cons, num_av);
    }
    if (numLinearEqCons != num_lin_eq_cons) {
      size_t num_av = linearEqConCoeffs.numCols();
      if (num_av == 0)
	num_av = continuousLowerBnds.length() + discreteIntLowerBnds.length()
	       + discreteRealLowerBnds.length();
      numLinearEqCons = num_lin_eq_cons;
      linearEqConTargets.resize(num_lin_eq_cons);
      linearEqConCoeffs.reshape(num_lin_eq_cons, num_av);
    }
  }
}


/** Resizes the linear coefficient arrays based on change in active
    variables view. */
void Constraints::
reshape_update_linear(const SharedVariablesData& prev_svd,
		      const SharedVariablesData& curr_svd)
{
  if (constraintsRep) // envelope
    constraintsRep->reshape_update_linear(prev_svd, curr_svd);
  else { // base class implementation for letter

    short  active_view = curr_svd.view().first,
      prev_active_view = prev_svd.view().first;
    if ( active_view == prev_active_view ||
	 ( numLinearIneqCons == 0 && numLinearEqCons == 0 ) )
      return;

    // Preserve previous linear constraint data 
    size_t num_cv = curr_svd.cv(), num_div = curr_svd.div(),
      num_drv = curr_svd.drv(), num_av = num_cv + num_div + num_drv,
      curr_cntr, prev_cntr, col;
    RealMatrix prev_lin_ineq_coeffs = linearIneqConCoeffs,
	       prev_lin_eq_coeffs   = linearEqConCoeffs;
    linearIneqConCoeffs.shape(numLinearIneqCons, num_av); // init to 0
    linearEqConCoeffs.shape(numLinearEqCons,     num_av); // init to 0
    if ( ( prev_active_view == RELAXED_ALL || prev_active_view == MIXED_ALL ) &&
	 active_view >= RELAXED_DESIGN) { // contract (ensure omission of zeros)
      curr_cntr = 0;  prev_cntr = curr_svd.cv_start();
      for (col=0; col<prev_cntr; ++col) // ensure no loss of information
	check_zeros(prev_lin_ineq_coeffs[col], numLinearIneqCons);
      if (numLinearIneqCons)
	for (col=0; col<num_cv; ++col, ++curr_cntr, ++prev_cntr)
	  copy_data(prev_lin_ineq_coeffs[prev_cntr],
		    linearIneqConCoeffs[curr_cntr], numLinearIneqCons);
      if (numLinearEqCons)
	for (col=0; col<num_cv; ++col, ++curr_cntr, ++prev_cntr)
	  copy_data(prev_lin_eq_coeffs[prev_cntr], linearEqConCoeffs[curr_cntr],
		    numLinearEqCons);
      col = prev_cntr;  prev_cntr += curr_svd.div_start();
      for (; col<prev_cntr; ++col) // ensure no loss of information
	check_zeros(prev_lin_ineq_coeffs[col], numLinearIneqCons);
      for (col=0; col<num_div; ++col, ++curr_cntr, ++prev_cntr) {
	copy_data(prev_lin_ineq_coeffs[prev_cntr],
		  linearIneqConCoeffs[curr_cntr], numLinearIneqCons);
	copy_data(prev_lin_eq_coeffs[prev_cntr],  linearEqConCoeffs[curr_cntr],
		  numLinearEqCons);
      }
      col = prev_cntr;  prev_cntr += curr_svd.drv_start();
      for (; col<prev_cntr; ++col) // ensure no loss of information
	check_zeros(prev_lin_ineq_coeffs[col], numLinearIneqCons);
      for (col=0; col<num_drv; ++col, ++curr_cntr, ++prev_cntr) {
	copy_data(prev_lin_ineq_coeffs[prev_cntr],
		  linearIneqConCoeffs[curr_cntr], numLinearIneqCons);
	copy_data(prev_lin_eq_coeffs[prev_cntr],  linearEqConCoeffs[curr_cntr],
		  numLinearEqCons);
      }
    }
    else if ( ( active_view == RELAXED_ALL || active_view == MIXED_ALL ) &&
	      prev_active_view >= RELAXED_DESIGN) { // inflate (pad with zeros)
      prev_cntr = 0;  curr_cntr = prev_svd.cv_start();
      for (col=0; col<num_cv; ++col, ++curr_cntr, ++prev_cntr) {
	copy_data(prev_lin_ineq_coeffs[prev_cntr],
		  linearIneqConCoeffs[curr_cntr], numLinearIneqCons);
	copy_data(prev_lin_eq_coeffs[prev_cntr],  linearEqConCoeffs[curr_cntr],
		  numLinearEqCons);
      }
      curr_cntr += prev_svd.div_start();
      for (col=0; col<num_div; ++col, ++curr_cntr, ++prev_cntr) {
	copy_data(prev_lin_ineq_coeffs[prev_cntr],
		  linearIneqConCoeffs[curr_cntr], numLinearIneqCons);
	copy_data(prev_lin_eq_coeffs[prev_cntr],  linearEqConCoeffs[curr_cntr],
		  numLinearEqCons);
      }
      curr_cntr += prev_svd.drv_start();
      for (col=0; col<num_drv; ++col, ++curr_cntr, ++prev_cntr) {
	copy_data(prev_lin_ineq_coeffs[prev_cntr],
		  linearIneqConCoeffs[curr_cntr], numLinearIneqCons);
	copy_data(prev_lin_eq_coeffs[prev_cntr],  linearEqConCoeffs[curr_cntr],
		  numLinearEqCons);
      }
    }
    else {
      Cerr << "Error: unsupported view combination in Constraints::"
	   << "reshape_update_linear()" << std::endl;
      abort_handler(CONS_ERROR);
    }
  }
}


/** Convenience function called from derived class constructors.  The number of
    variables active for applying linear constraints is currently defined to be
    the number of active continuous variables plus the number of active discrete
    {int,real} variables (most general case), even though very few optimizers
    can currently support mixed variable linear constraints. */
void Constraints::manage_linear_constraints(const ProblemDescDB& problem_db)
{
  const RealVector& linear_ineq_cons
    = problem_db.get_rv("variables.linear_inequality_constraints");
  const RealVector& linear_eq_cons
    = problem_db.get_rv("variables.linear_equality_constraints");
  size_t lin_ineq_len = linear_ineq_cons.length(),
         lin_eq_len   = linear_eq_cons.length();
  // get number of active variables to which linear constraints are applied.
  size_t num_vars = sharedVarsData.cv() + sharedVarsData.div()
                  + sharedVarsData.drv();

  if (lin_ineq_len || lin_eq_len) { // check sanity of inputs
    // check on num_vars is embedded so that, if there are no linear
    // constraints, the error is managed downstream within the iterators
    if (num_vars == 0) {
      Cerr << "Error: no active variables in Constraints::"
	   << "manage_linear_constraints()." << std::endl;
      abort_handler(CONS_ERROR);
    }
    else if (lin_ineq_len%num_vars || lin_eq_len%num_vars) {
      Cerr << "Error: number of terms in linear constraint specification not "
	   << "evenly\n       divisible by " << num_vars << " variables."
	   << std::endl;
      abort_handler(CONS_ERROR);
    }
  }

  if (lin_ineq_len) {
    numLinearIneqCons = lin_ineq_len/num_vars;
    copy_data(linear_ineq_cons, linearIneqConCoeffs, (int)numLinearIneqCons,
	      (int)num_vars);

    // linear ineq bounds already assigned in ctor initialization
    size_t i, len_lower_bnds = linearIneqConLowerBnds.length(),
              len_upper_bnds = linearIneqConUpperBnds.length();
    if (!len_lower_bnds) {
      linearIneqConLowerBnds.sizeUninitialized(numLinearIneqCons);
      linearIneqConLowerBnds
	= -std::numeric_limits<Real>::infinity(); // default lower bounds
    }
    else if (len_lower_bnds != numLinearIneqCons) {
      Cerr << "Error: length of linear inequality lower bounds specification "
           << "not equal to\n       number of linear inequality constraints."
           << std::endl;
      abort_handler(CONS_ERROR);
    }
    if (!len_upper_bnds) {
      linearIneqConUpperBnds.sizeUninitialized(numLinearIneqCons);
      linearIneqConUpperBnds = 0.0; // default upper bounds
    }
    else if (len_upper_bnds != numLinearIneqCons) {
      Cerr << "Error: length of linear inequality upper bounds specification "
           << "not equal to\n       number of linear inequality constraints."
           << std::endl;
      abort_handler(CONS_ERROR);
    }
    // Sanity check on bounds (prevents subtle specification error resulting
    // from specifying positive lower bounds and relying on the default upper
    // bounds of 0).
    for (i=0; i<numLinearIneqCons; i++) {
      if (linearIneqConLowerBnds[i] > linearIneqConUpperBnds[i]) {
	Cerr << "Error: linear inequality lower bound values must be less than "
	     << "or equal to\n       linear inequality upper bound values."
	     << std::endl;
	abort_handler(CONS_ERROR);
      }
    }
  }
  if (lin_eq_len) {
    numLinearEqCons = lin_eq_len/num_vars;
    copy_data(linear_eq_cons, linearEqConCoeffs, (int)numLinearEqCons,
	      (int)num_vars);

    // linear eq targets already assigned in ctor initialization
    size_t len_targets = linearEqConTargets.length();
    if (!len_targets) {
      linearEqConTargets.sizeUninitialized(numLinearEqCons);
      linearEqConTargets = 0.0;
    }
    else if (len_targets != numLinearEqCons) {
      Cerr << "Error: length of linear equality targets specification not "
           << "equal to\n       number of linear equality constraints."
	   << std::endl;
      abort_handler(CONS_ERROR);
    }
  }
}

} // namespace Dakota
