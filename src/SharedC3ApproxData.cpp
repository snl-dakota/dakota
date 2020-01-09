/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedC3ApproxData
//- Description:  Implementation code for SharedC3ApproxData class
//-               
//- Owner:        Mike Eldred

#include "SharedC3ApproxData.hpp"
#include "ProblemDescDB.hpp"
#include "NonDIntegration.hpp"

#include "pecos_stat_util.hpp"
#include "pecos_global_defs.hpp"

#include <assert.h>
//#define DEBUG

namespace Dakota {


SharedC3ApproxData::
SharedC3ApproxData(ProblemDescDB& problem_db, size_t num_vars):
  SharedApproxData(BaseConstructor(), problem_db, num_vars),
  startOrder(problem_db.get_sizet("model.c3function_train.start_order")),
  maxOrder(problem_db.get_sizet("model.c3function_train.max_order")),
  startRank(problem_db.get_sizet("model.c3function_train.start_rank")),
  kickRank(problem_db.get_sizet("model.c3function_train.kick_rank")),
  maxRank(problem_db.get_sizet("model.c3function_train.max_rank")),
  adaptRank(problem_db.get_bool("model.c3function_train.adapt_rank")),
  regressType(problem_db.get_short("model.surrogate.regression_type")),
  regressRegParam(problem_db.get_real("model.surrogate.regression_penalty")),
  roundingTol(problem_db.get_real("model.c3function_train.rounding_tolerance")),
  solverTol(problem_db.get_real("model.c3function_train.solver_tolerance")),
  maxSolverIterations(problem_db.get_int("model.max_solver_iterations")),
  crossMaxIter(
    problem_db.get_int("model.c3function_train.max_cross_iterations")),
  c3Verbosity(0),//problem_db.get_int("model.c3function_train.verbosity")),
  adaptConstruct(false), crossVal(false)
{
  // This ctor used for user-spec of DataFitSurrModel (surrogate global ft)

  approxOpts = multi_approx_opts_alloc(num_vars);
  oneApproxOpts = (struct OneApproxOpts **)
    malloc(num_vars * sizeof(struct OneApproxOpts *));
  for (size_t ii = 0; ii < num_vars; ii++){
    struct OpeOpts * opts = ope_opts_alloc(LEGENDRE);
    ope_opts_set_lb(opts,-2); // BUG?
    ope_opts_set_ub(opts, 2); // BUG?
    ope_opts_set_nparams(opts,startOrder+1); // startnum = startord + 1
    // Note: maxOrder unused for regression;
    //       to be used for adaptation by cross-approximation
    ope_opts_set_maxnum(opts,maxOrder+1);    //   maxnum =   maxord + 1
    oneApproxOpts[ii] = one_approx_opts_alloc(POLYNOMIAL,opts);
    multi_approx_opts_set_dim(approxOpts,ii,oneApproxOpts[ii]);
    // oneApproxOpts[ii] = NULL;
    // multi_approx_opts_set_dim(approxOpts,ii,oneApproxOpts[ii]);
  }
}

  
SharedC3ApproxData::
SharedC3ApproxData(const String& approx_type,
		   const UShortArray& approx_order, size_t num_vars,
		   short data_order, short output_level):
  SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars, data_order,
		   output_level),
  // default values overridden by set_parameter
  startOrder(2), maxOrder(4), //maxnum(5),
  startRank(5), kickRank(2), maxRank(10), adaptRank(false),
  regressType(FT_LS), // non-regularized least sq
  solverTol(), roundingTol(1.e-10), crossMaxIter(5), maxSolverIterations(1000),
  c3Verbosity(0), adaptConstruct(false), crossVal(false)
{
  // This ctor used by lightweight/on-the-fly DataFitSurrModel ctor

  // short basis_type; approx_type_to_basis_type(approxType, basis_type);

  approxOpts = multi_approx_opts_alloc(num_vars);
  oneApproxOpts = (struct OneApproxOpts **)
    malloc(num_vars * sizeof(struct OneApproxOpts *));
  for (size_t ii = 0; ii < num_vars; ii++)
    oneApproxOpts[ii] = NULL;
}


