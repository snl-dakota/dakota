/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "EfficientSubspaceMethod.hpp"
#include "ProbabilityTransformModel.hpp"
#include "NonDLHSSampling.hpp"
#include "ActiveSubspaceModel.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"

namespace Dakota {

EfficientSubspaceMethod::
EfficientSubspaceMethod(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model),
  seedSpec(probDescDB.get_int("method.random_seed")),
  subspaceSamples(probDescDB.get_int("method.samples")) // default 0
{
  // the Iterator initializes:
  //   maxIterations    (default -1)
  //   convergenceTol   (default 1.0e-4); tolerance before checking recon error
  //   maxFunctionEvals (default 1000)
  validate_inputs();

  // would be useful to initialize the sub-iterator here for setting
  // concurrency, but don't know the variable sizes yet
  // init_reduced_sampler(subspaceSamples);

  // Communicator management on the fullspace model is now handled in
  // ActiveSubspaceModel; only the default init/set/free are needed in
  // this Iterator, as they will setup iteratedModel with
  // maxEvalConcurrency
  maxEvalConcurrency *= subspaceSamples;
}


EfficientSubspaceMethod::~EfficientSubspaceMethod()
{
  /* empty destructor */
}


void EfficientSubspaceMethod::core_run()
{
  // ----
  // Instantiate the DACE iterator, using default RNG and samples
  // ----
  int mc_seed = (seedSpec) ? seedSpec : generate_system_seed();
  if (seedSpec)
    Cout << "ESM: build seed (user-specified) = " << mc_seed << std::endl;
  else
    Cout << "ESM: build seed (system-generated) = " << mc_seed << std::endl;

  // perform the reduced space UQ
  Cout << "\nESM: Performing reduced-space UQ" << std::endl;
  reduced_space_uq(iteratedModel);
  // Iterator subspace_iterator(subspace_model);
  // subspace_iterator.run();
  Cout << "\nESM: Reduced-space UQ complete" << std::endl;
}


void EfficientSubspaceMethod::validate_inputs()
{
  bool error_flag = false;

  if (iteratedModel.model_type() != "subspace") {
    Cerr << "\nError: ESM requires model of type subspace.\n";
    error_flag = true;
  }

  // validate iteration controls

  // emulator samples don't count toward the sample budget
  if (subspaceSamples <= 0) {
    subspaceSamples = 100;
    Cout << "\nInfo: Efficient subspace method setting samples = "
         << subspaceSamples << "\n      (10*samples specified)." << std::endl;
  }

  // validate variables specification
  if (numContinuousVars != numNormalVars
      || numDiscreteIntVars > 0 || numDiscreteRealVars > 0) {
    error_flag = true;
    Cerr << "\nError: Efficient subspace method only supports normal uncertain "
         << "variables;\n       remove other variable specifications."
         << std::endl;
  }

  if (error_flag)
    abort_handler(-1);
}


// void EfficientSubspaceMethod::derived_init_communicators(ParLevLIter pl_iter)
// {
//   // TODO: once we construct the sub-iterators earlier, init them here.
// }


void EfficientSubspaceMethod::derived_set_communicators(ParLevLIter pl_iter)
{
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);
}


// void EfficientSubspaceMethod::derived_free_communicators(ParLevLIter pl_iter)
// {
//   // TODO: once we construct the sub-iterators earlier, free them here.
// }


/** This function is experimental and needs to be reviewed and cleaned
    up. */
void EfficientSubspaceMethod::reduced_space_uq(Model& subspace_model)
{
  // TODO: additional UQ methods
  // construct an LHS sampler to work on the subspace model
  unsigned short sample_type = SUBMETHOD_RANDOM;
  int mc_seed = (seedSpec) ? seedSpec : generate_system_seed();
  if (seedSpec)
    Cout << "ESM: subspace seed (user-specified) = " << mc_seed << std::endl;
  else
    Cout << "ESM: subspace seed (system-generated) = " << mc_seed << std::endl;
  // might want true for multiple calls...
  bool vary_pattern = false;
  NonDLHSSampling* lhs_rep =
    new NonDLHSSampling(subspace_model, sample_type, subspaceSamples, mc_seed,
                        String(), vary_pattern);

  // transfer ownership of allocated memory to an Iterator envelope
  Iterator reduced_space_sampler;
  reduced_space_sampler.assign_rep(lhs_rep, false);

  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  reduced_space_sampler.init_communicators(pl_iter);

  bool all_data = true;
  bool gen_stats = true;
  reduced_space_sampler.sampling_reset(subspaceSamples, all_data, gen_stats);

  ActiveSet active_set = reduced_space_sampler.active_set(); // copy
  unsigned short request_value = 1;
  active_set.request_values(request_value);
  reduced_space_sampler.active_set(active_set);

  reduced_space_sampler.sub_iterator_flag(false);
  reduced_space_sampler.run(pl_iter);

  // reduced space UQ results
  Cout << " --- ESM: Results of reduced-space UQ --- \n";
  reduced_space_sampler.print_results(Cout);

  reduced_space_sampler.free_communicators(pl_iter);
}


} // namespace Dakota
