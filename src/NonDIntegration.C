/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2001, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDIntegration
//- Description: Implementation code for NonDIntegration class
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#include "data_types.h"
#include "system_defs.h"
#include "NonDIntegration.H"
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "ProblemDescDB.H"

static const char rcsId[]="@(#) $Id: NonDIntegration.C,v 1.57 2004/06/21 19:57:32 mseldre Exp $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called
    and probDescDB can be queried for settings from the method
    specification.  It is not currently used, as there are not yet
    separate nond_quadrature/nond_sparse_grid method specifications. */
NonDIntegration::NonDIntegration(Model& model): NonD(model), numIntegrations(0)
  //, standAloneMode(true)
{
  initialize_random_variable_types(EXTENDED_U);
  //natafTransform.initialize_random_variable_correlations(
  //  model.uncertain_correlations());

  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteRealVars) {
    Cerr << "\nError: discrete random variables are not currently supported in "
	 << "NonDIntegration." << std::endl;
    abort_handler(-1);
  }
}


/** This alternate constructor is used for on-the-fly generation and
    evaluation of numerical integration points. */
NonDIntegration::NonDIntegration(NoDBBaseConstructor, Model& model): 
  NonD(NoDBBaseConstructor(), model), numIntegrations(0)
  //, standAloneMode(false)
{
  // The passed model (stored in iteratedModel) is G(u): it is recast to
  // standard space and does not include a DataFit recursion.

  // initialize_random_variables(natafTransform) is called externally (e.g.,
  // NonDExpansion::initialize_expansion()) and passed data from outer context.
}


NonDIntegration::~NonDIntegration()
{ }


void NonDIntegration::quantify_uncertainty()
{
  //if (standAloneMode)
  //  initialize_random_variable_parameters(); // capture any dist param updates
  //else
  check_variables(natafTransform.x_types()); // deferred from alternate ctor

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
    NonDIntegration::quantify_uncertainty() */
void NonDIntegration::check_variables(const Pecos::ShortArray& x_types)
{
  // base class default definition of virtual function
  bool err_flag = false;

  numContDesVars
    = std::count(x_types.begin(), x_types.end(), (short)Pecos::DESIGN);
  numContStateVars
    = std::count(x_types.begin(), x_types.end(), (short)Pecos::STATE);

  if (x_types.size()    != numContinuousVars ||
      numContinuousVars != numContDesVars      + numContAleatUncVars +
                           numContEpistUncVars + numContStateVars) {
    Cerr << "Error: mismatch in active variable counts in NonDIntegration::"
	 << "check_variables()." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);
}

} // namespace Dakota
