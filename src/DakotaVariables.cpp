/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Variables
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"
#include "RelaxedVariables.hpp"
#include "MixedVariables.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"
#include "dakota_system_defs.hpp"
#include "boost/functional/hash/hash.hpp"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/utility.hpp>  // for std::pair
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/array.hpp>

static const char rcsId[]="@(#) $Id: DakotaVariables.cpp 7037 2010-10-23 01:18:08Z mseldre $";

BOOST_CLASS_EXPORT(Dakota::Variables)

namespace Dakota {


/** This constructor is the one which must build the base class data for all
    derived classes.  get_variables() instantiates a derived class letter
    and the derived constructor selects this base class constructor in its 
    initialization list (to avoid the recursion of the base class constructor
    calling get_variables() again).  Since the letter IS the representation, 
    its representation pointer is set to NULL. */
Variables::
Variables(BaseConstructor, const ProblemDescDB& problem_db,
	  const ShortShortPair& view):
  sharedVarsData(problem_db, view)
{
  shape(); // size all*Vars arrays
  build_views(); // construct active/inactive views of all arrays
}


/** This constructor is the one which must build the base class data for all
    derived classes.  get_variables() instantiates a derived class letter
    and the derived constructor selects this base class constructor in its 
    initialization list (to avoid the recursion of the base class constructor
    calling get_variables() again).  Since the letter IS the representation, 
    its representation pointer is set to NULL. */
Variables::
Variables(BaseConstructor, const SharedVariablesData& svd):
  sharedVarsData(svd)
{
  shape(); // size all*Vars arrays
  build_views(); // construct active/inactive views of all arrays
}


/** The default constructor: variablesRep is NULL in this case (a populated
    problem_db is needed to build a meaningful Variables object). */
Variables::Variables()
{ /* empty ctor */ }


/** This is the primary envelope constructor which uses problem_db to
    build a fully populated variables object.  It only needs to
    extract enough data to properly execute get_variables(problem_db),
    since the constructor overloaded with BaseConstructor builds the
    actual base class data inherited by the derived classes. */
Variables::Variables(const ProblemDescDB& problem_db):
  // Set the rep pointer to the appropriate derived variables class
  variablesRep(get_variables(problem_db))
{
  if (!variablesRep) // bad type or insufficient memory
    abort_handler(-1);
}


/** Initializes variablesRep to the appropriate derived type, as given
    by problem_db attributes.  The standard derived class constructors
    are invoked.  */
std::shared_ptr<Variables>
Variables::get_variables(const ProblemDescDB& problem_db)
{
  ShortShortPair view = get_view(problem_db);

  // This get_variables version invokes the standard constructor.
  short active_view = view.first;
  switch (active_view) {
  case MIXED_ALL: case MIXED_DESIGN: case MIXED_ALEATORY_UNCERTAIN:
  case MIXED_EPISTEMIC_UNCERTAIN: case MIXED_UNCERTAIN: case MIXED_STATE:
    return std::make_shared<MixedVariables>(problem_db, view); break;
  case RELAXED_ALL: case RELAXED_DESIGN: case RELAXED_ALEATORY_UNCERTAIN:
  case RELAXED_EPISTEMIC_UNCERTAIN: case RELAXED_UNCERTAIN: case RELAXED_STATE:
    return std::make_shared<RelaxedVariables>(problem_db, view); break;
  default:
    Cerr << "Variables active view " << active_view << " not currently "
	 << "supported in derived Variables classes." << std::endl;
    return std::shared_ptr<Variables>(); break;
  }
}


/** This is the alternate envelope constructor for instantiations on
    the fly.  This constructor executes get_variables(view), which
    invokes the default derived/base constructors, followed by a
    resize() based on vars_comps. */
Variables::Variables(const SharedVariablesData& svd):
  // for variablesRep, instantiate the appropriate derived variables class
  variablesRep(get_variables(svd))
{
  if (!variablesRep) // bad type or insufficient memory
    abort_handler(-1);
}


/** Initializes variablesRep to the appropriate derived type, as given
    by view.  The default derived class constructors are invoked. */
std::shared_ptr<Variables>
Variables::get_variables(const SharedVariablesData& svd) const
{
  short active_view = svd.view().first;
  switch (active_view) {
  case MIXED_ALL: case MIXED_DESIGN: case MIXED_ALEATORY_UNCERTAIN:
  case MIXED_EPISTEMIC_UNCERTAIN: case MIXED_UNCERTAIN: case MIXED_STATE:
    return std::make_shared<MixedVariables>(svd); break;
  case RELAXED_ALL: case RELAXED_DESIGN: case RELAXED_ALEATORY_UNCERTAIN:
  case RELAXED_EPISTEMIC_UNCERTAIN: case RELAXED_UNCERTAIN: case RELAXED_STATE:
    return std::make_shared<RelaxedVariables>(svd); break;
  default:
    Cerr << "Variables active view " << active_view << " not currently "
	 << "supported in derived Variables classes." << std::endl;
    return std::shared_ptr<Variables>(); break;
  }
}


ShortShortPair Variables::get_view(const ProblemDescDB& problem_db) const
{
  // Manage variable views (all, design, uncertain, aleatory, epistemic,
  // or state) and continuous-discrete domain types (mixed or relaxed).
  // The former are accomplished with Teuchos/Boost views, whereas the
  // latter utilize subclassing (they are not views).

  // Since this is used by the envelope, don't set variablesView
  ShortShortPair view; // view.first = active, view.second = inactive

  // View logic has 3 levels, listed in order of precedence:
  // 1. Explicit view selection in variables specification
  // 2. Peek at responses spec, and infer design view from optimization or
  //    calibration response data types
  // 3. For generic response data, peek at method spec and infer view

  // Domain logic has 2 levels, listed in order of precedence:
  // 1. Explicit mixed/relaxed domain selection in variables specification
  // 2. Peek at method spec and infer mixed/relaxed domain

  // ---------------------------------------------------------------
  // ACTIVE VIEW: ALL, DESIGN, UNCERTAIN, ALEATORY, EPISTEMIC, STATE
  // ---------------------------------------------------------------
  short view_spec = problem_db.get_short("variables.view"),
      domain_spec = problem_db.get_short("variables.domain");
  bool relaxed = ( domain_spec   == RELAXED_DOMAIN ||      // level 1 domain
		   ( domain_spec == DEFAULT_DOMAIN &&      // level 2 domain
		     method_domain(problem_db) == RELAXED_DOMAIN ) );
  switch (view_spec) {
  case DEFAULT_VIEW: // no view selection in variables spec -> level 2,3 view
    if (response_view(problem_db) == DESIGN_VIEW) // precedence level 2 view
      view.first = (relaxed) ? RELAXED_DESIGN : MIXED_DESIGN;
    else                                          // precedence level 3 view
      view.first = method_map(method_view(problem_db), relaxed);
    break;
  default: // view selection: ALL, DESIGN, UNCERTAIN, and STATE views
    view.first = method_map(view_spec, relaxed);
  }

  view.second = EMPTY_VIEW; // inactive views only set by NestedModel
#ifdef DEBUG
  Cout << "Variables view: active_view = " << view.first
       << " inactive_view = " << view.second << std::endl;
#endif // DEBUG
  return view;
}


/** Copy constructor manages sharing of variablesRep. */
Variables::Variables(const Variables& vars):
  variablesRep(vars.variablesRep)
{ /* empty ctor */ }


Variables Variables::operator=(const Variables& vars)
{
  variablesRep = vars.variablesRep;
  return *this; // calls copy constructor since returned by value
}


Variables::~Variables()
{ /* empty dtor */ }


void Variables::active_view(short view1)
{
  if (variablesRep)
    variablesRep->active_view(view1);
  else {

    sharedVarsData.active_view(view1);
    build_active_views(); // unconditional: avoid Vars/Cons order dependence

    if (view1 == RELAXED_ALL || view1 == MIXED_ALL) {
      if (sharedVarsData.view().second)
	sharedVarsData.inactive_view(EMPTY_VIEW);
      build_inactive_views(); // unconditional: avoid Vars/Cons order dependence
    }

    // View assigment checks are performed here (rather than in the
    // NestedModel ctor or within the Model recursion) so that all
    // levels of recursion are adequately addressed.
    check_view_compatibility();
  }
}


void Variables::inactive_view(short view2)
{
  if (variablesRep)
    variablesRep->inactive_view(view2);
  else {
    short view1 = sharedVarsData.view().first;
    if (view2 == RELAXED_ALL || view2 == MIXED_ALL) {
      Cerr << "Error: Variables inactive view may not be ALL." << std::endl;
      abort_handler(VARS_ERROR);
    }
    else if ( (view1 == RELAXED_ALL || view1 == MIXED_ALL) && view2) {
      Cerr << "Warning: ignoring non-EMPTY inactive view for active ALL view "
	   << "in Variables." << std::endl;
      return; //abort_handler(VARS_ERROR);
    }

    sharedVarsData.inactive_view(view2);
    build_inactive_views(); // unconditional: avoid Vars/Cons order dependence
    // View assigment checks are performed here (rather than in the
    // NestedModel ctor or within the Model recursion) so that all
    // levels of recursion are adequately addressed.
    check_view_compatibility();
  }
}


/** Aggregate view and domain settings. */
short Variables::method_map(short view_spec, bool relaxed) const
{
  switch (view_spec) {
  case ALL_VIEW: // precedence level 1 view selection
    return (relaxed) ? RELAXED_ALL : MIXED_ALL;             break;
  case DESIGN_VIEW: // precedence level 1 view selection
    return (relaxed) ? RELAXED_DESIGN : MIXED_DESIGN;       break;
  case UNCERTAIN_VIEW: // precedence level 1 view selection
    return (relaxed) ? RELAXED_UNCERTAIN : MIXED_UNCERTAIN; break;
  case ALEATORY_UNCERTAIN_VIEW: // precedence level 1 view selection
    return (relaxed) ? RELAXED_ALEATORY_UNCERTAIN  : MIXED_ALEATORY_UNCERTAIN;
    break;
  case EPISTEMIC_UNCERTAIN_VIEW: // precedence level 1 view selection
    return (relaxed) ? RELAXED_EPISTEMIC_UNCERTAIN : MIXED_EPISTEMIC_UNCERTAIN;
    break;
  case STATE_VIEW: // precedence level 1 view selection
    return (relaxed) ? RELAXED_STATE : MIXED_STATE;         break;
  default:
    Cerr << "Error: unsupported view selection in Variables::method_map()"
	 << std::endl;
    abort_handler(-1);
    return EMPTY_VIEW;
    break;
  }
}


short Variables::method_domain(const ProblemDescDB& problem_db) const
{
  // B&B employs relaxation, all other employ mixed continuous-discrete
  return (problem_db.get_ushort("method.algorithm") == BRANCH_AND_BOUND) ?
    RELAXED_DOMAIN : MIXED_DOMAIN;
}


short Variables::method_view(const ProblemDescDB& problem_db) const
{
  // last resort: active view if no user spec and no responses inference

  unsigned short method_name = problem_db.get_ushort("method.algorithm");
  if (method_name & PSTUDYDACE_BIT)
    return ALL_VIEW;
  else if (method_name & NOND_BIT) {
    // NonD method enum vals are ordered as aleatory, both (sampling), epistemic
    if (method_name == RANDOM_SAMPLING || method_name == LIST_SAMPLING ||
	method_name == MULTILEVEL_SAMPLING) { // MC/LHS, MLMC, import samples
      size_t num_auv = problem_db.get_sizet("variables.aleatory_uncertain"),
	     num_euv = problem_db.get_sizet("variables.epistemic_uncertain");
      if (num_auv && num_euv) return UNCERTAIN_VIEW;
      else if (num_euv)       return EPISTEMIC_UNCERTAIN_VIEW;
      else if (num_auv)       return ALEATORY_UNCERTAIN_VIEW;
      else {
	Cerr << "Error: uncertain variables required for sampling method in "
	     << "Variables::get_view()." << std::endl;
	abort_handler(-1);    return DEFAULT_VIEW;
      }
    }
    else if (method_name > RANDOM_SAMPLING) return EPISTEMIC_UNCERTAIN_VIEW;
    else                                    return ALEATORY_UNCERTAIN_VIEW;
  }
  else if (method_name & VERIF_BIT) return STATE_VIEW;
  else                              return DESIGN_VIEW;
}


short Variables::response_view(const ProblemDescDB& problem_db) const
{
  // if optimization or calibration response set, infer an active design view
  // for all methods excepting members of NonDCalibration branch.  Excluding
  // all NOND_BIT methods complicates DACE with nond_sampling, so this is
  // avoided for now.  Another alternative would be to make RANDOM_SAMPLING
  // the special case (potentally only for submethods LHS and RANDOM).
  return ( problem_db.get_sizet("responses.num_objective_functions") ||
	   ( problem_db.get_sizet("responses.num_calibration_terms") && 
	// ( problem_db.get_ushort("method.algorithm") & NOND_BIT ) == 0) ) ?
	     problem_db.get_ushort("method.algorithm") != BAYES_CALIBRATION ) )
    ? DESIGN_VIEW : DEFAULT_VIEW;
}


void Variables::check_view_compatibility()
{
  const ShortShortPair& view = sharedVarsData.view();

  short active_view = view.first, inactive_view = view.second;
#ifdef DEBUG
  Cout << "Variables view update: active_view = " << active_view
       << " inactive_view = " << inactive_view << std::endl;
#endif // DEBUG

  // check assigned active and inactive views for compatibility
  bool error_flag = false;

  // don't allow any combination of RELAXED and MIXED
  if ( ( ( active_view == RELAXED_ALL ||
	   ( active_view >= RELAXED_DESIGN &&
	     active_view <= RELAXED_STATE ) ) &&
	 ( inactive_view == MIXED_ALL ||
	   ( inactive_view >= MIXED_DESIGN &&
	     inactive_view <= MIXED_STATE ) ) ) ||
       ( ( active_view == MIXED_ALL ||
	   ( active_view >= MIXED_DESIGN && active_view <= MIXED_STATE ) ) &&
	 ( inactive_view == RELAXED_ALL ||
	   ( inactive_view >= RELAXED_DESIGN &&
	     inactive_view <= RELAXED_STATE ) ) ) ) {
    Cerr << "Error: subModel active and inactive views are inconsistent in "
	 << "RELAXED/MIXED definition in Variables::check_view_compatibility()."
	 << std::endl;
    error_flag = true;
  }

  // don't overlap any types as both active and inactive
  if ( active_view == inactive_view ||
       // don't overlap ALL and DISTINCT
       ( ( active_view == RELAXED_ALL || active_view == MIXED_ALL ) &&
	 inactive_view >= RELAXED_DESIGN && inactive_view <= MIXED_STATE ) ||
       ( active_view >= RELAXED_DESIGN && active_view <= MIXED_STATE &&
	 ( inactive_view == RELAXED_ALL || inactive_view == MIXED_ALL ) ) ||
       // don't overlap aggregated and separated UNCERTAIN
       ( ( active_view   == RELAXED_UNCERTAIN ||
	   active_view   == MIXED_UNCERTAIN ) &&
	 ( inactive_view == RELAXED_ALEATORY_UNCERTAIN  ||
	   inactive_view == RELAXED_EPISTEMIC_UNCERTAIN ||
	   inactive_view == MIXED_ALEATORY_UNCERTAIN   ||
	   inactive_view == MIXED_EPISTEMIC_UNCERTAIN ) ) ||
       ( ( active_view   == RELAXED_ALEATORY_UNCERTAIN  ||
	   active_view   == RELAXED_EPISTEMIC_UNCERTAIN ||
	   active_view   == MIXED_ALEATORY_UNCERTAIN   ||
	   active_view   == MIXED_EPISTEMIC_UNCERTAIN ) &&
	 ( inactive_view == RELAXED_UNCERTAIN ||
	   inactive_view == MIXED_UNCERTAIN ) ) ) {
    Cerr << "Error: subModel active and inactive views must not overlap in "
	 << "Variables::check_view_compatibility()." << std::endl;
    error_flag = true;
  }

  if (error_flag)
    abort_handler(-1);
}


void Variables::build_active_views()
{
  // called only from letters

  // Initialize active view vectors and counts
  if (sharedVarsData.view().first == EMPTY_VIEW) {
    Cerr << "Error: active view cannot be EMPTY_VIEW in Variables::"
	 << "build_active_views()." << std::endl;
    abort_handler(-1);
  }
  sharedVarsData.initialize_active_start_counts();
  sharedVarsData.initialize_active_components();

  size_t num_cv  = sharedVarsData.cv(),    num_div = sharedVarsData.div(),
       /*num_dsv = sharedVarsData.dsv(),*/ num_drv = sharedVarsData.drv();
  continuousVars = (num_cv) ?
    RealVector(Teuchos::View, &allContinuousVars[sharedVarsData.cv_start()],
	       num_cv) :
    RealVector(); // return to default empty initialization
  discreteIntVars = (num_div) ?
    IntVector(Teuchos::View, &allDiscreteIntVars[sharedVarsData.div_start()],
	      num_div) :
    IntVector(); // return to default empty initialization
  // as for label arrays, generate StringMultiArrayViews on the fly
  //if (num_dsv) {
  //  size_t dsv_start = sharedVarsData.dsv_start();
  //  discreteStringVars = allDiscreteStringVars[boost::indices[
  //    idx_range(dsv_start, dsv_start+num_dsv)]];
  //}
  discreteRealVars = (num_drv) ?
    RealVector(Teuchos::View, &allDiscreteRealVars[sharedVarsData.drv_start()],
	       num_drv) :
    RealVector(); // return to default empty initialization
}


void Variables::build_inactive_views()
{
  // called only from letters

  // Initialize inactive view vectors and counts
  if (sharedVarsData.view().second == MIXED_ALL ||
      sharedVarsData.view().second == RELAXED_ALL) {
    Cerr << "Error: inactive view cannot be ALL in Variables::"
	 << "build_inactive_views()." << std::endl;
    abort_handler(-1);
  }
  sharedVarsData.initialize_inactive_start_counts();
  sharedVarsData.initialize_inactive_components();

  size_t num_icv  = sharedVarsData.icv(),    num_idiv = sharedVarsData.idiv(),
       /*num_idsv = sharedVarsData.idsv(),*/ num_idrv = sharedVarsData.idrv();
  inactiveContinuousVars = (num_icv) ?
    RealVector(Teuchos::View, &allContinuousVars[sharedVarsData.icv_start()],
	       num_icv) :
    RealVector(); // return to default empty initialization
  inactiveDiscreteIntVars = (num_idiv) ?
    IntVector(Teuchos::View, &allDiscreteIntVars[sharedVarsData.idiv_start()],
	      num_idiv) :
    IntVector(); // return to default empty initialization
  // as for label arrays, generate StringMultiArrayViews on the fly
  //if (num_idsv) {
  //  size_t idsv_start = sharedVarsData.idsv_start();
  //  inactiveDiscreteStringVars = allDiscreteStringVars[boost::indices[
  //    idx_range(idsv_start, idsv_start+num_idsv)]];
  //}
  inactiveDiscreteRealVars = (num_idrv) ?
    RealVector(Teuchos::View, &allDiscreteRealVars[sharedVarsData.idrv_start()],
	       num_idrv) :
    RealVector(); // return to default empty initialization
}


template<class Archive> 
void Variables::load(Archive& ar, const unsigned int version)
{
  // Binary version.

  // Can't read into the rep's SVD because there might not be a rep
  SharedVariablesData svd;
  ar & svd;

  if (variablesRep) { // should not occur in current usage
    // BMA: Leaving this conditional due to related problems in Response
    if (sharedVarsData.view() != svd.view()) {
      Cerr << "Warning: variables type mismatch in Variables::load(Archive&)."
	   << std::endl;
      variablesRep = get_variables(svd);
    }
  }
  else // read from restart: variablesRep must be instantiated
    variablesRep = get_variables(svd);

  // This code block would normally be the default implementation
  // (without variablesRep forwards), but we must support creation
  // of new letters above.
  ar & variablesRep->allContinuousVars;
  ar & variablesRep->allDiscreteIntVars;
  ar & variablesRep->allDiscreteStringVars;
  ar & variablesRep->allDiscreteRealVars;

  // rebuild active/inactive views
  variablesRep->build_views();
  // types/ids not required
}


template<class Archive> 
void Variables::save(Archive& ar, const unsigned int version) const
{
  // Binary version.
  if (variablesRep)
    variablesRep->save(ar, version); // envelope fwd to letter
  else { // default implementation for letters
    ar & sharedVarsData;

    ar & allContinuousVars;
    ar & allDiscreteIntVars;
    ar & allDiscreteStringVars;
    ar & allDiscreteRealVars;
    // types/ids not required
  }
}


// These shouldn't be necessary, but using to avoid static linking
// issues until can find the right Boost macro ordering
template void Variables:: 
load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive& ar, 
				      const unsigned int version); 
