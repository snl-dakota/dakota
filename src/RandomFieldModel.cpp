/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "RandomFieldModel.hpp"
#include "ParallelLibrary.hpp"

namespace Dakota {

/// initialization of static needed by RecastModel
RandomFieldModel* RandomFieldModel::rfmInstance(NULL);


RandomFieldModel::RandomFieldModel(ProblemDescDB& problem_db):
  RecastModel(problem_db, get_sub_model(problem_db)),
  // LPS TODO: initialize other class data members off problemDB
  numFunctions(subModel.num_functions()),
  numObservations(0), expansionForm(RF_KARHUNEN_LOEVE),
  requestedReducedRank(0), percentVariance(1.0),
  actualReducedRank(5)
{
  rfmInstance = this;
  modelType = "random_field";

  init_dace_iterator(problem_db);

  validate_inputs();
}


RandomFieldModel::~RandomFieldModel()
{  /* empty dtor */  }


Model RandomFieldModel::get_sub_model(ProblemDescDB& problem_db)
{
  Model sub_model;

  const String& propagation_model_pointer
    = problem_db.get_string("model.rf.propagation_model_pointer");
  size_t model_index = problem_db.get_db_model_node(); // for restoration
  problem_db.set_db_model_nodes(propagation_model_pointer);
  sub_model = problem_db.get_model();
  //check_submodel_compatibility(actualModel);
  problem_db.set_db_model_nodes(model_index); // restore

  return sub_model;
}


void RandomFieldModel::init_dace_iterator(ProblemDescDB& problem_db)
{
  const String& dace_method_pointer
    = problem_db.get_string("model.dace_method_pointer");

  if (!dace_method_pointer.empty()) { // global DACE approximations
    size_t method_index = problem_db.get_db_method_node(); // for restoration
    size_t model_index  = problem_db.get_db_model_node();  // for restoration
    problem_db.set_db_list_nodes(dace_method_pointer);

    // instantiate the DACE iterator, which instantiates the actual model
    daceIterator = problem_db.get_iterator();
    daceIterator.sub_iterator_flag(true);

    // retrieve the actual model from daceIterator (invalid for selected
    // meta-iterators, e.g., hybrids)
    Model& rf_generating_model = daceIterator.iterated_model();
    // BMA TODO: review
    //    check_submodel_compatibility(actualModel);
    // if outer level output is verbose/debug and actualModel verbosity is
    // defined by the DACE method spec, request fine-grained evaluation
    // reporting for purposes of the final output summary.  This allows verbose
    // final summaries without verbose output on every dace-iterator completion.
    if (outputLevel > NORMAL_OUTPUT)
      rf_generating_model.fine_grained_evaluation_counters();

    problem_db.set_db_method_node(method_index); // restore method only
    problem_db.set_db_model_nodes(model_index);  // restore all model nodes


    // TODO: may want to make this a quieter, lighter iterator?
    daceIterator.sub_iterator_flag(true);
  }
}


void RandomFieldModel::validate_inputs()
{
  //if (buildField) {
  if (rfDataFilename.empty() && daceIterator.is_null()) {
    Cerr << "\nError: Random field model requires data_file or "
	 << "dace_method_pointer" << std::endl;
    abort_handler(MODEL_ERROR);
  }

  //  }
}


/** May eventually take on init_comms and related operations.  Also
    may want ide of build/update like DataFitSurrModel, eventually. */
bool RandomFieldModel::initialize_mapping()
{
  // TODO: create modes to switch between generating, accepting, and
  // underlying model

  get_field_data();

  // runtime operation to identify the subspace model (if not later
  // returning to update the subspace)
  identify_field_model();

  // complete initialization of the base RecastModel
  initialize_recast();

  // TODO: generalize to other distribution types
  // convert the normal distributions to the reduced space and set in the
  // reduced model
  //  uncertain_vars_to_subspace();

  // update with subspace constraints
  //  update_linear_constraints();
  if (expansionForm == RF_KARHUNEN_LOEVE)
    return true; // Size of variables always changes 
  else
    return false; // size of variables unchanged
}


bool RandomFieldModel::finalize_mapping()
{
  // TODO: return to submodel space
  // probably don't do this...
  return false; // This will become true when TODO is implemented.
}


void RandomFieldModel::get_field_data()
{
  // TODO: either load the data matrix using ReducedBasis utilities or
  // run daceIterator

  // populate rfBuildData;
  
  // temporary data for testing
  std::ifstream rf_file;
  rf_file.open("rfbuild.test");
  RealVectorArray va;
  read_sized_data(rf_file, va, 5, 50);
  rfBuildData.reshape(5,50);
  copy_data(va,rfBuildData);
  
}


void RandomFieldModel::identify_field_model()
{
  switch(expansionForm) {
  case RF_KARHUNEN_LOEVE:
    // TODO: Use ReducedBasis class to manage the decomposition
    {rfBasis.set_matrix(rfBuildData);
    rfBasis.update_svd(); //includes centering the matrix
    percentVariance = 0.9; //hardcoded: need to remove
    ReducedBasis::VarianceExplained truncation(percentVariance);
    actualReducedRank = truncation.get_num_components(rfBasis);
    Cout << "number of actual basis functions used " 
         << actualReducedRank
         << '\n' << std::endl;
    //These will have a size of actualReducedRank
    RealVector rfEigenvalues = rfBasis.get_singular_values();
    RealMatrix rfBasisFns = rfBasis.get_right_singular_vector_transpose();
    } 
    break;
  case RF_PCA_GP:
    break;
  default:
    Cerr << "Not implemented";
    break;
  }
}


/** Initialize the recast model to augment the uncertain variables
    with actualReducedRank additional N(0,1) variables, with no
    response function mapping (for now).*/
void RandomFieldModel::initialize_recast()
{
  // For now, we assume the map is over all functions, without
  // distinguishing primary from secondary

  // ---
  // Variables mapping: RecastModel maps augmented (uncertainVars +
  // RFcoeffs) variables to original submodel which accepts only
  // uncertainVars (plus a field realization)
  //
  // BMA TODO: In the case of KL, vars are expanded by RFcoeffs; in
  // case of PCA/GP, only original uncVars are needed and the vars map
  // is likely the identify.
  //
  // Consider two derived classes or deriving PCA/GP model from
  // DataFitSurrModel, especially since no Recast is needed for the
  // PCA/GP case.  
  // ---

  // We assume the mapping is for all active variables, but only
  // normal uncertain get modified
  size_t submodel_cv = subModel.cv();
  size_t submodel_dv = subModel.div() + subModel.dsv() + subModel.drv();
  size_t submodel_vars = submodel_cv + submodel_dv;


  size_t recast_cv = actualReducedRank;
  size_t recast_vars = recast_cv + submodel_dv;

  // In general, each submodel continuous variable depends on all of
  // the recast (reduced) variables; others are one-to-one.
  Sizet2DArray vars_map_indices(submodel_vars);
  for (size_t i=0; i<submodel_cv; ++i) {
    vars_map_indices[i].resize(recast_cv);
    for (size_t j=0; j<recast_cv; ++j)
      vars_map_indices[i][j] = j;
  }
  for (size_t i=0; i<submodel_dv; ++i) {
    vars_map_indices[submodel_cv + i].resize(1);
    vars_map_indices[submodel_cv + i][0] = recast_cv + i;
  }
  // Variables map is linear
  bool nonlinear_vars_mapping = false;

  SizetArray vars_comps_total = variables_resize();
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation

  // Primary and secondary mapping are one-to-one (NULL callbacks)
  // TODO: can we get RecastModel to tolerate empty indices when no
  // map is present?
  size_t num_primary = subModel.num_primary_fns(),
    num_secondary = subModel.num_functions() - subModel.num_primary_fns(),
    num_recast_fns = num_primary + num_secondary,
    recast_secondary_offset = subModel.num_nonlinear_ineq_constraints();

  Sizet2DArray primary_resp_map_indices(num_primary);
  for (size_t i=0; i<num_primary; i++) {
    primary_resp_map_indices[i].resize(1);
    primary_resp_map_indices[i][0] = i;
  }

  Sizet2DArray secondary_resp_map_indices(num_secondary);
  for (size_t i=0; i<num_secondary; i++) {
    secondary_resp_map_indices[i].resize(1);
    secondary_resp_map_indices[i][0] = num_primary + i;
  }

  BoolDequeArray nonlinear_resp_mapping(numFunctions,
					BoolDeque(numFunctions, false));

  // Initial response order for the newly built subspace model same as
  // the subModel (does not augment with gradient request)
  const Response& curr_resp = subModel.current_response();
  short recast_resp_order = 1; // recast resp order to be same as original resp
  if (!curr_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (!curr_resp.function_hessians().empty())  recast_resp_order |= 4;

  RecastModel::
    init_sizes(vars_comps_total, all_relax_di, all_relax_dr, num_primary, 
	       num_secondary, recast_secondary_offset, recast_resp_order);

  RecastModel::
    init_maps(vars_map_indices, nonlinear_vars_mapping, vars_mapping, 
	      set_mapping, primary_resp_map_indices, secondary_resp_map_indices,
	      nonlinear_resp_mapping, NULL, NULL);
}


/// Create a variables components totals array with the reduced space
/// size for continuous variables
/// TODO: augment normal uncVars for KL case
SizetArray RandomFieldModel::variables_resize()
{
  const SharedVariablesData& svd = subModel.current_variables().shared_data();
  SizetArray vc_totals = svd.components_totals();

  // the map size only changes for KL to augment the coeffs by
  // actualReducedRank N(0,1) variables
  if (expansionForm == RF_KARHUNEN_LOEVE) {
     // TODO: validate active view on subModel in case there are now CAUV
     vc_totals[TOTAL_CAUV] += actualReducedRank;
  }
  return vc_totals;
}


/** Initialzie the aleatory dist params for the KL coeffs */
void RandomFieldModel::initialize_rf_coeffs()
{
  // BMA TODO: which ADP to update from?

  // the map size only changes for KL to augment the coeffs by
  // actualReducedRank N(0,1) variables
  if (expansionForm == RF_KARHUNEN_LOEVE) {
    // get submodel normal parameters (could get from current object as well)
    const Pecos::AleatoryDistParams& sm_adp = 
      subModel.aleatory_distribution_parameters();
    RealVector normal_means = sm_adp.normal_means();
    RealVector normal_std_deviations = sm_adp.normal_std_deviations();
    // append normal variables
    int num_normal = normal_means.length();
    normal_means.resize(num_normal + actualReducedRank);
    normal_std_deviations.resize(num_normal + actualReducedRank);
    for (int i=0; i<actualReducedRank; ++i) {
      normal_means[num_normal + i] = 1.0;
      normal_std_deviations[num_normal + i] = 0.0;
    }
    // update ADP on thie RandomFieldModel
    Pecos::AleatoryDistParams& rfm_adp = aleatory_distribution_parameters();
    rfm_adp.normal_means(normal_means);
    rfm_adp.normal_std_deviations(normal_std_deviations);
  }
}

/// map the active continuous recast variables to the active
/// submodel variables 
void RandomFieldModel::vars_mapping(const Variables& recast_xi_vars, 
				    Variables& sub_model_x_vars)
{
  // use the N(0,1) vars to generate a field realization
  rfmInstance->generate_field_realization();

  // send the submodel all but the N(0,1) vars
}


void RandomFieldModel::generate_field_realization()
{

}


/// map the inbound ActiveSet to the sub-model (map derivative variables)
void RandomFieldModel::set_mapping(const Variables& recast_vars,
				   const ActiveSet& recast_set,
				   ActiveSet& sub_model_set)
{

}

 
}
