/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SharedApproxData.hpp"
#include "ProblemDescDB.hpp"
#include "SharedPecosApproxData.hpp"
#ifdef HAVE_C3
#include "SharedC3ApproxData.hpp"
#endif
#ifdef HAVE_SURFPACK
#include "SharedSurfpackApproxData.hpp"
#endif // HAVE_SURFPACK
#include "ActiveKey.hpp"

//#define DEBUG


namespace Dakota {

/** This constructor is the one which must build the base class data
    for all derived classes.  get_shared_data() instantiates a derived
    class letter and the derived constructor selects this base class
    constructor in its initialization list (to avoid recursion in the
    base class constructor calling get_shared_data() again).  Since the
    letter IS the representation, its rep pointer is set to NULL (an
    uninitialized pointer causes problems in ~SharedApproxData). */
SharedApproxData::
SharedApproxData(BaseConstructor, ProblemDescDB& problem_db, size_t num_vars):
  // See base constructor in DakotaIterator.cpp for full discussion of output
  // verbosity.  For approximations, verbose adds quad poly coeff reporting.
  numVars(num_vars), approxType(problem_db.get_string("model.surrogate.type")),
  buildDataOrder(1), outputLevel(problem_db.get_short("method.output")),
  modelExportPrefix(
    problem_db.get_string("model.surrogate.model_export_prefix")),
  modelExportFormat(
    problem_db.get_ushort("model.surrogate.model_export_format"))
{
  // increment the buildDataOrder based on derivative usage and response
  // gradient/Hessian specifications and approximation type support.  The
  // converse of enforcing minimal data requirements (e.g., TANA) is
  // enforced in the derived classes.
  bool global_approx = strbegins(approxType, "global_"),
    use_derivs = problem_db.get_bool("model.surrogate.derivative_usage");
  buildDataOrder = 1;
  if ( !global_approx || (global_approx && use_derivs) ) {

    // retrieve actual_model_pointer specification and set the DB
    const String& actual_model_ptr
      = problem_db.get_string("model.surrogate.truth_model_pointer");
    size_t model_index = problem_db.get_db_model_node(); // for restoration
    problem_db.set_db_model_nodes(actual_model_ptr);

    if (problem_db.get_string("responses.gradient_type") != "none") {
      if (!global_approx || approxType == "global_polynomial"      ||
	  approxType == "global_regression_orthogonal_polynomial"  ||
#ifdef ALLOW_GLOBAL_HERMITE_INTERPOLATION
	  strends(approxType, "_interpolation_polynomial")         ||
#else
	  approxType == "piecewise_nodal_interpolation_polynomial" ||
	  approxType == "piecewise_hierarchical_interpolation_polynomial" ||
#endif
	  approxType == "global_kriging")
	buildDataOrder |= 2;
      else
	Cerr << "Warning: use_derivatives is not currently supported by "
	     << approxType << " for gradient incorporation.\n\n";
    }
    if (problem_db.get_string("responses.hessian_type")  != "none") {
      if (approxType == "local_taylor" || approxType == "global_polynomial")
	buildDataOrder |= 4;
      else
	Cerr << "Warning: use_derivatives is not currently supported by "
	     << approxType << " for Hessian incorporation.\n\n";
    }

    // restore the specification
    problem_db.set_db_model_nodes(model_index);
  }
}


/** This constructor is the one which must build the base class data
    for all derived classes.  get_shared_data() instantiates a derived
    class letter and the derived constructor selects this base class
    constructor in its initialization list (to avoid recursion in the
    base class constructor calling get_shared_data() again).  Since the
    letter IS the representation, its rep pointer is set to NULL (an
    uninitialized pointer causes problems in ~SharedApproxData). */
SharedApproxData::
SharedApproxData(NoDBBaseConstructor, const String& approx_type,
		 size_t num_vars, short data_order, short output_level):
  numVars(num_vars), approxType(approx_type), outputLevel(output_level),
  modelExportFormat(NO_MODEL_FORMAT), modelExportPrefix("")
{
  bool global_approx = strbegins(approxType, "global_");
  buildDataOrder = 1;
  if (data_order & 2) {
    if (!global_approx || approxType == "global_polynomial"      ||
        approxType == "global_regression_orthogonal_polynomial"  ||
#ifdef ALLOW_GLOBAL_HERMITE_INTERPOLATION
	strends(approxType, "_interpolation_polynomial")         ||
#else
	approxType == "piecewise_nodal_interpolation_polynomial" ||
	approxType == "piecewise_hierarchical_interpolation_polynomial" ||
#endif
	approxType == "global_kriging")
      buildDataOrder |= 2;
    else
      Cerr << "Warning: use_derivatives is not currently supported by "
	   << approxType << " for gradient incorporation.\n\n";
  }
  if (data_order & 4) {
    if (approxType == "local_taylor" || approxType == "global_polynomial")
      buildDataOrder |= 4;
    else
      Cerr << "Warning: use_derivatives is not currently supported by "
	   << approxType << " for Hessian incorporation.\n\n";
  }
}


/** For the default constructor, dataRep is NULL. */
SharedApproxData::SharedApproxData() //:
  //buildDataOrder(1), outputLevel(NORMAL_OUTPUT),
  //modelExportFormat(NO_MODEL_FORMAT), modelExportPrefix(""),
{ /* empty ctor */}


/** Envelope constructor only needs to extract enough data to properly
    execute get_shared_data, since SharedApproxData(BaseConstructor, problem_db)
    builds the actual base class data for the derived approximations. */
SharedApproxData::SharedApproxData(ProblemDescDB& problem_db, size_t num_vars):
  // Set the rep pointer to the appropriate derived type
  dataRep(get_shared_data(problem_db, num_vars))
{
  if ( !dataRep ) // bad type or insufficient memory
    abort_handler(APPROX_ERROR);
}


/** Used only by the envelope constructor to initialize dataRep to the 
    appropriate derived type. */
std::shared_ptr<SharedApproxData> SharedApproxData::
get_shared_data(ProblemDescDB& problem_db, size_t num_vars)
{
  const String& approx_type = problem_db.get_string("model.surrogate.type");
  //if (approx_type == "local_taylor")
  //  return new SharedTaylorApproxData(problem_db, num_vars);
  //else if (approx_type == "multipoint_tana")
  //  return new SharedTANA3ApproxData(problem_db, num_vars);
  if (strends(approx_type, "_orthogonal_polynomial") ||
      strends(approx_type, "_interpolation_polynomial"))
    return std::make_shared<SharedPecosApproxData>(problem_db, num_vars);
#ifdef HAVE_C3
  else if (approx_type == "global_function_train")
    return std::make_shared<SharedC3ApproxData>(problem_db, num_vars);
#endif
  //else if (approx_type == "global_gaussian")
  //  return new SharedGaussProcApproxData(problem_db, num_vars);
#ifdef HAVE_SURFPACK
  else if (approx_type == "global_polynomial"     ||
	   approx_type == "global_kriging"        ||
	   approx_type == "global_neural_network" || // TO DO: Two ANN's ?
	   approx_type == "global_radial_basis"   ||
	   approx_type == "global_mars"           ||
	   approx_type == "global_moving_least_squares" ||
	   // Overloading use of SharedSurfpackApproxData for now:
	   approx_type == "global_exp_gauss_proc" ||
	   approx_type == "global_exp_poly")
    return std::make_shared<SharedSurfpackApproxData>(problem_db, num_vars);
#endif // HAVE_SURFPACK
  else {
    //Cerr << "Error: SharedApproxData type " << approx_type
    //     << " not available." << std::endl;
    //return NULL;
    return std::shared_ptr<SharedApproxData>
      (new SharedApproxData(BaseConstructor(), problem_db, num_vars));
  }
}


/** This is the alternate envelope constructor for instantiations on
    the fly.  Since it does not have access to problem_db, it utilizes
    the NoDBBaseConstructor constructor chain. */
SharedApproxData::
SharedApproxData(const String& approx_type, const UShortArray& approx_order,
		 size_t num_vars, short data_order, short output_level):
  // Set the rep pointer to the appropriate derived type
  dataRep(get_shared_data(approx_type, approx_order, num_vars,
			  data_order, output_level))
{
  if ( !dataRep ) // bad type or insufficient memory
    abort_handler(APPROX_ERROR);
}


/** Used only by the envelope constructor to initialize dataRep to the 
    appropriate derived type. */
std::shared_ptr<SharedApproxData> SharedApproxData::
get_shared_data(const String& approx_type, const UShortArray& approx_order, 
		size_t num_vars, short data_order, short output_level)
{
  //if (approx_type == "local_taylor")
  //  approx = new SharedTaylorApproxData(num_vars, data_order, output_level);
  //else if (approx_type == "multipoint_tana")
  //  approx = new SharedTANA3ApproxData(num_vars, data_order, output_level);
  if (strends(approx_type, "_orthogonal_polynomial") ||
      strends(approx_type, "_interpolation_polynomial"))
    return std::make_shared<SharedPecosApproxData>
      (approx_type, approx_order, num_vars, data_order, output_level);
#ifdef HAVE_C3
  else if (approx_type == "global_function_train")
    return std::make_shared<SharedC3ApproxData>
      (approx_type, approx_order, num_vars, data_order, output_level);
#endif
  //else if (approx_type == "global_gaussian")
  //  approx = new SharedGaussProcApproxData(num_vars, data_order,output_level);
#ifdef HAVE_SURFPACK
  else if (approx_type == "global_polynomial"     ||
	   approx_type == "global_kriging"        ||
	   approx_type == "global_neural_network" || // TO DO: Two ANN's ?
	   approx_type == "global_radial_basis"   ||
	   approx_type == "global_mars"           ||
	   approx_type == "global_moving_least_squares" ||
	   approx_type == "global_voronoi_surrogate" ||
	   // Overloading use of SharedSurfpackApproxData for now:
	   approx_type == "global_exp_gauss_proc" ||
	   approx_type == "global_exp_poly"
           )
    return std::make_shared<SharedSurfpackApproxData>
      (approx_type, approx_order, num_vars, data_order, output_level);
#endif // HAVE_SURFPACK
  else {
    //Cerr << "Error: ApproxData type " << approx_type << " not available."
    //     << std::endl;
    //approx = NULL;
    return std::shared_ptr<SharedApproxData>
      (new SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars,
			    data_order, output_level));
  }
  return std::shared_ptr<SharedApproxData>();
}


