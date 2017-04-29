/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "RandomFieldModel.hpp"
#include "ParallelLibrary.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"

namespace Dakota {

/// initialization of static needed by RecastModel
RandomFieldModel* RandomFieldModel::rfmInstance(NULL);


RandomFieldModel::RandomFieldModel(ProblemDescDB& problem_db):
  RecastModel(problem_db, get_sub_model(problem_db)),
  // LPS TODO: initialize other class data members off problemDB
  numFunctions(subModel.num_functions()),
  numObservations(0), 
  expansionForm(problem_db.get_ushort("model.rf.expansion_form")),
  covarianceForm(problem_db.get_ushort("model.rf.analytic_covariance")),
  requestedReducedRank(problem_db.get_int("model.rf.expansion_bases")),
  percentVariance(problem_db.get_real("model.truncation_tolerance")),
  actualReducedRank(5), mappingInitialized(false)
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
  if (rfDataFilename.empty() && daceIterator.is_null() && (covarianceForm == NOCOVAR)) {
    Cerr << "\nError: Random field model requires data_file or "
	 << "dace_method_pointer or specification of an analytic covariance" 
         << std::endl;
    abort_handler(MODEL_ERROR);
  }

  //  }
}


/** May eventually take on init_comms and related operations.  Also
    may want ide of build/update like DataFitSurrModel, eventually. */
bool RandomFieldModel::initialize_mapping(ParLevLIter pl_iter)
{
  RecastModel::initialize_mapping(pl_iter);

  // TODO: create modes to switch between generating, accepting, and
  // underlying model
  fieldRealizationId=0;

  // runtime operation to identify the random field approximation
  if (covarianceForm != NOCOVAR) { 
    rf_suite_identify_field_model();
    expansionForm == RF_KARHUNEN_LOEVE;
  }
  else { 
    get_field_data();
    identify_field_model();
  }

  // complete initialization of the base RecastModel
  initialize_recast();

  mappingInitialized = true;

  if (expansionForm == RF_KARHUNEN_LOEVE) {
    // augment AleatoryDistParams with normal(0,1)
    initialize_rf_coeffs();
    // update message lengths for send/receive of parallel jobs (normally
    // performed once in Model::init_communicators() just after construct time)
    estimate_message_lengths();

    return true; // size of variables changed
  }
  // may need true for PCA/GP if the vars characterization changes
  return false; // size of variables unchanged
}


bool RandomFieldModel::finalize_mapping()
{
  mappingInitialized = false;

  // TODO: return to submodel space
  // probably don't do this...

  RecastModel::finalize_mapping();

  return false; // This will become true when TODO is implemented.
}


/** Populate rfBuildData */
void RandomFieldModel::get_field_data()
{
  // TODO: either load the data matrix using ReducedBasis utilities or
  // run daceIterator

  if (daceIterator.is_null()) {
    // TODO: temporary data for testing
    std::ifstream rf_file;
    rf_file.open("rfbuild.test");
    RealVectorArray va;
    read_sized_data(rf_file, va, 5, 50);
    rfBuildData.reshape(5,50);
    copy_data(va,rfBuildData);
  }
  else {
    // generate samples of the random field
    Cout << "\nRandomFieldModel: Gathering random field data from RF-generating "
         << "model" << std::endl;
    daceIterator.run();
    size_t num_samples = daceIterator.num_samples();
    // BMA TODO: relax assumption of allSamples / compactMode
    // Generalize to discrete vars
    if (expansionForm == RF_PCA_GP) {
      rfBuildVars.reshape(subModel.cv(), num_samples);
      rfBuildVars.assign(daceIterator.all_samples());
    }
    rfBuildData.reshape(num_samples, numFunctions);
    const IntResponseMap& all_resp = daceIterator.all_responses();
    IntRespMCIter r_it = all_resp.begin(); 
    for (size_t samp=0; samp<num_samples; ++samp, ++r_it)
      for (size_t fn=0; fn<numFunctions; ++fn) 
        rfBuildData(samp,fn) = r_it->second.function_value(fn);
  }
}


/// Alternative to below function when using RFSuite
void RandomFieldModel::rf_suite_identify_field_model()
{
  actualReducedRank = requestedReducedRank;
  // Build an RF suite reduced model
  // dprepro to insert covariance type / parameters / correl lengths, bases, percent variance
  Cout << "In rf_suite_identify_field_model  " << '\n';
  //std::system("dprepro covsettings.txt KL_solve.template KL_solve.i");
  std::system("./run_kl_solve.sh");
}


