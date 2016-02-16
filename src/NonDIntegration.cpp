/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDIntegration
//- Description: Implementation code for NonDIntegration class
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "NonDIntegration.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: NonDIntegration.cpp,v 1.57 2004/06/21 19:57:32 mseldre Exp $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called
    and probDescDB can be queried for settings from the method
    specification.  It is not currently used, as there are not yet
    separate nond_quadrature/nond_sparse_grid method specifications. */
NonDIntegration::NonDIntegration(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model), numIntegrations(0), sequenceIndex(0),
  dimPrefSpec(probDescDB.get_rv("method.nond.dimension_preference"))
  //, standAloneMode(true)
{
  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars) {
    Cerr << "\nError: discrete random variables are not currently supported in "
	 << "NonDIntegration." << std::endl;
    abort_handler(-1);
  }

  initialize_random_variable_transformation();
  initialize_random_variable_types(EXTENDED_U);
  // Note: initialize_random_variable_parameters() is performed at run time
  initialize_random_variable_correlations();
  verify_correlation_support(EXTENDED_U);
  initialize_final_statistics(); // default statistics set
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical integration points. */
NonDIntegration::NonDIntegration(unsigned short method_name, Model& model): 
  NonD(method_name, model), numIntegrations(0), sequenceIndex(0)
  //, standAloneMode(false)
{
  // The passed model (stored in iteratedModel) is G(u): it is recast to
  // standard space and does not include a DataFit recursion.

  // initialize_random_variables(natafTransform) is called externally (e.g.,
  // NonDExpansion::initialize_u_space_model()) to allow access to data from
  // outer context.
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical integration points. */
NonDIntegration::
NonDIntegration(unsigned short method_name, Model& model,
		const RealVector& dim_pref): 
  NonD(method_name, model), numIntegrations(0), sequenceIndex(0),
  dimPrefSpec(dim_pref) //, standAloneMode(false)
{
  // The passed model (stored in iteratedModel) is G(u): it is recast to
  // standard space and does not include a DataFit recursion.

  // initialize_random_variables(natafTransform) is called externally (e.g.,
  // NonDExpansion::initialize_u_space_model()) to allow access to data from
  // outer context.
}


NonDIntegration::~NonDIntegration()
{ }

bool NonDIntegration::resize()
{
  bool parent_reinit_comms = NonD::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDIntegration::core_run()
{
  //if (standAloneMode)
  //  initialize_random_variable_parameters(); // capture any dist param updates
  //else
  check_variables(natafTransform.x_random_variables());//deferred from alt ctors

  // generate integration points
  get_parameter_sets(iteratedModel);

  // convenience function from Analyzer for evaluating parameter sets.  Data
  // flags are set to be compatible with DataFitSurrModel::build_global().
  evaluate_parameter_sets(iteratedModel, true, false);

  // Needed for general use outside of NonDPolynomialChaos
  //if (standAloneMode)
  //  evaluate_integrals();

  ++numIntegrations;
}


/** Virtual function called from probDescDB-based constructors and from
    NonDIntegration::core_run() */
void NonDIntegration::
check_variables(const std::vector<Pecos::RandomVariable>& x_ran_vars)
{
  // base class default definition of virtual function
  bool err_flag = false;

  numContDesVars = numContIntervalVars = numContStateVars = 0;
  size_t i, num_v = x_ran_vars.size(); short x_type;
  for (i=0; i<num_v; ++i) {
    x_type = x_ran_vars[i].type();
    if      (x_type == Pecos::CONTINUOUS_DESIGN)   ++numContDesVars;
    else if (x_type == Pecos::CONTINUOUS_INTERVAL) ++numContIntervalVars;
    else if (x_type == Pecos::CONTINUOUS_STATE)    ++numContStateVars;
  }

  if (x_ran_vars.size()   != numContinuousVars   ||
      numContEpistUncVars != numContIntervalVars ||
      numContinuousVars   != numContDesVars      + numContAleatUncVars +
                             numContEpistUncVars + numContStateVars) {
    Cerr << "Error: mismatch in active variable counts in NonDIntegration::"
	 << "check_variables()." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);
}


void NonDIntegration::print_points_weights(const String& tabular_name)
{
  size_t i, num_pts = allSamples.numCols(), num_vars = allSamples.numRows();
  if (num_pts && num_vars) {
    std::ofstream pts_wts_file(tabular_name.c_str());
    const RealVector& t1_wts = numIntDriver.type1_weight_sets();
    bool weights = (t1_wts.length() > 0);
    pts_wts_file << std::setprecision(write_precision) 
		 << std::resetiosflags(std::ios::floatfield) << "%   id ";
    if (weights)
      pts_wts_file << std::setw(write_precision+6) << "weight ";
    write_data_tabular(pts_wts_file,
		       iteratedModel.continuous_variable_labels());
    pts_wts_file << '\n';
    for (i=0; i<num_pts; ++i) {
      pts_wts_file << std::setw(6) << i+1 << ' ';
      if (weights)
	pts_wts_file << std::setw(write_precision+5) << t1_wts[i] << ' ';
      write_data_tabular(pts_wts_file, allSamples[i], num_vars);
      pts_wts_file << '\n';
    }
  }
}


/** Converts a scalar order specification and a vector anisotropic
    dimension preference into an anisotropic order vector.  It is used
    for initialization and does not enforce a reference lower bound
    (see also NonDQuadrature::update_anisotropic_order()). */
void NonDIntegration::
dimension_preference_to_anisotropic_order(unsigned short scalar_order_spec,
					  const RealVector& dim_pref_spec,
					  size_t num_v,
					  UShortArray& aniso_order)
{
  // Note: this fn is the inverse of anisotropic_order_to_dimension_preference()

  if (dim_pref_spec.empty()) {
    aniso_order.assign(num_v, scalar_order_spec);
    return;
  }

  Real max_dim_pref = dim_pref_spec[0];
  size_t i, max_dim_pref_index = 0;
  for (i=1; i<num_v; ++i)
    if (dim_pref_spec[i] > max_dim_pref)
      { max_dim_pref = dim_pref_spec[i]; max_dim_pref_index = i; }

  aniso_order.resize(num_v);
  for (i=0; i<num_v; ++i)
    aniso_order[i] = (i == max_dim_pref_index) ? scalar_order_spec :
      (unsigned short)(scalar_order_spec * dim_pref_spec[i] / max_dim_pref);
      // truncates fractional order
}


/** Converts a vector anisotropic order into a scalar order and vector
    anisotropic dimension preference. */
void NonDIntegration::
anisotropic_order_to_dimension_preference(const UShortArray& aniso_order,
					  unsigned short& scalar_order,
					  RealVector& dim_pref)
{
  // Note: this fn is the inverse of dimension_preference_to_anisotropic_order()

  scalar_order = aniso_order[0];
  size_t i, num_v = aniso_order.size(); bool anisotropic = false;
  for (i=1; i<num_v; ++i)
    if (aniso_order[i] > scalar_order)
      { scalar_order = aniso_order[i]; anisotropic = true; }

  if (anisotropic) { // preserve ratios; normalization not required
    dim_pref.sizeUninitialized(num_v);
    for (i=0; i<num_v; ++i)
      dim_pref[i] = (Real)aniso_order[i];
  }
  else
    dim_pref.sizeUninitialized(0);
}


void NonDIntegration::increment_grid_preference(const RealVector& dim_pref)
{
  // derived classes must provide at least one of increment_grid_preference()
  // or increment_grid_weights(), but need not provide both.  Therefore, the
  // default base class implementation of increment_grid_preference(pref) is
  // to convert pref to wts and invoke increment_grid_weights(wts)

  size_t i, num_pref = dim_pref.length();
  RealVector aniso_wts(num_pref);
  for (i=0; i<num_pref; ++i)
    aniso_wts[i] = 1./dim_pref[i];
  increment_grid_weights(aniso_wts);
}


void NonDIntegration::increment_grid_weights(const RealVector& aniso_wts)
{
  // derived classes must provide at least one of increment_grid_preference()
  // or increment_grid_weights(), but need not provide both.  Therefore, the
  // default base class implementation of increment_grid_weights(wts) is to
  // convert wts to pref and invoke increment_grid_preference(pref)

  size_t i, num_wts = aniso_wts.length();
  RealVector dim_pref(num_wts);
  for (i=0; i<num_wts; ++i)
    dim_pref[i] = 1./aniso_wts[i];
  increment_grid_preference(dim_pref);
}


void NonDIntegration::increment_specification_sequence()
{ ++sequenceIndex; } // default overridden by NonD{Quadrature,SparseGrid}

} // namespace Dakota