/** Copy constructor manages sharing of dataRep. */
SharedApproxData::SharedApproxData(const SharedApproxData& shared_data):
  dataRep(shared_data.dataRep)
{ /* empty ctor */ }


SharedApproxData SharedApproxData::
operator=(const SharedApproxData& shared_data)
{
  dataRep = shared_data.dataRep;
  return *this; // calls copy constructor since returned by value
}


SharedApproxData::~SharedApproxData()
{ /* empty dtor */ }


void SharedApproxData::active_model_key(const Pecos::ActiveKey& key)
{
  // > AGGREGATED_MODEL_PAIR uses {HF,LF} order as does ApproxInterface::*_add()
  // > When managing distinct sets of paired truth,surrogate data (e.g., one set
  //   of data for Q_l - Q_lm1 and another for Q_lm1 - Q_lm2, it is important to
  //   identify the lm1 data with a specific pairing --> data group index
  //   pre-pend in {truth,surrogate,combined} keys.

  if (dataRep) dataRep->active_model_key(key);
  else activeKey = key;//.copy();
}


void SharedApproxData::clear_model_keys()
{
  if (dataRep) dataRep->clear_model_keys();
  else activeKey.clear();
}


/*
void SharedApproxData::link_multilevel_surrogate_data()
{
  if (dataRep) dataRep->link_multilevel_surrogate_data();
  //else no-op (no linkage required for derived SharedApproxData)

  //else
  //  switch (discrepancy_reduction()) {
  //  case Pecos::DISTINCT_DISCREPANCY: case Pecos::RECURSIVE_DISCREPANCY:
  //    approxDataKeys.resize(3); // HF, LF, discrep
  //    break;
  //  default: // default ctor linkages are sufficient
  //    break;
  //  }
}
*/