template void Variables:: 
save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive& ar, 
				      const unsigned int version) const; 


void Variables::read(std::istream& s)
{
  if (variablesRep)
    variablesRep->read(s); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual read function.\n"
         << "No default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Variables::write(std::ostream& s, unsigned short vars_part) const
{
  if (variablesRep)
    variablesRep->write(s, vars_part); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual write function.\n"
	 << "No default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Variables::write_aprepro(std::ostream& s) const
{
  if (variablesRep)
    variablesRep->write_aprepro(s); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual write_aprepro "
	 << "function.\nNo default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Variables::read_annotated(std::istream& s)
{
  // ASCII version for neutral file I/O.
  ShortShortPair view;
  s >> view.first;
  // exception handling since EOF may not be captured properly; shouldn't get 
  // hit if loops wrapping this read whitespace before reading Variables...
  if (s.eof())
    throw FileReadException("Empty record in Variables::read_annotated()");
  s >> view.second;
  size_t i;
  SizetArray vars_comps_totals(NUM_VC_TOTALS);
  for (i=0; i<NUM_VC_TOTALS; ++i)
    s >> vars_comps_totals[i];
  // for annotated (neutral), read the binary representation (string of 0/1)

  // BMA TODO: verify whether these need to be sized (shouldn't have to be)
  // size_t num_adiv
  //   = vars_comps_totals[TOTAL_DDIV]  + vars_comps_totals[TOTAL_DAUIV]
  //   + vars_comps_totals[TOTAL_DEUIV] + vars_comps_totals[TOTAL_DSIV],
  // num_adrv
  //   = vars_comps_totals[TOTAL_DDRV]  + vars_comps_totals[TOTAL_DAURV]
  //   + vars_comps_totals[TOTAL_DEURV] + vars_comps_totals[TOTAL_DSRV];
  // BitArray all_relax_di(num_adiv), all_relax_dr(num_adrv);
  BitArray all_relax_di, all_relax_dr;

  // the bit arrays can be length 0; have to conditionally read their data
  size_t ardi_size, ardr_size;
  s >> ardi_size;
  if (ardi_size > 0)
    s >> all_relax_di;
  s >> ardr_size;
  if (ardr_size > 0)
    s >> all_relax_dr;

  SharedVariablesData svd(view, vars_comps_totals, all_relax_di, all_relax_dr);

  if (variablesRep) { // should not occur in current usage
    // BMA: Leaving this conditional due to related problems in Response
    if (sharedVarsData.view() != view) {
      Cerr << "Warning: variables type mismatch in Variables::read(istream&)."
	   << std::endl;
      variablesRep = get_variables(svd);
    }
  }
  else // read from neutral file: variablesRep must be instantiated
    variablesRep = get_variables(svd);

  // This code block would normally be the default implementation (without
  // variablesRep forwards), but we must support creation of new letters above.
  read_data_annotated(s, variablesRep->allContinuousVars,
		      all_continuous_variable_labels());
  read_data_annotated(s, variablesRep->allDiscreteIntVars,
		      all_discrete_int_variable_labels());
  read_data_annotated(s, variablesRep->allDiscreteStringVars,
		      all_discrete_string_variable_labels());
  read_data_annotated(s, variablesRep->allDiscreteRealVars,
		      all_discrete_real_variable_labels());
  // rebuild active/inactive views
  variablesRep->build_views();
  // types/ids not required
}


void Variables::write_annotated(std::ostream& s) const
{
  // ASCII version for neutral file I/O.
  if (variablesRep)
    variablesRep->write_annotated(s); // envelope fwd to letter
  else { // default implementation for letters
    const ShortShortPair& view = sharedVarsData.view();
    const SizetArray& vc_totals = sharedVarsData.components_totals();
    const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
    const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();
    s << view.first  << ' ' << view.second << ' ';
    size_t i;
    for (i=0; i<NUM_VC_TOTALS; ++i)
      s << vc_totals[i] << ' ';
    // for annotated (neutral), write the binary representation (string of 0/1)
    // have to write the size as a flag since they may be length 0 in the mixed case
    s << all_relax_di.size() << ' ' << all_relax_di << ' ';
    s << all_relax_dr.size() << ' ' << all_relax_dr << ' ';
    write_data_annotated(s, allContinuousVars,
			 all_continuous_variable_labels());
    write_data_annotated(s, allDiscreteIntVars,
			 all_discrete_int_variable_labels());
    write_data_annotated(s, allDiscreteStringVars,
			 all_discrete_string_variable_labels());
    write_data_annotated(s, allDiscreteRealVars,
			 all_discrete_real_variable_labels());
    // types/ids not required
    s << '\n';
  }
}


void Variables::read_tabular(std::istream& s, unsigned short vars_part)
{
  if (variablesRep)
    variablesRep->read_tabular(s, vars_part); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual read_tabular "
         << "function.\nNo default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Variables::write_tabular(std::ostream& s, unsigned short vars_part) const
{
  if (variablesRep)
    variablesRep->write_tabular(s, vars_part); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual write_tabular "
	 << "function.\nNo default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Variables::
write_tabular_partial(std::ostream& s, size_t start_index,
		      size_t num_items) const
{
  if (variablesRep)
    variablesRep->write_tabular_partial(s, start_index, num_items);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual write_tabular_"
	 << "partial function.\nNo default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Variables::
write_tabular_labels(std::ostream& s, unsigned short vars_part) const
{
  if (variablesRep)
    variablesRep->write_tabular_labels(s, vars_part); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual write_tabular_labels"
	 << " function.\nNo default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


void Variables::
write_tabular_partial_labels(std::ostream& s, size_t start_index,
			     size_t num_items) const
{
  if (variablesRep)
    variablesRep->write_tabular_partial_labels(s, start_index, num_items);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual write_tabular_"
	 << "partial_labels function.\nNo default defined at base class."
	 << std::endl;
    abort_handler(-1);
  }
}


void Variables::read(MPIUnpackBuffer& s)
{
  // MPI buffer version: allow passing of an empty envelope.
  bool buffer_has_letter;
  s >> buffer_has_letter;
  if (buffer_has_letter) {
    ShortShortPair view;
    s >> view.first >> view.second;
    size_t i;
    SizetArray vars_comps_totals(NUM_VC_TOTALS);
    for (i=0; i<NUM_VC_TOTALS; ++i)
      s >> vars_comps_totals[i];
    BitArray all_relax_di, all_relax_dr;
    s >> all_relax_di;
    s >> all_relax_dr;

    SharedVariablesData svd(view, vars_comps_totals, all_relax_di,
			    all_relax_dr);

    if (variablesRep) { // should not occur in current usage
      if (sharedVarsData.view() != view) {
	Cerr << "Warning: variables type mismatch in "
	     << "Variables::read(MPIUnpackBuffer&)." << std::endl;
	variablesRep = get_variables(svd);
      }
    }
    else // buffer read on slaves: variablesRep must be instantiated
      variablesRep = get_variables(svd);

    // This code block would normally be the default implementation (without
    // variablesRep forwards), but we must support additional logic above/below.
    read_data(s, variablesRep->allContinuousVars,
	      all_continuous_variable_labels());
    read_data(s, variablesRep->allDiscreteIntVars,
	      all_discrete_int_variable_labels());
    read_data(s, variablesRep->allDiscreteStringVars,
	      all_discrete_string_variable_labels());
    read_data(s, variablesRep->allDiscreteRealVars,
	      all_discrete_real_variable_labels());
    // rebuild active/inactive views
    variablesRep->build_views();
  }
  else if (variablesRep) {
    // empty variables envelope sent across buffer; make this one match
    variablesRep.reset();
  }
}


void Variables::write(MPIPackBuffer& s) const
{
  // MPI buffer version: allow passing of an empty envelope.
  bool have_letter = !is_null();
  s << have_letter;
  if (have_letter) {
    const ShortShortPair& view = variablesRep->sharedVarsData.view();
    const SizetArray& vc_totals
      = variablesRep->sharedVarsData.components_totals();
    const BitArray& all_relax_di
      = variablesRep->sharedVarsData.all_relaxed_discrete_int();
    const BitArray& all_relax_dr
      = variablesRep->sharedVarsData.all_relaxed_discrete_real();
    s << view.first << view.second;
    size_t i;
    for (i=0; i<NUM_VC_TOTALS; ++i)
      s << vc_totals[i];
    s << all_relax_di;
    s << all_relax_dr;
    write_data(s, variablesRep->allContinuousVars,
	       all_continuous_variable_labels());
    write_data(s, variablesRep->allDiscreteIntVars,
	       all_discrete_int_variable_labels());
    write_data(s, variablesRep->allDiscreteStringVars,
	       all_discrete_string_variable_labels());
    write_data(s, variablesRep->allDiscreteRealVars,
	       all_discrete_real_variable_labels());
    // types/ids not required
  }
  // else empty envelope: send nothing other than initial bool
}


StringArray Variables::ordered_labels(unsigned short vars_part) const
{
  if (variablesRep)
    return variablesRep->ordered_labels(vars_part);
  else {
    // for convenience, use tabular writer tailored to mixed/relaxed
    // TODO: generalize the writer to use a helper that builds an array
    std::ostringstream oss;
    write_tabular_labels(oss, vars_part);
    return strsplit(oss.str());
  }
}


/** Deep copies are used for history mechanisms such as bestVariablesArray
    and data_pairs since these must catalogue copies (and should not
    change as the representation within currentVariables changes). */
Variables Variables::copy(bool deep_svd) const
{
  // the envelope class instantiates a new envelope and a new letter and copies
  // current attributes into the new objects.
  Variables vars; // new envelope, variablesRep=NULL

  if (variablesRep) {
    // deep copy of Variables
    vars.variablesRep =  deep_svd ?
      get_variables(variablesRep->sharedVarsData.copy()) : //    deep SVD copy
      get_variables(variablesRep->sharedVarsData);         // shallow SVD copy

    vars.variablesRep->copy_rep(variablesRep);
  }

  return vars;
}


/** This alternate copy() allows preservation of all*Vars array values while
    adopting different views.  It avoids copying, building views, and then 
    subsequently re-building the views after a view type update. */
Variables Variables::copy(const SharedVariablesData& svd) const
{
  // the envelope class instantiates a new envelope and a new letter and copies
  // current attributes into the new objects.
  Variables vars; // new envelope, variablesRep=NULL

  if (variablesRep) {
    vars.variablesRep = get_variables(svd); // deep copy of Vars, shared svd
    vars.variablesRep->copy_rep(variablesRep);
  }

  return vars;
}


void Variables::copy_rep(std::shared_ptr<Variables> source_vars_rep)
{
  allContinuousVars     = source_vars_rep->allContinuousVars;
  allDiscreteIntVars    = source_vars_rep->allDiscreteIntVars;
  allDiscreteStringVars = source_vars_rep->allDiscreteStringVars;
  allDiscreteRealVars   = source_vars_rep->allDiscreteRealVars;

  build_views();
}


void Variables::shape()
{
  if (variablesRep) // envelope
    variablesRep->shape();
  else { // base class implementation for letters
    size_t num_acv, num_adiv, num_adsv, num_adrv;
    sharedVarsData.all_counts(num_acv, num_adiv, num_adsv, num_adrv);

    allContinuousVars.sizeUninitialized(num_acv);
    allDiscreteIntVars.sizeUninitialized(num_adiv);
    allDiscreteStringVars.resize(boost::extents[num_adsv]);
    allDiscreteRealVars.sizeUninitialized(num_adrv);

    build_views(); // construct active/inactive views of all arrays
  }
}


void Variables::reshape()
{
  if (variablesRep) // envelope
    variablesRep->reshape();
  else { // base class implementation for letters
    size_t num_acv, num_adiv, num_adsv, num_adrv;
    sharedVarsData.all_counts(num_acv, num_adiv, num_adsv, num_adrv);

    allContinuousVars.resize(num_acv);
    allDiscreteIntVars.resize(num_adiv);
    allDiscreteStringVars.resize(boost::extents[num_adsv]);
    allDiscreteRealVars.resize(num_adrv);

    build_views(); // construct active/inactive views of all arrays
  }
}


void Variables::inactive_into_all_variables(const Variables& vars)
{
  // Set inactive variables only, leaving remainder of data unchanged
  if (variablesRep)
    variablesRep->inactive_into_all_variables(vars);
  else {
    size_t icv_start = vars.icv_start(),  idiv_start = vars.idiv_start(),
          idsv_start = vars.idsv_start(), idrv_start = vars.idrv_start(),
          num_icv    = vars.icv(),        num_idiv   = vars.idiv(),
          num_idsv   = vars.idsv(),       num_idrv   = vars.idrv();
    if ( icv_start + num_icv  > acv()  || idiv_start + num_idiv > adiv() ||
	idsv_start + num_idsv > adsv() || idrv_start + num_idrv > adrv() ) {
      Cerr << "Error: inconsistent counts in Variables::"
	   << "inactive_into_all_variables()." << std::endl;
      abort_handler(VARS_ERROR);
    }
    copy_data_partial(vars.inactive_continuous_variables(),
		      allContinuousVars, icv_start);
    copy_data_partial(vars.inactive_discrete_int_variables(),
		      allDiscreteIntVars, idiv_start);
    allDiscreteStringVars[boost::indices[idx_range(idsv_start, num_idsv)]]
      = vars.inactive_discrete_string_variables();
    copy_data_partial(vars.inactive_discrete_real_variables(),
		      allDiscreteRealVars, idrv_start);
  }
}


/// tolerance-based equality for Variables (used in lookup_by_nearby_val())
bool nearby(const Variables& vars1, const Variables& vars2, Real tol)
{
  // this operator is a friend of Variables

  std::shared_ptr<Variables> v1_rep = vars1.variablesRep,
                             v2_rep = vars2.variablesRep;

  // Require different rep pointers
  if (v1_rep == v2_rep)
    return true;

  // Could require identical view, but in terms of cache lookups for purposes
  // of response retrieval, this is not necessary:
  //if ( v1_rep->sharedVarsData.view() != v2_rep->sharedVarsData.view())
  //  return false;

  // tolerance-based equality (ignore labels/types/ids).  Enforce exact equality
  // in discrete real, since these values should not be subject to roundoff.
  return ( nearby(v1_rep->allContinuousVars, v2_rep->allContinuousVars, tol) &&
	   v1_rep->allDiscreteIntVars     == v2_rep->allDiscreteIntVars      &&
	   v1_rep->allDiscreteStringVars  == v2_rep->allDiscreteStringVars   &&
	   v1_rep->allDiscreteRealVars    == v2_rep->allDiscreteRealVars );
}


/// strict binary equality operator (used in boost hashed lookups)
bool operator==(const Variables& vars1, const Variables& vars2)
{
  // this function is a friend of Variables

  std::shared_ptr<Variables> v1_rep = vars1.variablesRep,
                             v2_rep = vars2.variablesRep;

  if (v1_rep == v2_rep)
    return true;

  // Could require identical view, but in terms of cache lookups for purposes
  // of response retrieval, this is not necessary:
  //if (v1_rep->sharedVarsData.view() != v2_rep->sharedVarsData.view())
  //  return false;

  // Require identical content in variable array lengths and values
  // (ignore labels/types/ids) using Teuchos::SerialDenseVector::operator==
  return (v1_rep->allContinuousVars     == v2_rep->allContinuousVars     &&
	  v1_rep->allDiscreteIntVars    == v2_rep->allDiscreteIntVars    &&
	  v1_rep->allDiscreteStringVars == v2_rep->allDiscreteStringVars &&
	  v1_rep->allDiscreteRealVars   == v2_rep->allDiscreteRealVars);
}


/// hash_value for Variables - required by the new BMI hash_set of PRPairs
std::size_t hash_value(const Variables& vars)
{
  // this function is a friend of Variables

  std::size_t seed = 0;
  // require identical views and variables data
  std::shared_ptr<Variables> v_rep = vars.variablesRep;
  boost::hash_combine(seed, v_rep->sharedVarsData.view());
  // hash_value() for SerialDenseVectors and StringMultiArrays defined
  // in dakota_data_util.hpp
  boost::hash_combine(seed, v_rep->allContinuousVars);
  boost::hash_combine(seed, v_rep->allDiscreteIntVars);
  boost::hash_combine(seed, v_rep->allDiscreteStringVars);
  boost::hash_combine(seed, v_rep->allDiscreteRealVars);
  return seed;
}


void Variables::
as_vector(const StringSetArray& dss_vals, RealVector& var_values) const
{
  if (variablesRep)
    variablesRep->as_vector(dss_vals, var_values);
  else {
    size_t num_cv = cv(), num_div = div(), num_dsv = dsv();
    copy_data_partial(continuous_variables(), var_values, 0);
    merge_data_partial(discrete_int_variables(), var_values, num_cv);
    for (size_t i=0; i<num_dsv; ++i) {
      size_t ind = set_value_to_index(discrete_string_variable(i), dss_vals[i]);
      var_values[num_cv + num_div + i] = (Real) ind;
    }
    copy_data_partial(discrete_real_variables(), var_values, 
		      num_cv + num_div + num_dsv);
  }
}


} // namespace Dakota
