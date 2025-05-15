/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_types.hpp"
#include "dakota_data_util.hpp"
#include "interface_utils.hpp"
#include "DakotaInterface.hpp"
#include "DakotaResponse.hpp"
#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"


#ifdef HAVE_AMPL
#undef NO // avoid name collision from UTILIB
#include "external/ampl/asl.h"
#endif // HAVE_AMPL

//#define DEBUG

namespace Dakota {

// Initialization of static interface ID counters
size_t Interface::noSpecIdNum = 0;

std::shared_ptr<Interface> Interface::get_interface(ProblemDescDB& problem_db) {

  ProblemDescDB* const study_ptr = problem_db.get_rep().get();

  if(Interface::interfaceCache.count(study_ptr) == 0) {
    std::cout << "Interface::get_interface(): Study not in the cache\n";
    Interface::interfaceCache[study_ptr] = std::list<std::shared_ptr<Interface>>();
  } else {
    std::cout << "Interface::get_interface(): Study found in cache\n";
  }

  auto& study_cache = Interface::interfaceCache[study_ptr];

  // Have to worry about loss of encapsulation and use of context _above_ this
  // specification, i.e., any dependence on iterator/model/variables/responses
  // specifications (dependence on the environment specification is OK since
  // there is only one):
  // > Interface: method.output
  // > ApplicationInterface: responses.gradient_type, responses.hessian_type,
  //     responses.gradients.mixed.id_analytic
  // > DakotaInterface: responses.labels

  // ApproximationInterfaces and related classes are OK, since they are
  // instantiated with assign_rep() for each unique DataFitSurrModel instance:
  // > ApproximationInterface: model.surrogate.function_ids
  // > Approximation: method.output, model.surrogate.type,
  //     model.surrogate.derivative_usage
  // > SurfpackApproximation: model.surrogate.polynomial_order,
  //     model.surrogate.kriging_correlations
  // > TaylorApproximation: model.surrogate.truth_model_pointer,
  //     responses.hessian_type
  // > OrthogPolyApproximation: method.nond.expansion_{terms,order}

  // The DB list nodes are set prior to calling get_interface():
  // >    interface_ptr spec -> id_interface must be defined
  // > no interface_ptr spec -> id_interf ignored, interf spec = last parsed
  auto id_interface = problem_db.interface_id();
  if(id_interface.empty()) {
    std::cout << "Interface::get_interface(): interface_id() is empty.\n";
    id_interface = "NO_MODEL_ID";
  }
  auto m_it
    = std::find_if(study_cache.begin(), study_cache.end(),
                   [&id_interface](std::shared_ptr<Interface> m) {return m->interface_id() == id_interface;});
  if (m_it == study_cache.end()) {
    std::cout << "Interface:get_interface(): Interface lookup failed.\n";
    study_cache.push_back(InterfaceUtils::get_interface(problem_db));
    m_it = --study_cache.end();
  }
  std::cout << "Interface::get_interface(): returning interface of type " << (*m_it)->interface_type() << std::endl;
  return *m_it;
}

std::list<std::shared_ptr<Interface>>& Interface::interface_cache(ProblemDescDB& problem_db) {
  const ProblemDescDB* const study_ptr = problem_db.get_rep().get();
  try {
    return Interface::interfaceCache.at(study_ptr);
  } catch(std::out_of_range) {
    Cerr << "Interface::interface_cache() called with nonexistent study!\n";
    throw;
  }
}

void Interface::remove_cached_interface(const ProblemDescDB& problem_db) {
  const ProblemDescDB* const study_ptr = problem_db.get_rep().get();
  Interface::interfaceCache.erase(study_ptr);
}

std::map<const ProblemDescDB*, std::list<std::shared_ptr<Interface>>> Interface::interfaceCache{};


/** Base class constructor to initialize class data for all
    inherited interfaces.  InterfaceUtils::get_interface(...)
    instantiates derived classs */

Interface::Interface(const ProblemDescDB& problem_db): 
  interfaceType(problem_db.get_ushort("interface.type")),
  interfaceId(problem_db.get_string("interface.id")), 
  analysisComponents(
    problem_db.get_s2a("interface.application.analysis_components")),
  algebraicMappings(false),
  coreMappings(true), outputLevel(problem_db.get_short("method.output")),
  currEvalId(0), fineGrainEvalCounters(outputLevel > NORMAL_OUTPUT),
  evalIdCntr(0), newEvalIdCntr(0), evalIdRefPt(0), newEvalIdRefPt(0),
  multiProcEvalFlag(false), ieDedSchedFlag(false),
  // See base constructor in DakotaIterator.cpp for full discussion of output
  // verbosity.  Interfaces support the full granularity in verbosity.
  appendIfaceId(true), asl(NULL)
{
#ifdef DEBUG
  outputLevel = DEBUG_OUTPUT;
#endif // DEBUG
  if(interfaceId.empty())
      interfaceId = user_auto_id();
  // Process the algebraic_mappings file (an AMPL .nl file) to get the number
  // of variables/responses (currently, the tags are converted to index arrays
  // at evaluation time, using the passed vars and response).
  // TO DO: parallel bcast of data or very proc reads file?
  const String& ampl_file_name
    = problem_db.get_string("interface.algebraic_mappings");
  if (!ampl_file_name.empty()) {
#ifdef HAVE_AMPL
    algebraicMappings = true;
    bool hess_flag
      = (problem_db.get_string("responses.hessian_type") == "analytic");
    asl = (hess_flag) ? ASL_alloc(ASL_read_pfgh) : ASL_alloc(ASL_read_fg);
    // allow user input of either stub or stub.nl
    String stub = (strends(ampl_file_name, ".nl")) ? 
      String(ampl_file_name, 0, ampl_file_name.size() - 3) : ampl_file_name;
    //std::ifstream ampl_nl(ampl_file_name);
    fint stub_str_len = stub.size();
    // BMA NOTE: casting away the constness as done historically in DakotaString
    char* nonconst_stub = (char*) stub.c_str();
    FILE* ampl_nl = jac0dim(nonconst_stub, stub_str_len);
    if (!ampl_nl) {
      Cerr << "\nError: failure opening " << ampl_file_name << std::endl;
      abort_handler(IO_ERROR);
    }
    int rtn = (hess_flag) ? pfgh_read(ampl_nl, ASL_return_read_err)
                          :   fg_read(ampl_nl, ASL_return_read_err);
    if (rtn) {
      Cerr << "\nError: AMPL processing problem with " << ampl_file_name
	   << std::endl;
      abort_handler(IO_ERROR);
    }

    // extract input/output tag lists
    String row = stub + ".row", col = stub + ".col", ampl_tag;

    std::ifstream ampl_col(col.c_str());
    if (!ampl_col) {
      Cerr << "\nError: failure opening " << col << std::endl;
      abort_handler(IO_ERROR);
    }
    algebraicVarTags.resize(n_var);
    for (size_t i=0; i<n_var; i++) {
      std::getline(ampl_col, ampl_tag);
      if (ampl_col.good())
	algebraicVarTags[i] = ampl_tag;
      else {
	Cerr << "\nError: failure reading AMPL col file " << col
	     << std::endl;
	abort_handler(IO_ERROR);
      }
    }

    std::ifstream ampl_row(row.c_str());
    if (!ampl_row) {
      Cerr << "\nError: failure opening " << row << std::endl;
      abort_handler(IO_ERROR);
    }
    algebraicFnTags.resize(n_obj+n_con);
    algebraicFnTypes.resize(n_obj+n_con);
    algebraicConstraintWeights.resize(n_con);
    for (size_t i=0; i<n_obj+n_con; i++) {
      getline(ampl_row, ampl_tag);
      if (ampl_row.good()) {
	algebraicFnTags[i] = ampl_tag;
	algebraicFnTypes[i] = algebraic_function_type(ampl_tag);
      }
      else {
	Cerr << "\nError: failure reading AMPL row file " << row
	     << std::endl;
	abort_handler(IO_ERROR);
      }
    }

#ifdef DEBUG
    Cout << ">>>>> algebraicVarTags =\n" << algebraicVarTags
	 << "\n>>>>> algebraicFnTags =\n" << algebraicFnTags
	 << "\n>>>>> algebraicFnTypes =\n" << algebraicFnTypes << std::endl;
#endif

#else
    Cerr << "\nError: algebraic_mappings not supported without the AMPL solver "
	 << "library provided with the Acro package." << std::endl;
    abort_handler(-1);
#endif // HAVE_AMPL
  }
}


Interface::Interface(size_t num_fns, short output_level):
  interfaceId(no_spec_id()), algebraicMappings(false), coreMappings(true),
  outputLevel(output_level), currEvalId(0), 
  fineGrainEvalCounters(outputLevel > NORMAL_OUTPUT), evalIdCntr(0), 
  newEvalIdCntr(0), evalIdRefPt(0), newEvalIdRefPt(0), multiProcEvalFlag(false),
  ieDedSchedFlag(false), appendIfaceId(true)
{
#ifdef DEBUG
  outputLevel = DEBUG_OUTPUT;
#endif // DEBUG
}


/** Copy constructor */
Interface::Interface(const Interface& interface_in)
{ /* empty ctor */ }


Interface Interface::operator=(const Interface& interface_in)
{
  return *this; // calls copy constructor since returned by value
}


Interface::~Interface()
{ /* empty dtor */ }


void Interface::fine_grained_evaluation_counters(size_t num_fns)
{
  if (!fineGrainEvalCounters) {
    init_evaluation_counters(num_fns);
    fineGrainEvalCounters = true;
  }
}


void Interface::init_evaluation_counters(size_t num_fns)
{
  //if (fnLabels.empty()) {
  //  fnLabels.resize(num_fns);
  //  build_labels(fnLabels, "response_fn_"); // generic resp fn labels
  //}
  if (fnValCounter.size() != num_fns) {
    fnValCounter.assign(num_fns, 0);     fnGradCounter.assign(num_fns, 0);
    fnHessCounter.assign(num_fns, 0);    newFnValCounter.assign(num_fns, 0);
    newFnGradCounter.assign(num_fns, 0); newFnHessCounter.assign(num_fns, 0);
    fnValRefPt.assign(num_fns, 0);       fnGradRefPt.assign(num_fns, 0);
    fnHessRefPt.assign(num_fns, 0);      newFnValRefPt.assign(num_fns, 0);
    newFnGradRefPt.assign(num_fns, 0);   newFnHessRefPt.assign(num_fns, 0);
  }
}


void Interface::set_evaluation_reference()
{
  evalIdRefPt    = evalIdCntr;
  newEvalIdRefPt = newEvalIdCntr;

  if (fineGrainEvalCounters) {
    size_t i, num_fns = fnValCounter.size();
    for (i=0; i<num_fns; i++) {
      fnValRefPt[i]     =     fnValCounter[i];
      newFnValRefPt[i]  =  newFnValCounter[i];
      fnGradRefPt[i]    =    fnGradCounter[i];
      newFnGradRefPt[i] = newFnGradCounter[i];
      fnHessRefPt[i]    =    fnHessCounter[i];
      newFnHessRefPt[i] = newFnHessCounter[i];
    }
  }
}


void Interface::
print_evaluation_summary(std::ostream& s, bool minimal_header,
			 bool relative_count) const
{
  // standard evaluation summary
  if (minimal_header) {
    if (interfaceId.empty() || interfaceId == "NO_ID")
      s << "  Interface evaluations";
    else
      s << "  " << interfaceId << " evaluations";
  }
  else {
    s << "<<<<< Function evaluation summary";
    if (!(interfaceId.empty() || interfaceId == "NO_ID"))
      s << " (" << interfaceId << ')';
  }
  int     fn_evals = (relative_count) ? evalIdCntr - evalIdRefPt
                                      : evalIdCntr;
  int new_fn_evals = (relative_count) ? newEvalIdCntr - newEvalIdRefPt
                                      : newEvalIdCntr;
  s << ": " << fn_evals << " total (" << new_fn_evals << " new, "
    << fn_evals - new_fn_evals << " duplicate)\n";

  // detailed evaluation summary
  if (fineGrainEvalCounters) {
    size_t i, num_fns = std::min(fnValCounter.size(), fnLabels.size());
    for (i=0; i<num_fns; i++) {
      int t_v = (relative_count) ?     fnValCounter[i] -     fnValRefPt[i]
                                 :     fnValCounter[i];
      int n_v = (relative_count) ?  newFnValCounter[i] -  newFnValRefPt[i]
                                 :  newFnValCounter[i];
      int t_g = (relative_count) ?    fnGradCounter[i] -    fnGradRefPt[i]
                                 :    fnGradCounter[i];
      int n_g = (relative_count) ? newFnGradCounter[i] - newFnGradRefPt[i]
                                 : newFnGradCounter[i];
      int t_h = (relative_count) ?    fnHessCounter[i] -    fnHessRefPt[i]
                                 :    fnHessCounter[i];
      int n_h = (relative_count) ? newFnHessCounter[i] - newFnHessRefPt[i]
                                 : newFnHessCounter[i];
      s << std::setw(15) << fnLabels[i] << ": "
        << t_v << " val ("  << n_v << " n, " << t_v - n_v << " d), "
        << t_g << " grad (" << n_g << " n, " << t_g - n_g << " d), "
        << t_h << " Hess (" << n_h << " n, " << t_h - n_h << " d)\n";
    }
  }
}


/// default implementation just sets the list of eval ID tags;
/// derived classes containing additional models or interfaces should
/// override (currently no use cases)
void Interface::
eval_tag_prefix(const String& eval_id_str, bool append_iface_id)
{
  evalTagPrefix = eval_id_str;
  appendIfaceId = append_iface_id;
}


void Interface::map(const Variables& vars, const ActiveSet& set,
		    Response& response, bool asynch_flag)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::
init_algebraic_mappings(const Variables& vars, const Response& response)
{
  size_t i, num_alg_vars = algebraicVarTags.size(),
    num_alg_fns = algebraicFnTags.size();

  algebraicACVIndices.resize(num_alg_vars);
  algebraicACVIds.resize(num_alg_vars);
  StringMultiArrayConstView acv_labels = vars.all_continuous_variable_labels();
  SizetMultiArrayConstView  acv_ids    = vars.all_continuous_variable_ids();
  for (i=0; i<num_alg_vars; ++i) {
    // Note: variable mappings only support continuous variables.
    //       discrete variables are not directly supported by ASL interface.
    size_t acv_index = find_index(acv_labels, algebraicVarTags[i]);
    //size_t adv_index = find_index(adv_labels, algebraicVarTags[i]);
    if (acv_index == _NPOS) { // && adv_index == _NPOS) {
      Cerr << "\nError: AMPL column label " << algebraicVarTags[i] << " does "
	   <<"not exist in Dakota continuous variable descriptors.\n"
	   << std::endl;
      abort_handler(INTERFACE_ERROR);
    }
    else {
      algebraicACVIndices[i] = acv_index;
      //algebraicADVIndices[i] = adv_index;
      algebraicACVIds[i] = acv_ids[acv_index];
    }
  }

  algebraicFnIndices.resize(num_alg_fns);
  const StringArray& fn_labels = response.function_labels();
  for (size_t i=0; i<num_alg_fns; ++i) {
    size_t fn_index = Pecos::find_index(fn_labels, algebraicFnTags[i]);
    if (fn_index == _NPOS) {
      Cerr << "\nError: AMPL row label " << algebraicFnTags[i] << " does not "
	   <<"exist in Dakota response descriptors.\n" << std::endl;
      abort_handler(INTERFACE_ERROR);
    }
    else
      algebraicFnIndices[i] = fn_index;
  }
}


void Interface::
asv_mapping(const ActiveSet& total_set, ActiveSet& algebraic_set,
	    ActiveSet& core_set)
{
  const ShortArray& total_asv = total_set.request_vector();
  const SizetArray& total_dvv = total_set.derivative_vector();

  // algebraic_asv/dvv:

  // the algebraic active set is defined over reduced algebraic function
  // and variable spaces, rather than the original spaces.  This simplifies
  // algebraic_mappings() and allows direct copies of data from AMPL.
  size_t i, num_alg_fns = algebraicFnTags.size(),
    num_alg_vars = algebraicVarTags.size();
  ShortArray algebraic_asv(num_alg_fns);
  SizetArray algebraic_dvv(num_alg_vars);
  for (i=0; i<num_alg_fns; i++) // map total_asv to algebraic_asv
    algebraic_asv[i] = total_asv[algebraicFnIndices[i]];

  algebraic_set.request_vector(algebraic_asv);
  algebraic_set.derivative_vector(algebraic_dvv);
  algebraic_set.derivative_start_value(1);

  // core_asv/dvv:

  // for now, core_asv is the same as total_asv, since there is no mechanism
  // yet to determine if the algebraic_mapping portion is the complete
  // definition (for which core_asv requests could be turned off).
  core_set.request_vector(total_asv);
  core_set.derivative_vector(total_dvv);
}


void Interface::
asv_mapping(const ActiveSet& algebraic_set, ActiveSet& total_set)
{
  const ShortArray& algebraic_asv = algebraic_set.request_vector();
  size_t i, num_alg_fns = algebraicFnTags.size();
  for (i=0; i<num_alg_fns; i++) // map algebraic_asv to total_asv
    total_set.request_value(algebraic_asv[i], algebraicFnIndices[i]);
}


void Interface::
algebraic_mappings(const Variables& vars, const ActiveSet& algebraic_set,
		   Response& algebraic_response)
{
#ifdef HAVE_AMPL
  // make sure cur_ASL is pointing to the ASL of this interface
  // this is important for problems with multiple interfaces
  set_cur_ASL(asl);
  const ShortArray& algebraic_asv = algebraic_set.request_vector();
  const SizetArray& algebraic_dvv = algebraic_set.derivative_vector();
  size_t i, num_alg_fns = algebraic_asv.size(),
    num_alg_vars = algebraic_dvv.size();
  bool grad_flag = false, hess_flag = false;
  for (i=0; i<num_alg_fns; ++i) {
    if (algebraic_asv[i] & 2)
      grad_flag = true;
    if (algebraic_asv[i] & 4)
      hess_flag = true;
  }

  // dak_a_c_vars (DAKOTA space) -> nl_vars (reduced AMPL space)
  const RealVector& dak_a_c_vars = vars.all_continuous_variables();
  //IntVector  dak_a_d_vars = vars.all_discrete_variables();
  Real* nl_vars = new Real [num_alg_vars];
  for (i=0; i<num_alg_vars; i++)
    nl_vars[i] = dak_a_c_vars[algebraicACVIndices[i]];

  // nl_vars -> algebraic_response
  algebraic_response.reset_inactive(); // zero inactive data
  Real fn_val; RealVector fn_grad; RealSymMatrix fn_hess;
  fint err = 0;
  for (i=0; i<num_alg_fns; i++) {
    // nl_vars -> response fns via AMPL
    if (algebraic_asv[i] & 1) {
      if (algebraicFnTypes[i] > 0)
	fn_val = objval(algebraicFnTypes[i]-1, nl_vars, &err);
      else
	fn_val = conival(-1-algebraicFnTypes[i], nl_vars, &err);
      if (err) {
	Cerr << "\nError: AMPL processing failure in objval().\n" << std::endl;
	abort_handler(INTERFACE_ERROR);
      }
      algebraic_response.function_value(fn_val, i);
    }
    // nl_vars -> response grads via AMPL
    if (algebraic_asv[i] & 6) { // need grad for Hessian
      fn_grad = algebraic_response.function_gradient_view(i);
      if (algebraicFnTypes[i] > 0)
	objgrd(algebraicFnTypes[i]-1, nl_vars, fn_grad.values(), &err);
      else
	congrd(-1-algebraicFnTypes[i], nl_vars, fn_grad.values(), &err);
      if (err) {
	Cerr << "\nError: AMPL processing failure in objgrad().\n" << std::endl;
	abort_handler(INTERFACE_ERROR);
      }
    }
    // nl_vars -> response Hessians via AMPL
    if (algebraic_asv[i] & 4) {
      fn_hess = algebraic_response.function_hessian_view(i);
      // the fullhess calls must follow corresp call to objgrad/congrad
      if (algebraicFnTypes[i] > 0)
	fullhes(fn_hess.values(), num_alg_vars, algebraicFnTypes[i]-1,
		NULL, NULL);
      else {
	algebraicConstraintWeights.assign(algebraicConstraintWeights.size(), 0);
	algebraicConstraintWeights[-1-algebraicFnTypes[i]] = 1;
	fullhes(fn_hess.values(), num_alg_vars, num_alg_vars, NULL, 
		&algebraicConstraintWeights[0]); 
      }
    }
  }
  delete [] nl_vars;
  algebraic_response.function_labels(algebraicFnTags);
#ifdef DEBUG
  Cout << ">>>>> algebraic_response.fn_labels\n"
       << algebraic_response.function_labels() << std::endl;
#endif // DEBUG

  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Algebraic mapping applied.\n";
#endif // HAVE_AMPL
}


/** This function will get invoked even when only algebraic mappings are 
    active (no core mappings from derived_map), since the AMPL 
    algebraic_response may be ordered differently from the total_response.
    In this case, the core_response object is unused. */
void Interface::
response_mapping(const Response& algebraic_response,
		 const Response& core_response, Response& total_response)
{
  const ShortArray& total_asv = total_response.active_set_request_vector();
  const SizetArray& total_dvv = total_response.active_set_derivative_vector();
  size_t i, j, k, num_total_fns = total_asv.size(),
    num_total_vars = total_dvv.size();
  bool grad_flag = false, hess_flag = false;
  for (i=0; i<num_total_fns; ++i) {
    if (total_asv[i] & 2)
      grad_flag = true;
    if (total_asv[i] & 4)
      hess_flag = true;
  }

  // core_response contributions to total_response:

  if (coreMappings) {
    total_response.reset_inactive();
    const ShortArray& core_asv = core_response.active_set_request_vector();
    size_t num_core_fns = core_asv.size();
    for (i=0; i<num_core_fns; ++i) {
      if (core_asv[i] & 1)
	total_response.function_value(core_response.function_value(i), i);
      if (core_asv[i] & 2)
	total_response.function_gradient(
	  core_response.function_gradient_view(i), i);
      if (core_asv[i] & 4)
	total_response.function_hessian(core_response.function_hessian(i), i);
    }
  }
  else {
    // zero all response data before adding algebraic data to it
    total_response.reset();
  }

  // algebraic_response contributions to total_response:

  const ShortArray& algebraic_asv
    = algebraic_response.active_set_request_vector();
  size_t num_alg_fns = algebraic_asv.size(),
    num_alg_vars = algebraic_response.active_set_derivative_vector().size();
  if (num_alg_fns > num_total_fns) {
    Cerr << "Error: response size mismatch in Interface::response_mapping()."
	 << std::endl;
    abort_handler(-1);
  }
  if ( (grad_flag || hess_flag) && num_alg_vars > num_total_vars) {
    Cerr << "Error: derivative variables size mismatch in Interface::"
         << "response_mapping()." << std::endl;
    abort_handler(-1);
  }
  SizetArray algebraic_dvv_indices;
  if (grad_flag || hess_flag) {
    algebraic_dvv_indices.resize(num_alg_vars);
    using Pecos::find_index;
    for (i=0; i<num_alg_vars; ++i)
      algebraic_dvv_indices[i] = find_index(total_dvv, algebraicACVIds[i]);
      // Note: _NPOS return is handled below
  }
  // augment total_response
  const RealVector& algebraic_fn_vals = algebraic_response.function_values();
  const RealMatrix& algebraic_fn_grads
    = algebraic_response.function_gradients();
  const RealSymMatrixArray& algebraic_fn_hessians
    = algebraic_response.function_hessians();
  RealVector total_fn_vals = total_response.function_values_view();
  for (i=0; i<num_alg_fns; ++i) {
    size_t fn_index = algebraicFnIndices[i];
    if (algebraic_asv[i] & 1)
      total_fn_vals[fn_index] += algebraic_fn_vals[i];
    if (algebraic_asv[i] & 2) {
      const Real* algebraic_fn_grad = algebraic_fn_grads[i];
      RealVector total_fn_grad
	= total_response.function_gradient_view(fn_index);
      for (j=0; j<num_alg_vars; j++) {
	size_t dvv_index = algebraic_dvv_indices[j];
	if (dvv_index != _NPOS)
	  total_fn_grad[dvv_index] += algebraic_fn_grad[j];
      }
    }
    if (algebraic_asv[i] & 4) {
      const RealSymMatrix& algebraic_fn_hess = algebraic_fn_hessians[i];
      RealSymMatrix total_fn_hess
	= total_response.function_hessian_view(fn_index);
      for (j=0; j<num_alg_vars; ++j) {
	size_t dvv_index_j = algebraic_dvv_indices[j];
	if (dvv_index_j != _NPOS) {
	  for (k=0; k<=j; ++k) {
	    size_t dvv_index_k = algebraic_dvv_indices[k];
	    if (dvv_index_k != _NPOS)
	      total_fn_hess(dvv_index_j,dvv_index_k) +=	algebraic_fn_hess(j,k);
	  }
	}
      }
    }
  }

  // output response sets:

  if (outputLevel == DEBUG_OUTPUT) {
    if (coreMappings) Cout << "core_response:\n" << core_response;
    Cout << "algebraic_response:\n" << algebraic_response
	 << "total_response:\n"     << total_response << '\n';
  }
}


String Interface::final_eval_id_tag(int iface_eval_id)
{
  if (appendIfaceId)
    return evalTagPrefix + "." + std::to_string(iface_eval_id);
  return evalTagPrefix;
}


int Interface::algebraic_function_type(String functionTag) 
{
#ifdef HAVE_AMPL
  int i;
  for (i=0; i<n_obj; i++)
    if (strcontains(functionTag, obj_name(i)))
      return i+1;
  for (i=0; i<n_con; i++)
    if (strcontains(functionTag, con_name(i)))
      return -(i+1);

  Cerr << "Error: No function type available for \'" << functionTag << "\' " 
       << "via algebraic_mappings interface." << std::endl;
  abort_handler(INTERFACE_ERROR);
  return 0; // does not get returned but quiets compiler warning
#else
  return 0;
#endif // HAVE_AMPL
}

const IntResponseMap& Interface::synchronize()
{
  return InterfaceUtils::no_derived_method_error<IntResponseMap>();
}


const IntResponseMap& Interface::synchronize_nowait()
{
  return InterfaceUtils::no_derived_method_error<IntResponseMap>();
}


void Interface::cache_unmatched_response(int raw_id)
{
  IntRespMIter rr_it = rawResponseMap.find(raw_id);
  if (rr_it != rawResponseMap.end()) {
    cachedResponseMap.insert(*rr_it);
    rawResponseMap.erase(rr_it);
  }
}


void Interface::cache_unmatched_responses()
{
  cachedResponseMap.insert(rawResponseMap.begin(), rawResponseMap.end());
  rawResponseMap.clear();
}


void Interface::serve_evaluations()
{
  Cerr << "Error: No derived serve_evaluations() method.\n"
       << "nNo default serve_evaluations defined at Interface"
       << " base class." << std::endl;
  abort_handler(-1);
}


void Interface::stop_evaluation_servers()
{
  Cerr << "Error: No derived stop_evaluation_servers() method.\n"
       << "No default stop_evaluation_servers defined at "
       << "Interface base class." << std::endl;
  abort_handler(-1);
}


void Interface::init_communicators(const IntArray& message_lengths,
				   int max_eval_concurrency)
{
  // ApproximationInterfaces: do nothing
}


void Interface::set_communicators(const IntArray& message_lengths,
				  int max_eval_concurrency)
{
  // ApproximationInterfaces: do nothing
}


void Interface::init_serial()
{
  // ApproximationInterfaces: do nothing
}


int Interface::asynch_local_evaluation_concurrency() const
{
  return 0; // default (redefined only for ApplicationInterfaces)
}


short Interface::interface_synchronization() const
{
  return SYNCHRONOUS_INTERFACE; // default (ApproximationInterfaces)
}


int Interface::minimum_points(bool constraint_flag) const
{
  return 0; // default (currently redefined only for ApproximationInterfaces)
}


int Interface::recommended_points(bool constraint_flag) const
{
  return 0; // default (currently redefined only for ApproximationInterfaces)
}


void Interface::active_model_key(const Pecos::ActiveKey& key)
{
  // default implementation is no-op
}


void Interface::clear_model_keys()
{
  // default implementation is no-op
}


void Interface::
approximation_function_indices(const SizetSet& approx_fn_indices)
{
  // default implementation is no-op
}


void Interface::
update_approximation(const Variables& vars, const IntResponsePair& response_pr)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::
update_approximation(const RealMatrix& samples, const IntResponseMap& resp_map)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::
update_approximation(const VariablesArray& vars_array,
		     const IntResponseMap& resp_map)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::
append_approximation(const Variables& vars, const IntResponsePair& response_pr)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::
append_approximation(const RealMatrix& samples, const IntResponseMap& resp_map)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::
append_approximation(const VariablesArray& vars_array,
		     const IntResponseMap& resp_map)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::
append_approximation(const IntVariablesMap& vars_map,
		     const IntResponseMap&  resp_map)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::
build_approximation(const RealVector&  c_l_bnds, const RealVector&  c_u_bnds,
		    const IntVector&  di_l_bnds, const IntVector&  di_u_bnds,
		    const RealVector& dr_l_bnds, const RealVector& dr_u_bnds)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::export_approximation()
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::rebuild_approximation(const BitArray& rebuild_fns)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::replace_approximation(const IntResponsePair& response_pr)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::replace_approximation(const IntResponseMap& resp_map)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::track_evaluation_ids(bool track)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::pop_approximation(bool save_data)
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::push_approximation()
{
  InterfaceUtils::no_derived_method_error();
}


bool Interface::push_available()
{
  return InterfaceUtils::no_derived_method_error<bool>();
}


void Interface::finalize_approximation()
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::clear_inactive()
{
  //  default: no inactive data to clear
}


void Interface::combine_approximation()
{
  InterfaceUtils::no_derived_method_error();
}


void Interface::combined_to_active(bool clear_combined)
{
  InterfaceUtils::no_derived_method_error();
}


bool Interface::advancement_available()
{
  return true; // only a few cases throttle advancements
}


bool Interface::formulation_updated() const
{
  return InterfaceUtils::no_derived_method_error<bool>();
}


void Interface::formulation_updated(bool update)
{
  InterfaceUtils::no_derived_method_error();
}


Real2DArray Interface::
cv_diagnostics(const StringArray& metric_types, unsigned num_folds)
{
  return InterfaceUtils::no_derived_method_error<Real2DArray>();
}


RealArray Interface::challenge_diagnostics(const String& metric_type,
					    const RealMatrix& challenge_pts)
{
  return InterfaceUtils::no_derived_method_error<RealArray>();
}


void Interface::clear_current_active_data()
{
  // ApplicationInterfaces: do nothing
}


void Interface::clear_active_data()
{
  // ApplicationInterfaces: do nothing
}


SharedApproxData& Interface::shared_approximation()
{
  Cerr << "Error: No derived " << __func__ << " method.\n"
       << "No default at Interface" << " base class." << std::endl;
  abort_handler(-1);

  throw INTERFACE_ERROR;
}


std::vector<Approximation>& Interface::approximations()
{
  InterfaceUtils::no_derived_method_error();
  //return InterfaceUtils::no_derived_method_error<std::vector<Approximation>>();
  throw INTERFACE_ERROR;
}


const Pecos::SurrogateData& Interface::approximation_data(size_t fn_index)
{
  Cerr << "Error: No derived " << __func__ << " method.\n"
       << "No default at Interface" << " base class." << std::endl;
  abort_handler(-1);

  throw INTERFACE_ERROR;
}


const RealVectorArray& Interface::approximation_coefficients(bool normalized)
{
  return InterfaceUtils::no_derived_method_error<RealVectorArray>();
}


void Interface::
approximation_coefficients(const RealVectorArray& approx_coeffs,
			   bool normalized)
{
  InterfaceUtils::no_derived_method_error();
}


const RealVector& Interface::approximation_variances(const Variables& vars)
{
  return InterfaceUtils::no_derived_method_error<RealVector>();
}


const StringArray& Interface::analysis_drivers() const
{
  return InterfaceUtils::no_derived_method_error<StringArray>();
}


const String2DArray& Interface::analysis_components() const
{
  return analysisComponents;
}


void Interface::discrepancy_emulation_mode(short mode)
{
  // no-op
}


bool Interface::evaluation_cache() const
{
  return false; // default
}


bool Interface::restart_file() const
{
  return false; // default
}


void Interface::file_cleanup() const
{
  // no-op
}

/** Rationale: The parser allows multiple user-specified interfaces with
    empty (unspecified) ID. However, only a single Interface with empty
    ID can be constructed (if it's the only one present, or the "last
    one parsed"). Therefore decided to prefer NO_ID over NO_ID_<num>
    for consistency with interface NO_ID convention. Additionally, NO_ID
    is preferred over NO_INTERFACE_ID (contrast with Iterator and Model)
    to preserve backward compatibility
 */
String Interface::user_auto_id()
{
  // // increment and then use the current ID value
  // return String("NO_ID_") + std::to_string(++userAutoIdNum);
  return String("NO_ID");
}

/** Rationale: For now NOSPEC_ID_ is chosen due to historical
    id="NO_SPECIFICATION" used for internally-constructed
    Iterators. Longer-term, consider auto-generating an ID that
    includes the context from which the method is constructed, e.g.,
    the parent method or model's ID, together with its name. */
String Interface::no_spec_id()
{
  // increment and then use the current ID value
  return String("NOSPEC_INTERFACE_ID_") + std::to_string(++noSpecIdNum);
}
} // namespace Dakota
