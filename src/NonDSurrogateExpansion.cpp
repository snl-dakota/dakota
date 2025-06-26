/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "NonDSurrogateExpansion.hpp"

#include "DakotaResponse.hpp"
#include "DataFitSurrModel.hpp"
#include "ProbabilityTransformModel.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_system_defs.hpp"
#include "dakota_tabular_io.hpp"
// #define DEBUG

namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDSurrogateExpansion::NonDSurrogateExpansion(ProblemDescDB& problem_db,
                                               ParallelLibrary& parallel_lib,
                                               std::shared_ptr<Model> model)
    : NonDExpansion(problem_db, parallel_lib, model)
// base classes extract generic UQ spec for stochastic expansions
{
  // ----------------
  // Resolve settings
  // ----------------
  // short data_order,
  //  u_space_type = probDescDB.get_short("method.nond.expansion_type");
  // resolve_inputs(u_space_type, data_order);

  if (iteratedModel->model_type() != "surrogate") {
    Cerr << "Error: NonDSurrogateExpansion requires a surrogate model "
         << "specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  const String& surr_type = iteratedModel->surrogate_type();
  if (surr_type == "global_function_train") {  // || surr_type == "global_"
    // transformation, DataFit, and DACE configuration performed by Model spec
    // All expansion config settings are pulled in that ctor chain

    uSpaceModel = std::static_pointer_cast<DataFitSurrModel>(iteratedModel);

    // Notes on managing the u-space transformation:
    // > wrapping iteratedModel here applies the transformation on top of the
    //   incoming DataFitSurrModel --> insufficient for internal build.
    //     uSpaceModel->assign_rep(new ProbabilityTransformModel(iteratedModel,
    //       u_space_type), false); // only affects exp_sampler
    // > modifying the DataFitSurrModel ctor requires care because the
    //   daceIterator spec points to the actualModel spec (when DACE is active)
    //   and daceIterator should sample in u-space for a u-space approx.  This
    //   requires recasting + reinserting the model + re-initializing DACE (no
    //   thanks), or instantiating the model first (using truth_model_pointer
    //   or model_pointer from DACE spec) + recasting + instantiating DACE on
    //   recast (similar to lightwt DataFitSurrModel ctor used by PCE/SC).
    //   >> either an option in existing DataFitSurrModel or a specialization
    //      in new ProbTransDataFitSurrModel sub-class?
    // > trigger recursion based on random variable "standardize" spec, similar
    //   to variable/objective/constraint scaling --> ScalingModel in Minimizer
    //   >> Problem: don't want to recast every model that includes that vars
    //      spec, as it is often shared throughout a recursion.
    //   >> Possible soln: similar to Minimizer scaling routines triggered
    //      from variables/responses spec, restrict recasting logic to DFS?
    //      >>> might be useful to support prob transforms without DFS...
    //      >>> first model that can performs the prob transform and "consumes"
    //          the standardization -> Models above it only see transformed
    //          random vars + no standardize request (or recognize no-op for a
    //          persistent request--> no further standardization to perform)
    //   >> Some methods/models require standardization and override default
    // > For now, define logic in DFSModel ctor for data fits hard-wired with
    //   (orthog,interp,fn_train) and without (everything else) standardization
    //   >> allows pathway to specification-based {orthog,interp} as well

    // publish random variable types
    // TO DO: needs to be integrated within Model-driven workflow
    // initialize_data_fit_surrogate(iteratedModel);
  } else {
    Cerr << "Error: surrogate model specification of type '" << surr_type
         << "' not supported in NonDSurrogateExpansion." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // -------------------------------
  // Construct expSampler, if needed
  // -------------------------------
  construct_expansion_sampler(
      problem_db.get_ushort("method.sample_type"),
      problem_db.get_string("method.random_number_generator"),
      problem_db.get_ushort("method.nond.integration_refinement"),
      problem_db.get_iv("method.nond.refinement_samples"),
      probDescDB.get_string("method.import_approx_points_file"),
      probDescDB.get_ushort("method.import_approx_format"),
      probDescDB.get_bool("method.import_approx_active_only"));

  // update concurrency
  // if (numSamples) // samples is optional (default = 0)
  //  maxEvalConcurrency *= numSamples;
}

NonDSurrogateExpansion::~NonDSurrogateExpansion() {}

/* Base class implementation is sufficient for single-level
void NonDSurrogateExpansion::core_run()
{
  // TO DO: detect and execute a generic MLMF UQ workflow
  if (detect_mf())
    multifidelity_expansion();
  else if (detect_ml_regress())
    multilevel_regression();
  else
    NonDExpasion::core_run();
}
*/

void NonDSurrogateExpansion::print_results(std::ostream& s) {
  if (  // iteratedModel.subordinate_models(false).size() == 1 &&
      iteratedModel->truth_model()->solution_levels() > 1) {
    s << "<<<<< Samples per solution level:\n";
    print_multilevel_evaluation_summary(s, NLev);
    s << "<<<<< Equivalent number of high fidelity evaluations: "
      << equivHFEvals << std::endl;
  }

  NonDExpansion::print_results(s);
}

}  // namespace Dakota
