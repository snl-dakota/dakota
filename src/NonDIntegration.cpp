/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
  NonD(problem_db, model), numIntegrations(0),
  dimPrefSpec(probDescDB.get_rv("method.nond.dimension_preference"))
  //standAloneMode(true)
{
  /*
  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars) {
    Cerr << "\nError: discrete random variables are not currently supported in "
	 << "NonDIntegration." << std::endl;
    abort_handler(-1);
  }
  */

  initialize_final_statistics(); // default statistics set
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical integration points. */
NonDIntegration::NonDIntegration(unsigned short method_name, Model& model): 
  NonD(method_name, model), numIntegrations(0)//, standAloneMode(false)
{
  // The passed model (stored in iteratedModel) is G(u): it is recast to
  // standard space and does not include a DataFit recursion.
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical integration points. */
NonDIntegration::
NonDIntegration(unsigned short method_name, Model& model,
		const RealVector& dim_pref): 
  NonD(method_name, model), numIntegrations(0), dimPrefSpec(dim_pref)
  //standAloneMode(false)
{
  // The passed model (stored in iteratedModel) is G(u): it is recast to
  // standard space and does not include a DataFit recursion.
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
  //if (!standAloneMode)
  //  check_variables(x_dist.random_variables());//deferred from alt ctors

  // generate integration points
  get_parameter_sets(iteratedModel);

  // convenience function from Analyzer for evaluating parameter sets.  Data
  // flags are set to be compatible with DataFitSurrModel::build_global().
  evaluate_parameter_sets(iteratedModel);

  // Needed for general use outside of NonDPolynomialChaos
  //if (standAloneMode)
  //  evaluate_integrals();

  ++numIntegrations;
}


/** Virtual function called from probDescDB-based constructors and from
    NonDIntegration::core_run()
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
*/


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
		       ModelUtils::continuous_variable_labels(iteratedModel));
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


void NonDIntegration::evaluate_grid_increment()
{
  Cerr << "Error: derived class lacking redefinition for NonDIntegration::"
       << "evaluate_grid_increment()." << std::endl;
  abort_handler(METHOD_ERROR);
}


void NonDIntegration::push_grid_increment()
{
  Cerr << "Error: derived class lacking redefinition for NonDIntegration::"
       << "push_grid_increment()." << std::endl;
  abort_handler(METHOD_ERROR);
}


void NonDIntegration::pop_grid_increment()
{
  Cerr << "Error: derived class lacking redefinition for NonDIntegration::"
       << "pop_grid_increment()." << std::endl;
  abort_handler(METHOD_ERROR);
}


void NonDIntegration::merge_grid_increment()
{
  Cerr << "Error: derived class lacking redefinition for NonDIntegration::"
       << "merge_grid_increment()." << std::endl;
  abort_handler(METHOD_ERROR);
}


void NonDIntegration::update_reference()
{
  Cerr << "Error: derived class lacking redefinition for NonDIntegration::"
       << "update_reference()." << std::endl;
  abort_handler(METHOD_ERROR);
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
  increment_grid_weights(aniso_wts); // this virtual fn must be redefined
}


void NonDIntegration::increment_grid_preference()
{
  // derived classes must provide at least one of increment_grid_preference()
  // or increment_grid_weights(), but need not provide both.  Therefore, the
  // default base class implementation of increment_grid_preference(pref) is
  // to convert pref to wts and invoke increment_grid_weights(wts)

  increment_grid_weights(); // this virtual fn must be redefined
}


void NonDIntegration::increment_grid_weights(const RealVector& aniso_wts)
{
  // derived classes must provide at least one of increment_grid_preference()
  // or increment_grid_weights(), but need not provide both.  Therefore, the
  // default base class implementation of increment_grid_weights() is to
  // invoke increment_grid_preference()

  size_t i, num_wts = aniso_wts.length();
  RealVector dim_pref(num_wts);
  for (i=0; i<num_wts; ++i)
    dim_pref[i] = 1./aniso_wts[i];
  increment_grid_preference(dim_pref); // this virtual fn must be redefined
}


void NonDIntegration::increment_grid_weights()
{
  // derived classes must provide at least one of increment_grid_preference()
  // or increment_grid_weights(), but need not provide both.  Therefore, the
  // default base class implementation of increment_grid_weights() is to
  // invoke increment_grid_preference()

  increment_grid_preference(); // this virtual fn must be redefined
}

} // namespace Dakota
