/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedApproxData
//- Description:  Class implementation of base class for approximations
//-               
//- Owner:        Mike Eldred, Sandia National Laboratories

#include "SharedApproxData.hpp"
#include "ProblemDescDB.hpp"
#include "SharedPecosApproxData.hpp"
#ifdef HAVE_C3
#include "SharedC3ApproxData.hpp"
#endif
#ifdef HAVE_SURFPACK
#include "SharedSurfpackApproxData.hpp"
#endif // HAVE_SURFPACK

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
  outputLevel(problem_db.get_short("method.output")),
  numVars(num_vars), approxType(problem_db.get_string("model.surrogate.type")),
  buildDataOrder(1), activeDataIndex(0), 
  modelExportPrefix(
    problem_db.get_string("model.surrogate.model_export_prefix")),
  modelExportFormat(
    problem_db.get_ushort("model.surrogate.model_export_format")),
  dataRep(NULL), referenceCount(1)
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
      = problem_db.get_string("model.surrogate.actual_model_pointer");
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

  // initialize sequence of one empty key for first Approximation::approxData
  approxDataKeys.resize(1);  approxDataKeys[0].resize(1);

#ifdef REFCOUNT_DEBUG
  Cout << "SharedApproxData::SharedApproxData(BaseConstructor) called to build "
       << "base class for letter." << std::endl;
#endif
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
  activeDataIndex(0), modelExportFormat(NO_MODEL_FORMAT), modelExportPrefix(""),
  dataRep(NULL), referenceCount(1)
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

  // initialize sequence of one empty key for first Approximation::approxData
  approxDataKeys.resize(1);  approxDataKeys[0].resize(1);

#ifdef REFCOUNT_DEBUG
  Cout << "SharedApproxData::SharedApproxData(NoDBBaseConstructor) called to "
       << "build base class for letter." << std::endl;
#endif
}


/** For the default constructor, dataRep is NULL.  This makes it
    necessary to check for NULL in the copy constructor, assignment
    operator, and destructor. */
SharedApproxData::SharedApproxData():
  //buildDataOrder(1), outputLevel(NORMAL_OUTPUT),
  //modelExportFormat(NO_MODEL_FORMAT), modelExportPrefix(""),
  dataRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedApproxData::SharedApproxData() called to build empty "
       << "approximation object." << std::endl;
#endif
}


/** Envelope constructor only needs to extract enough data to properly
    execute get_shared_data, since SharedApproxData(BaseConstructor, problem_db)
    builds the actual base class data for the derived approximations. */
SharedApproxData::SharedApproxData(ProblemDescDB& problem_db, size_t num_vars):
  referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedApproxData::SharedApproxData(ProblemDescDB&) called to "
       << "instantiate envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate derived type
  dataRep = get_shared_data(problem_db, num_vars);
  if ( !dataRep ) // bad type or insufficient memory
    abort_handler(APPROX_ERROR);
}


/** Used only by the envelope constructor to initialize dataRep to the 
    appropriate derived type. */
SharedApproxData* SharedApproxData::
get_shared_data(ProblemDescDB& problem_db, size_t num_vars)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter in get_shared_data(ProblemDescDB&)."
       << std::endl;
#endif

  const String& approx_type = problem_db.get_string("model.surrogate.type");
  //if (approx_type == "local_taylor")
  //  return new SharedTaylorApproxData(problem_db, num_vars);
  //else if (approx_type == "multipoint_tana")
  //  return new SharedTANA3ApproxData(problem_db, num_vars);
  if (strends(approx_type, "_orthogonal_polynomial") ||
      strends(approx_type, "_interpolation_polynomial"))
    return new SharedPecosApproxData(problem_db, num_vars);
#ifdef HAVE_C3
  else if (approx_type == "global_function_train")
    return new SharedC3ApproxData(problem_db,num_vars);
#endif
  //else if (approx_type == "global_gaussian")
  //  return new SharedGaussProcApproxData(problem_db, num_vars);
#ifdef HAVE_SURFPACK
  else if (approx_type == "global_polynomial"     ||
	   approx_type == "global_kriging"        ||
	   approx_type == "global_neural_network" || // TO DO: Two ANN's ?
	   approx_type == "global_radial_basis"   ||
	   approx_type == "global_mars"           ||
	   approx_type == "global_moving_least_squares")
    return new SharedSurfpackApproxData(problem_db, num_vars);
#endif // HAVE_SURFPACK
  else {
    //Cerr << "Error: SharedApproxData type " << approx_type
    //     << " not available." << std::endl;
    //return NULL;
    return new SharedApproxData(BaseConstructor(), problem_db, num_vars);
  }
}