void RandomFieldModel::identify_field_model()
{
  // operations common to both representations
  rfBasis.set_matrix(rfBuildData);
  rfBasis.update_svd(true);  // true: center the matrix before factoring
  //percentVariance = 0.9; // hardcoded: need to remove
  ReducedBasis::VarianceExplained truncation(percentVariance);
  actualReducedRank = truncation.get_num_components(rfBasis);
  Cout << "RandomFieldModel: retaining " << actualReducedRank 
       << " basis functions." << std::endl;

  switch(expansionForm) {
  case RF_KARHUNEN_LOEVE: {
    Cout << "KL Expansion Form " << '\n';
    break;
  }
  case RF_PCA_GP: {
    
    Cout << "PCA Expansion Form " << '\n';

    int num_samples = rfBuildData.numRows();

    // Get the centered version of the original response matrix
    const RealMatrix& centered_matrix = rfBasis.get_matrix();
    // for now get the first factor score
    const RealMatrix& principal_comp
      = rfBasis.get_right_singular_vector_transpose();

    // Compute the factor scores
    RealMatrix factor_scores(num_samples, numFunctions);
    int myerr = factor_scores.multiply(Teuchos::NO_TRANS, Teuchos::TRANS, 1., 
                                       centered_matrix, principal_comp, 0.);

    // BMA TODO: verify why this size differs; seems that factor
    // scores should be n x p and that this could seg fault if
    // num_samples > numFunctions...
    RealMatrix
      f_scores(Teuchos::Copy, factor_scores, num_samples, num_samples, 0, 0);

    // build the GP approximations, one per principal component
    String approx_type("global_kriging"); // Surfpack GP
    UShortArray approx_order;
    short data_order = 1;  // assume only function values
    short output_level = NORMAL_OUTPUT;
    SharedApproxData sharedData;

    // BMA TODO: generalize to discrete vars if possible
    sharedData = SharedApproxData(approx_type, approx_order, subModel.cv(),
                                  data_order, output_level);

    // build one GP for each Principal Component
    gpApproximations.clear();
    for (int i = 0; i < actualReducedRank; ++i)
      gpApproximations.push_back(Approximation(sharedData));
    for (int i = 0; i < actualReducedRank; ++i) {
      RealVector factor_i = Teuchos::getCol(Teuchos::View,f_scores,i);
      gpApproximations[i].add(rfBuildVars, factor_i);
      gpApproximations[i].build();
      const String gp_string = boost::lexical_cast<String>(i);
      const String gp_prefix = "PCA_GP";
      gpApproximations[i].export_model(gp_string, gp_prefix, ALGEBRAIC_FILE);
    }
    break;
  }
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
  // is likely the identity.
  //
  // Consider two derived classes or deriving PCA/GP model from
  // DataFitSurrModel, especially since no Recast is needed for the
  // PCA/GP case.  
  // ---

  // We assume the mapping is for all active variables, but only
  // normal uncertain get modified
  size_t submodel_vars = 
    subModel.cv()+ subModel.div() + subModel.dsv() + subModel.drv();
  size_t recast_vars = submodel_vars + actualReducedRank;

  // BMA TODO: This is wrong!  Assumes normal lead the cv array!
  const Pecos::AleatoryDistParams& adp = 
    rfmInstance->subModel.aleatory_distribution_parameters();
  size_t num_sm_normal = adp.normal_means().length();

  // TODO: don't assume this for PCA case!

  // In general, each submodel continuous variable depends on all of
  // the recast (reduced) variables; others are one-to-one.
  Sizet2DArray vars_map_indices(submodel_vars);
  for (size_t i=0; i<submodel_vars; ++i) {
    vars_map_indices[i].resize(submodel_vars);
    size_t j = 0;
    for ( ; j<num_sm_normal; ++j)
      vars_map_indices[i][j] = j;
    // skip the N(0,1) coeffs, if they exist
    for ( ; j<num_sm_normal; ++j)
      vars_map_indices[i][j] = actualReducedRank + j;
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
  // BMA TODO: more gracefully insert the variables into their place

  // the map size only changes for KL to augment the coeffs by
  // actualReducedRank N(0,1) variables
  if (expansionForm == RF_KARHUNEN_LOEVE) {

    // get submodel normal parameters (could get from current object as well)
    const Pecos::AleatoryDistParams& sm_adp = 
      subModel.aleatory_distribution_parameters();
    RealVector normal_means = sm_adp.normal_means();
    RealVector normal_std_deviations = sm_adp.normal_std_deviations();
    RealVector normal_lb = sm_adp.normal_lower_bounds();
    RealVector normal_ub = sm_adp.normal_upper_bounds();

    // append normal variables
    int num_sm_normal = normal_means.length();
    normal_means.resize(num_sm_normal + actualReducedRank);
    normal_std_deviations.resize(num_sm_normal + actualReducedRank);
    normal_lb.resize(num_sm_normal + actualReducedRank);
    normal_ub.resize(num_sm_normal + actualReducedRank);
    // BMA TODO: update label management to not assume normal are leading vars
    StringMultiArrayConstView sm_cv_labels = 
      subModel.continuous_variable_labels();
    for (int i=0 ; i<num_sm_normal; ++i)
      currentVariables.continuous_variable_label(sm_cv_labels[i], i);
    for (int i=0; i<actualReducedRank; ++i) {
      normal_means[num_sm_normal + i] = 0.0;
      normal_std_deviations[num_sm_normal + i] = 1.0;
      normal_lb[num_sm_normal + i] = -std::numeric_limits<Real>::infinity();
      normal_ub[num_sm_normal + i] = std::numeric_limits<Real>::infinity();
      String xi_label = "xi_" + boost::lexical_cast<String>(i+1);
      currentVariables.
        continuous_variable_label(xi_label, num_sm_normal + i);
    }
    for (int i=num_sm_normal; i<sm_cv_labels.size(); ++i)
      currentVariables.
        continuous_variable_label(sm_cv_labels[i], actualReducedRank + i);

    // update ADP on the RandomFieldModel
    Pecos::AleatoryDistParams& rfm_adp = aleatory_distribution_parameters();
    rfm_adp.normal_means(normal_means);
    rfm_adp.normal_std_deviations(normal_std_deviations);
    rfm_adp.normal_lower_bounds(normal_lb);
    rfm_adp.normal_upper_bounds(normal_ub);
  }
}

/// map the active continuous recast variables to the active
/// submodel variables 
void RandomFieldModel::vars_mapping(const Variables& recast_augmented_vars, 
                                    Variables& sub_model_vars)
{
  if (rfmInstance->expansionForm == RF_KARHUNEN_LOEVE) {
    // send the submodel all but the N(0,1) vars

    // BMA TODO: generalize this for other views; for now, assume cv()
    // starts with normal uncertain
    size_t num_sm_cv = rfmInstance->subModel.cv();
    const Pecos::AleatoryDistParams& adp = 
      rfmInstance->subModel.aleatory_distribution_parameters();
    size_t num_sm_normal = adp.normal_means().length();
    const RealVector& augmented_cvars = 
      recast_augmented_vars.continuous_variables();

    size_t i = 0;
    RealVector sm_cvars(num_sm_cv);
    for ( ; i<num_sm_normal; ++i)
      sm_cvars[i] = augmented_cvars[i];
    // skip the N(0,1) coeffs, if they exist
    for ( ; i<num_sm_cv; ++i)
      sm_cvars[i] = augmented_cvars[rfmInstance->actualReducedRank + i];

    // propagate variables to subModel
    sub_model_vars.continuous_variables(sm_cvars);
    sub_model_vars.discrete_int_variables(recast_augmented_vars.
                                          discrete_int_variables());
    sub_model_vars.discrete_string_variables(recast_augmented_vars.
                                             discrete_string_variables());
    sub_model_vars.discrete_real_variables(recast_augmented_vars.
                                           discrete_real_variables());
  }
  else
    sub_model_vars.active_variables(recast_augmented_vars);
}


void RandomFieldModel::derived_evaluate(const ActiveSet& set)
{
  fieldRealizationId++;

  if (expansionForm == RF_KARHUNEN_LOEVE)
    generate_kl_realization();
  else if (expansionForm == RF_PCA_GP)
    generate_pca_gp_realization();

  // May need hook to communicate the appropraiate mesh file to the simulation

  RecastModel::derived_evaluate(set);

}


void RandomFieldModel::derived_evaluate_nowait(const ActiveSet& set)
{
  fieldRealizationId++;

  if (expansionForm == RF_KARHUNEN_LOEVE)
    generate_kl_realization();
  else if (expansionForm == RF_PCA_GP)
    generate_pca_gp_realization();

  RecastModel::derived_evaluate_nowait(set);
}


void RandomFieldModel::generate_kl_realization()
{
  // ReducedBasis gives the singular values of the centered data
  // matrix; the covariance is scaled by n-1:
  int cov_dof = std::sqrt( (double)rfBasis.get_matrix().numRows() - 1 );
  const RealVector& data_singular_values = rfBasis.get_singular_values();
  RealMatrix rf_ev_trans = rfBasis.get_right_singular_vector_transpose();

  // extract N(0,1) KL coefficients to generate a field realization
  //
  // BMA TODO: properly extract the N(0,1) vars from their place in
  // the overall vector when not in aleatory view (what's in the total
  // vector depends on the view...)
  const Pecos::AleatoryDistParams& adp = 
    subModel.aleatory_distribution_parameters();
  size_t num_sm_normal = adp.normal_means().length();
  const RealVector& augmented_cvars = currentVariables.continuous_variables();
  RealVector kl_coeffs(Teuchos::View, augmented_cvars.values() + num_sm_normal,
                       actualReducedRank);

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "Augmented continuous variables:\n" << augmented_cvars << std::endl;
    Cout << "KL random coeffs:\n" << kl_coeffs << std::endl;
  }

  // Run KL realize with RF Suite
  // dprepro to configure kl_realize.i with kl_coeffs, mesh number
  //std::system("dprepro randomcoeffs.txt KL_realize.template KL_realize.i");
  //std::system("launch -n 1 encore -i KL_realize.i");

  if (covarianceForm != NOCOVAR) { 
    std::system("./run_kl_realize.sh");
  }

  // post-condition: mesh file gets written containing the field realization
  // probably want to tag it with evalID...
  // bfs::copy(mesh.exo mesh.realize1.exo)
  // need to obtain the realization ID for this Model/Interface

  // For now use fieldRealizationId

  // May want to create a new derived Interface from
  // ForkApplicInterface called ExternalFieldManagerInterface
  // in order to better manage hierarchical tagging

  // augment the mean prediction with the covariance contributions
  RealVector kl_prediction = rfBasis.get_column_means();
  for (int i=0; i<actualReducedRank; ++i) {
    // BMA TODO: replace with matrix-vector ops (perhaps in ReducedBasis)
    Real mult = data_singular_values[i]/cov_dof * kl_coeffs[i];
    for (int k=0; k<numFunctions; ++k)
      kl_prediction[k] += mult*rf_ev_trans(i,k);
  }

  write_field(kl_prediction);
}


void RandomFieldModel::generate_pca_gp_realization()
{
  // augment the mean prediction with the GP contributions
  RealVector pca_prediction = rfBasis.get_column_means();
  // reduced basis comprised of the rows of V'
  const RealMatrix& principal_comp
    = rfBasis.get_right_singular_vector_transpose();

  for (int i=0; i<actualReducedRank; ++i) {
    const RealVector& new_sample = currentVariables.continuous_variables();
    Real pca_coeff = gpApproximations[i].value(new_sample);
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "DEBUG: pca_coeff = " << pca_coeff << '\n';
    for (int k=0; k<numFunctions; ++k)
      pca_prediction[k] += pca_coeff*principal_comp(i,k);
  }

  write_field(pca_prediction);
}


void RandomFieldModel::write_field(const RealVector& field_prediction)
{
  // TODO: write to file per eval, preferrably in work_directory
  // BMA TODO: something smarter than modelEvalCntr / eval ID
  if (outputLevel >= VERBOSE_OUTPUT) {
    String pred_count(boost::lexical_cast<String>(evaluation_id() + 1));
    std::ofstream myfile;
    myfile.open(("field_prediction." + pred_count + ".txt").c_str());
    Cout << "Field prediction " << pred_count << "\n";
    Cout << field_prediction << std::endl;
    for (int i=0; i<field_prediction.length(); ++i) 
      myfile << field_prediction[i] << " ";
    myfile << std::endl;
  }
}

/// map the inbound ActiveSet to the sub-model (map derivative variables)
void RandomFieldModel::set_mapping(const Variables& recast_vars,
				   const ActiveSet& recast_set,
				   ActiveSet& sub_model_set)
{

}

 
}