void SharedApproxData::integration_iterator(const Iterator& iterator)
{
  if (dataRep) dataRep->integration_iterator(iterator);
  else { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: integration_iterator() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


short SharedApproxData::discrepancy_reduction() const
{
  if (dataRep) return dataRep->discrepancy_reduction();
  else         return Pecos::NO_DISCREPANCY; // this enum is 0
}


void SharedApproxData::build()
{
  if (dataRep) dataRep->build();
  //else no-op (implementation not required for shared data)
}


void SharedApproxData::rebuild()
{
  if (dataRep) dataRep->rebuild();
  else         build();// if incremental not defined, fall back to full build
}


void SharedApproxData::pop(bool save_surr_data)
{
  if (dataRep) dataRep->pop(save_surr_data);
  //else no-op (derived implementation not required)
}


bool SharedApproxData::push_available()
{
  if (!dataRep) { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: push_available() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return dataRep->push_available();
}


size_t SharedApproxData::push_index(const Pecos::ActiveKey& key)
{
  if (!dataRep) { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: push_index() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return dataRep->push_index(key);
}


void SharedApproxData::pre_push()
{
  if (dataRep)
    dataRep->pre_push();
  //else no-op (derived implementation not required)
}


void SharedApproxData::post_push()
{
  if (dataRep)
    dataRep->post_push();
  //else no-op (derived implementation not required)
}


size_t SharedApproxData::finalize_index(size_t i, const Pecos::ActiveKey& key)
{
  if (!dataRep) { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: finalize_index() not available for this approximation type."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return dataRep->finalize_index(i, key);
}


void SharedApproxData::pre_finalize()
{
  if (dataRep)
    dataRep->pre_finalize();
  //else no-op (derived implementation not required)
}


void SharedApproxData::post_finalize()
{
  if (dataRep)
    dataRep->post_finalize();
  //else no-op (derived implementation not required)
}


void SharedApproxData::clear_inactive()
{
  if (dataRep)
    dataRep->clear_inactive();
  //else
  //  default: no stored data to clear
}


void SharedApproxData::pre_combine()
{
  if (dataRep)
    dataRep->pre_combine();
  //else no-op (derived implementation not required)
}


void SharedApproxData::post_combine()
{
  if (dataRep)
    dataRep->post_combine();
  //else no-op (derived implementation not required)
}


void SharedApproxData::combined_to_active(bool clear_combined)
{
  if (dataRep)
    dataRep->combined_to_active(clear_combined);
  //else no-op (derived implementation not required)
}


bool SharedApproxData::advancement_available()
{
  if (dataRep) return dataRep->advancement_available();
  else         return true; // only a few cases throttle advancements
}


void SharedApproxData::increment_order()
{
  if (dataRep)
    dataRep->increment_order();
  else { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: increment_order() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


void SharedApproxData::decrement_order()
{
  if (dataRep)
    dataRep->decrement_order();
  else { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: decrement_order() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


void SharedApproxData::
construct_basis(const Pecos::MultivariateDistribution& mv_dist)
{
  if (dataRep) dataRep->construct_basis(mv_dist);
  else { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: construct_basis() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


void SharedApproxData::
update_basis_distribution_parameters(const Pecos::MultivariateDistribution& mvd)
{
  if (dataRep)
    dataRep->update_basis_distribution_parameters(mvd);
  //else no-op (derived implementation not required)
}


bool SharedApproxData::formulation_updated() const
{
  if (dataRep) return dataRep->formulation_updated();
  else { // not virtual
    std::map<Pecos::ActiveKey, bool>::const_iterator cit
      = formUpdated.find(activeKey);
    return (cit == formUpdated.end()) ? false : cit->second;
  }
}


void SharedApproxData::formulation_updated(bool update)
{
  if (dataRep) dataRep->formulation_updated(update);
  else         formUpdated[activeKey] = update;
}


void SharedApproxData::
configuration_options(const Pecos::ExpansionConfigOptions& ec_options)
{
  if (dataRep)
    dataRep->configuration_options(ec_options);
  //else no-op (derived implementation not required)
}


void SharedApproxData::
configuration_options(const Pecos::BasisConfigOptions& bc_options)
{
  if (dataRep)
    dataRep->configuration_options(bc_options);
  //else no-op (derived implementation not required)
}


void SharedApproxData::
configuration_options(const Pecos::RegressionConfigOptions& rc_options)
{
  if (dataRep)
    dataRep->configuration_options(rc_options);
  //else no-op (derived implementation not required)
}


void SharedApproxData::random_variables_key(const BitArray& random_vars_key)
{
  if (dataRep)
    dataRep->random_variables_key(random_vars_key);
  //else no-op (derived implementation not required)
}


void SharedApproxData::refinement_statistics_mode(short stats_mode)
{
  if (dataRep)
    dataRep->refinement_statistics_mode(stats_mode);
  //else no-op (derived implementation not required)
}


const Pecos::BitArrayULongMap& SharedApproxData::sobol_index_map() const
{
  if (!dataRep) {
    Cerr << "Error: sobol_index_map() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return dataRep->sobol_index_map();
}

} // namespace Dakota