SharedC3ApproxData::~SharedC3ApproxData()
{
  multi_approx_opts_free(approxOpts); approxOpts = NULL;

  for (size_t i=0; i<numVars; ++i) {
    one_approx_opts_free_deep(&oneApproxOpts[i]);
    oneApproxOpts[i] = NULL;
  }
  free(oneApproxOpts); oneApproxOpts = NULL;
}


void SharedC3ApproxData::
construct_basis(const Pecos::MultivariateDistribution& u_dist)
{
  const ShortArray& u_types = u_dist.random_variable_types();
  assert (u_types.size() == numVars);

  for (size_t i=0; i < numVars; ++i) {
    // printf("i = %zu\n",i);
    struct OpeOpts * opts = NULL;
    switch (u_types[i]) {
    case Pecos::STD_NORMAL:
      opts = ope_opts_alloc(HERMITE);
      break;
    case Pecos::STD_UNIFORM:
      opts = ope_opts_alloc(LEGENDRE);
      break;
    default:
      PCerr << "Error: unsupported u-space type (" << u_types[i] << ") in "
	    << "SharedC3ApproxData::distribution_parameters()" << std::endl;
      abort_handler(-1);
      break;
    }
    // printf("push_back\n");
    ope_opts_set_nparams(opts,startOrder+1); // startnum = startord + 1
    // Note: maxOrder unused for regression;
    //       to be used for adaptation by cross-approximation
    ope_opts_set_maxnum(opts,maxOrder+1);    //   maxnum =   maxord + 1
    one_approx_opts_free_deep(&oneApproxOpts[i]);
    oneApproxOpts[i] = one_approx_opts_alloc(POLYNOMIAL,opts);
    // printf("set i\n");
    multi_approx_opts_set_dim(approxOpts,i,oneApproxOpts[i]);
  }
}

    
size_t SharedC3ApproxData::pre_combine(short combine_type)
{
  Cerr << "Error: SharedC3ApproxData::pre_combine() not yet implemented."
       << std::endl;
  abort_handler(APPROX_ERROR);
  return 0;
}


void SharedC3ApproxData::post_combine(short combine_type)
{
  Cerr << "Error: SharedC3ApproxData::post_combine() not yet implemented."
       << std::endl;
  abort_handler(APPROX_ERROR);
}


void SharedC3ApproxData::link_multilevel_surrogate_data()
{
  // Manage approxDataKeys and original/modified approxData indices.
  // Note: {surr,modSurr}Data instances are managed in PecosApproximation::
  //       link_multilevel_surrogate_data()

  switch (discrepancyType) {
  case Pecos::DISTINCT_DISCREP: case Pecos::RECURSIVE_DISCREP: {
    /*
    // expand approxDataKeys from default for discrepancy management:
    approxDataKeys.resize(2); // for surrData and modSurrData
    UShort2DArray&     surr_keys = approxDataKeys[0];
    UShort2DArray& mod_surr_keys = approxDataKeys[1];
    // surrData has either HF or HF,LF keys (raw data: level 0 or levels 1-L)
    // modSurrData has HF key (combined data / discrepancy)
    surr_keys.resize(2);  mod_surr_keys.resize(1);
    //surr_keys[0] = mod_surr_keys[0] = pecosSharedDataRep->active_key();

    // 0 for original raw data, 1 for processed (discrepancy) data
    // (C3Approximation::link_multilevel_surrogate_data() pushes mod_surr_data
    // onto end of approxData)
    //origSurrDataIndex = 0;  // same as initialized value
    modSurrDataIndex = 1; // update from initialized value
    */
    approxDataKeys.resize(3); // HF, LF, discrep
    break;
  }
  default: // default ctor linkages are sufficient
    break;
  }
}

