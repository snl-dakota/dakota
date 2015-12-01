/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "EfficientSubspaceMethod.hpp"
#include "NonDLHSSampling.hpp"
#include "ActiveSubspaceModel.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"

namespace Dakota {

EfficientSubspaceMethod::
EfficientSubspaceMethod(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model),
  seedSpec(probDescDB.get_int("method.random_seed")),
  initialSamples(probDescDB.get_int("method.samples")),    // default 0
  batchSize(probDescDB.get_int("method.nond.batch_size")), // default 0
  subspaceSamples(probDescDB.get_int("method.nond.emulator_samples")), // def 0
  transformVars(true)
 
{
  // the Iterator initializes:
  //   maxIterations    (default -1)
  //   convergenceTol   (default 1.0e-4); tolerance before checking recon error
  //   maxFunctionEvals (default 1000)
  validate_inputs();

  if (transformVars) {
    // Initialize transformation:
    initialize_random_variable_transformation();
    initialize_random_variable_types(STD_NORMAL_U);
    initialize_random_variable_parameters(); // TODO Move to runtime
    initialize_random_variable_correlations();
    //verify_correlation_support(STD_NORMAL_U);

    transform_model(iteratedModel, fullSpaceModel); // retain orig distrib bnds
  }
  else {
    fullSpaceModel = iteratedModel;
  }

  // would be useful to initialize the sub-iterator here for setting
  // concurrency, but don't know the variable sizes yet
  // init_reduced_sampler(subspaceSamples);

  // The same model is used in multiple contexts, with varying degrees
  // of concurrency: initialSamples, batchSize, verif_samples, and
  // finally subspaceSamples.  For now, we configure max eval
  // concurrency for the initialSamples and allow batches in
  // subsequent phases; could instead take max of all these if
  // desired.

  // The inbound model concurrency accounts for any finite differences,
  // update with the reduced space method's concurency now:
  maxEvalConcurrency *= subspaceSamples;
}


EfficientSubspaceMethod::~EfficientSubspaceMethod()
{
  /* empty destructor */
}


void EfficientSubspaceMethod::quantify_uncertainty()
{
  // ----
  // Instantiate the DACE iterator, using default RNG and samples
  // ----
  int mc_seed = (seedSpec) ? seedSpec : generate_system_seed();
  if (seedSpec)
    Cout << "ESM: build seed (user-specified) = " << mc_seed << std::endl;
  else
    Cout << "ESM: build seed (system-generated) = " << mc_seed << std::endl;

  // until we add to the Model constructor chain...
  ActiveSubspaceModel* as_model = new ActiveSubspaceModel
    (iteratedModel, mc_seed, initialSamples, batchSize, convergenceTol, 
     maxIterations, maxFunctionEvals);
  Model subspace_model;
  subspace_model.assign_rep(as_model, false);

  // This is constructed late, so hasn't had init comms called
  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  subspace_model.init_communicators(pl_iter, maxEvalConcurrency);
  subspace_model.set_communicators(pl_iter, maxEvalConcurrency);

  // build the reduced space model; this must occur at runtime as it
  // requires model evaluations
  as_model->initialize();

  // perform the reduced space UQ
  Cout << "\nESM: Performing reduced-space UQ" << std::endl;
  reduced_space_uq(subspace_model);
  // Iterator subspace_iterator(subspace_model);
  // subspace_iterator.run();
  Cout << "\nESM: Reduced-space UQ complete" << std::endl;

  subspace_model.free_communicators(pl_iter, maxEvalConcurrency);
}


void EfficientSubspaceMethod::validate_inputs()
{
  bool error_flag = false;

  // validate iteration controls

  // set default initialSamples, with lower bound of 2
  // TODO: allow other user control of initial sample rule?
  if (initialSamples <= 0) {
    initialSamples =
      (unsigned int) std::ceil( (double) numContinuousVars / 100.0 );
    initialSamples = std::max(2, initialSamples);
    Cout << "\nInfo: Efficient subspace method setting (initial) samples = "
         << initialSamples << "." << std::endl;
  }
  else if (initialSamples < 2) {
    initialSamples = 2;
    Cout << "\nWarning: Efficient subspace method resetting samples to minimum "
         << "allowed = " << initialSamples << "." << std::endl;
  }

  if (initialSamples > maxFunctionEvals) {
    error_flag = true;
    Cerr << "\nError: Efficient subspace method build samples exceeds function "
         << "budget." << std::endl;
  }

  if (batchSize <= 0) {
    // default is to add one point at a time
    batchSize = 1;
  }
  else if (batchSize > initialSamples) {
    Cout << "\nWarning: batch_size = " << batchSize << " exceeds (initial) "
         << "samples = " << initialSamples << ";\n        resetting batch_size "
         << "= " << initialSamples << "." << std::endl;
    batchSize = initialSamples;
  }

  // maxIterations controls the number of build iterations
  if (maxIterations < 0) {
    maxIterations = 1;
    Cout << "\nInfo: Efficient subspace method setting max_iterations = "
         << maxIterations << "." << std::endl;
  }

  // emulator samples don't count toward the sample budget
  if (subspaceSamples <= 0) {
    subspaceSamples = 10*initialSamples;
    Cout << "\nInfo: Efficient subspace method setting emulator_samples = "
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

  // validate response data
  if (fullSpaceModel.gradient_type() == "none") {
    error_flag = true;
    Cerr << "\nError: Efficient subspace method requires gradients.\n"
         << "       Please select numerical, analytic (recommended), or mixed "
         << "gradients." << std::endl;
  }

  if (error_flag)
    abort_handler(-1);
}


/**  This specialization is because the model is used in multiple
     contexts in this iterator, depending on build phase.  Note that
     this overrides the default behavior at Iterator which recurses
     into any submodels. */
// void EfficientSubspaceMethod::derived_init_communicators(ParLevLIter pl_iter)
// {
//   // TODO: once we construct the sub-iterators earlier, init them here.
// }


// Unnecessary due to run(pl_iter) which invokes set_communicators on
// fullSpaceSampler
void EfficientSubspaceMethod::derived_set_communicators(ParLevLIter pl_iter)
{
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);
  //  fullSpaceSampler.set_communicators(pl_iter);
}

// void EfficientSubspaceMethod::derived_free_communicators(ParLevLIter pl_iter)
// {
//   // TODO: once we construct the sub-iterators earlier, free them here.
// }



/** This function is experimental and needs to be reviewed and cleaned
    up.  In particular the translation of the correlations from full
    to reduced space is likely wrong.  Transformation may be correct
    for covariance, but likely not correlations. */
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

  // BMA: Is this needed? Isn't the passed model already in u-space?
  if (transformVars)
    lhs_rep->initialize_random_variables(natafTransform); // shallow copy

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