/** This is the alternate envelope constructor for instantiations on
    the fly.  Since it does not have access to problem_db, it utilizes
    the NoDBBaseConstructor constructor chain. */
SharedApproxData::
SharedApproxData(const String& approx_type, const UShortArray& approx_order,
		 size_t num_vars, short data_order, short output_level):
  referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedApproxData::SharedApproxData(String&) called to instantiate "
       << "envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate derived type
  dataRep = get_shared_data(approx_type, approx_order, num_vars,
			    data_order, output_level);
  if ( !dataRep ) // bad type or insufficient memory
    abort_handler(APPROX_ERROR);
}


/** Used only by the envelope constructor to initialize dataRep to the 
    appropriate derived type. */
SharedApproxData* SharedApproxData::
get_shared_data(const String& approx_type, const UShortArray& approx_order, 
		size_t num_vars, short data_order, short output_level)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter in get_shared_data(String&)."
       << std::endl;
#endif

  SharedApproxData* approx;
  //if (approx_type == "local_taylor")
  //  approx = new SharedTaylorApproxData(num_vars, data_order, output_level);
  //else if (approx_type == "multipoint_tana")
  //  approx = new SharedTANA3ApproxData(num_vars, data_order, output_level);
  if (strends(approx_type, "_orthogonal_polynomial") ||
      strends(approx_type, "_interpolation_polynomial"))
    approx = new SharedPecosApproxData(approx_type, approx_order, num_vars,
				       data_order, output_level);
#ifdef HAVE_C3
  else if (approx_type == "global_function_train")
    approx = new SharedC3ApproxData(approx_type, approx_order, num_vars,
				    data_order, output_level);
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
       approx_type == "global_voronoi_surrogate"
           )
    approx = new SharedSurfpackApproxData(approx_type, approx_order, num_vars,
					  data_order, output_level);
#endif // HAVE_SURFPACK
  else {
    //Cerr << "Error: ApproxData type " << approx_type << " not available."
    //     << std::endl;
    //approx = NULL;
    approx = new SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars,
				  data_order, output_level);
  }
  return approx;
}


/** Copy constructor manages sharing of dataRep and incrementing
    of referenceCount. */
SharedApproxData::SharedApproxData(const SharedApproxData& shared_data)
{
  // Increment new (no old to decrement)
  dataRep = shared_data.dataRep;
  if (dataRep) // Check for an assignment of NULL
    ++dataRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "SharedApproxData::SharedApproxData(SharedApproxData&)" << std::endl;
  if (dataRep)
    Cout << "dataRep referenceCount = " << dataRep->referenceCount
	 << std::endl;
#endif
}


/** Assignment operator decrements referenceCount for old dataRep, assigns
    new dataRep, and increments referenceCount for new dataRep. */