// *** TO DO: clarify distinction between current (active,truth,surrogate) model
// keys (which identify a single model form,lev) and approxData keys (richer:
// must disambiguate data for specific pairings).  Current design relies on the
// equivalence between truth key for both surrData and modSurrData and activeKey
// (only surrogate key gets modified to disambiguate).  This is not scalable to
// the case of n-way tuples for LVN,ACV ...
// > consolidate around one tuple-based active model key (Model also has the
//   notion of AGGREGATED,DISCREPANCY modes) and truth,surrogate are indices
//   within the tuple?
// > avoid top-down augmentations that cause problems bottom-up

// For now, just copying same key modifications as SharedPecosApproxData:

void SharedC3ApproxData::surrogate_model_key(const UShortArray& key)
{
  // approxDataKeys are organized in a 3D array: approxData instance by
  // {truth,surrogate} by multi-index key.  Note that AGGREGATED_MODELS mode
  // uses {HF,LF} order, as does ApproximationInterface::*_add()

  // Base/derived implementations assign/remove LF key for origSurrDataIndex.
  // Here, we modify the incoming LF key to allow association with its HF key.

  // *** NOTE: When managing distinct sets of paired truth,surrogate data (e.g.,
  // one set of data for discrepancy Q_l - Q_lm1 and another for Q_lm1 - Q_lm2,
  // it is important to identify the lm1 data with a specific pairing:
  // > Current approach: alter LF key (only) to disambiguate truth from
  //   surrogate key for the same model level
  // > Another approach: embed a tuple index to identify the key location; then
  //   an l,lm1 key concatenation could instead be used for discrepancy data in
  //   modSurrData (which must also include a tuple field to disambiguate:
  //   tuple-key1-[key2])
  //   >> Potential issue: this works fine for a recursive hierarchy, but is
  //      insufficient if lm1 could involve additional pairings
  
  //size_t d, num_d = approxDataKeys.size();
  UShort2DArray& raw_data_keys = approxDataKeys[0];
  if (key.empty()) { // prune second entry from each set of approxDataKeys
    /*
    for (i=0; i<num_sd; ++i)
    //if (maxNumKeys[i] > 1)      // need separate attribute to manage #keys
      if (surrogate_data_keys(i)) // since approxDataKeys shrinks/expands
        approxDataKeys[i].resize(1);
    */
    raw_data_keys.resize(1); // approxDataKeys[1] remains size 1
  }
  else {
    //for (d=0; i<num_d; ++d)
    //  if (surrogate_data_keys(i)) {
    raw_data_keys.resize(2);
    const UShortArray& hf_key = raw_data_keys[0]; // HF
    UShortArray&       lf_key = raw_data_keys[1]; // LF
    // Assign incoming LF key
    lf_key = key;
    lf_key.insert(lf_key.end(), hf_key.begin(), hf_key.end());
    //  }
  }
}


void SharedC3ApproxData::truth_model_key(const UShortArray& key)
{
  // approxDataKeys are organized in a 3D array: approxData instance by
  // {truth,surrogate} by multi-index key.  Note that AGGREGATED_MODELS mode
  // uses {HF,LF} order, as does ApproximationInterface::*_add()

  // Base class implementation updates only for origSurrDataIndex.  Here, we
  // will update across all approxData instances indicated in approxDataKeys.

  size_t d, num_d = approxDataKeys.size();
  for (d=0; d<num_d; ++d) {
    UShort2DArray& keys_d = approxDataKeys[d];
    //if (truth_data_keys(i)) { // for completeness
    switch (keys_d.size()) { // can remain 1 if no surrogate aggregation
    case 0: keys_d.push_back(key); break;
    case 1: keys_d[0] = key;       break;
    case 2: {
      UShortArray& hf_key = keys_d[0];
      UShortArray& lf_key = keys_d[1];
      if (hf_key != key) {
	// Assign HF key
	hf_key = key;
	// Update modified LF key (disambiguates pairings, see above)
	lf_key.resize(lf_key.size() - key.size());
	lf_key.insert(lf_key.end(), key.begin(), key.end());
      }
      break;
    }
    }
    //}
  }
}

} // namespace Dakota
