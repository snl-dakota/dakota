/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultilevelSampling
//- Description: Implementation code for NonDMultilevelSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDMultilevelSampling.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: NonDMultilevelSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMultilevelSampling::
NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model)
{ }


NonDMultilevelSampling::~NonDMultilevelSampling()
{ }


void NonDMultilevelSampling::pre_run()
{
  // run LHS to generate parameter sets
  get_parameter_sets(iteratedModel);
}


/** Loop over the set of samples and compute responses.  Compute
    statistics on the set of responses if statsFlag is set. */
void NonDMultilevelSampling::quantify_uncertainty()
{
  // identical to NonDSampling::quantify_uncertainty() for now...
  bool log_resp_flag = (allDataFlag || statsFlag), log_best_flag = false;
  evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
}


void NonDMultilevelSampling::post_run(std::ostream& s)
{
  // Statistics are generated here and output in print_results() below
  if (statsFlag) // calculate statistics on allResponses
    compute_statistics(allSamples, allResponses);

  Analyzer::post_run(s);
}


void NonDMultilevelSampling::print_results(std::ostream& s)
{
  if (statsFlag) {
    s << "\nStatistics based on " << numSamples << " samples:\n";
    print_statistics(s);
  }
}

} // namespace Dakota