SharedApproxData SharedApproxData::
operator=(const SharedApproxData& shared_data)
{
  if (dataRep != shared_data.dataRep) { // normal case: old != new
    // Decrement old
    if (dataRep) // Check for NULL
      if ( --dataRep->referenceCount == 0 ) 
	delete dataRep;
    // Assign and increment new
    dataRep = shared_data.dataRep;
    if (dataRep) // Check for NULL
      ++dataRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "SharedApproxData::operator=(SharedApproxData&)" << std::endl;
  if (dataRep)
    Cout << "dataRep referenceCount = " << dataRep->referenceCount
	 << std::endl;
#endif

  return *this; // calls copy constructor since returned by value
}


/** Destructor decrements referenceCount and only deletes dataRep
    when referenceCount reaches zero. */
SharedApproxData::~SharedApproxData()
{ 
  // Check for NULL pointer 
  if (dataRep) {
    --dataRep->referenceCount;
#ifdef REFCOUNT_DEBUG
    Cout << "dataRep referenceCount decremented to " 
	 << dataRep->referenceCount << std::endl;
#endif
    if (dataRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting dataRep" << std::endl;
#endif
      delete dataRep;
    }
  }
}


void SharedApproxData::active_model_key(const UShortArray& mi_key)
{
  if (dataRep)
    dataRep->active_model_key(mi_key);
  //else no-op (implementation not required for shared data)
}


const UShortArray& SharedApproxData::active_model_key() const
{
  if (!dataRep) { // virtual fn: no default, error if not supplied by derived
    Cerr << "Error: active_model_key() not available for this approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }

  return dataRep->active_model_key();
}


void SharedApproxData::clear_model_keys()
{
  if (dataRep)
    dataRep->clear_model_keys();
  //else no-op (implementation not required for shared data)
}


void SharedApproxData::link_multilevel_surrogate_data()
{
  if (dataRep)
    dataRep->link_multilevel_surrogate_data();
  else {
    Cerr << "Error: link_multilevel_surrogate_data() not available for this "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


void SharedApproxData::surrogate_model_key(const UShortArray& key)
{
  if (dataRep)
    dataRep->surrogate_model_key(key);
  else { // default implementation: no key augmentation
    UShort2DArray& data_keys = approxDataKeys[activeDataIndex];
    // AGGREGATED_MODELS mode uses {HF,LF} order, as does
    // ApproximationInterface::*_add()
    if (key.empty()) // prune second entry, if present, from approxDataKeys
      data_keys.resize(1);
    else {
      data_keys.resize(2);
      data_keys[1] = key; // assign incoming LF key
    }
  }
}


void SharedApproxData::truth_model_key(const UShortArray& key)
{
  if (dataRep)
    dataRep->truth_model_key(key);
  else { // default implementation: no key augmentation
    UShort2DArray& data_keys = approxDataKeys[activeDataIndex];
    // approxDataKeys size can remain 1 if no {truth,surrogate} aggregation
    switch  (data_keys.size()) {
    case 0:  data_keys.push_back(key); break;
    default: data_keys[0] = key;       break;
    }
  }
}


const UShortArray& SharedApproxData::surrogate_model_key() const
{
  if (dataRep)
    return dataRep->surrogate_model_key();
  else { // default implementation
    const UShort2DArray& data_keys = approxDataKeys[activeDataIndex];
    if (data_keys.size() < 2) {
      Cerr << "Error: no key defined in SharedApproxData::surrogate_model_key()"
	   << std::endl;
      abort_handler(APPROX_ERROR);
      // or could return empty key by value
    }
    return data_keys.back();
  }
}


const UShortArray& SharedApproxData::truth_model_key() const
{
  if (dataRep)
    return dataRep->truth_model_key();
  else // default implementation
    return approxDataKeys[activeDataIndex].front();
}


void SharedApproxData::build()
{
  if (dataRep)
    dataRep->build();
  //else no-op (implementation not required for shared data)
}


void SharedApproxData::rebuild()
{
  if (dataRep)
    dataRep->rebuild();
  else // if incremental rebuild not defined, fall back to full build
    build();
}


void SharedApproxData::pop(bool save_surr_data)
{
  if (dataRep)
    dataRep->pop(save_surr_data);
  //else no-op (implementation not required for shared data)
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


size_t SharedApproxData::push_index(const UShortArray& key)
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
  else {
    Cerr << "\nError: pre_push() not defined for this shared approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


void SharedApproxData::post_push()
{
  if (dataRep)
    dataRep->post_push();
  else {
    Cerr << "\nError: post_push() not defined for this shared approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


size_t SharedApproxData::finalize_index(size_t i, const UShortArray& key)
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
  else {
    Cerr << "\nError: pre_finalize() not defined for this shared approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


void SharedApproxData::post_finalize()
{
  if (dataRep)
    dataRep->post_finalize();
  else {
    Cerr << "\nError: post_finalize() not defined for this shared "
	 << "approximation type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
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
  else {
    Cerr << "\nError: pre_combine() not defined for this shared approximation "
	 << "type." << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


void SharedApproxData::post_combine()
{
  if (dataRep)
    dataRep->post_combine();
  //else
  //  default: no post combine required
}


void SharedApproxData::combined_to_active(bool clear_combined)
{
  if (dataRep)
    dataRep->combined_to_active(clear_combined);
  //else
  //  default: no op
}


void SharedApproxData::
update_basis_distribution_parameters(const Pecos::MultivariateDistribution& mvd)
{
  if (dataRep)
    dataRep->update_basis_distribution_parameters(mvd);
  //else
  //  default: no op
}


void SharedApproxData::
configuration_options(const Pecos::ExpansionConfigOptions& ec_options)
{
  if (dataRep)
    dataRep->configuration_options(ec_options);
  //else
  //  default: no op
}


void SharedApproxData::
configuration_options(const Pecos::BasisConfigOptions& bc_options)
{
  if (dataRep)
    dataRep->configuration_options(bc_options);
  //else
  //  default: no op
}


void SharedApproxData::
configuration_options(const Pecos::RegressionConfigOptions& rc_options)
{
  if (dataRep)
    dataRep->configuration_options(rc_options);
  //else
  //  default: no op
}


void SharedApproxData::random_variables_key(const BitArray& random_vars_key)
{
  if (dataRep)
    dataRep->random_variables_key(random_vars_key);
  //else
  //  default: no op
}


void SharedApproxData::refinement_statistics_type(short stats_type)
{
  if (dataRep)
    dataRep->refinement_statistics_type(stats_type);
  //else
  //  default: no op
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
