/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDC3FunctionTrain
//- Description: Implementation code for NonDC3FunctionTrain class
//- Owner:       Alex Gorodetsky

#include "dakota_system_defs.hpp"
#include "NonDC3FunctionTrain.hpp"
#include "C3Approximation.hpp"
#include "SharedC3ApproxData.hpp"
#include "DataFitSurrModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
//#define DEBUG


namespace Dakota {

NonDC3FunctionTrain* NonDC3FunctionTrain::c3Instance(NULL);


/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDC3FunctionTrain::
NonDC3FunctionTrain(ProblemDescDB& problem_db, Model& model):
  NonDExpansion(problem_db, model),
  // numSamplesOnEmulator(probDescDB.get_int("method.nond.samples_on_emulator")),
  importApproxPointsFile(probDescDB.get_string("method.import_approx_points_file")),
  importApproxFormat(probDescDB.get_ushort("method.import_approx_format")),
  importApproxActiveOnly( probDescDB.get_bool("method.import_approx_active_only")),
  importBuildPointsFile(
    probDescDB.get_string("method.import_build_points_file")),
  importBuildFormat(probDescDB.get_ushort("method.import_build_format")),
  importBuildActiveOnly(probDescDB.get_bool("method.import_build_active_only")),
  numSamplesOnModel(probDescDB.get_sizet("method.c3function_train.num_samples_for_construction"))
  // exportPointsFile(izet()
  //     problem_db.get_string("model.export_approx_points_file"))
{
  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order;
  short u_space_type = ASKEY_U;//probDescDB.get_short("method.nond.expansion_type");
  resolve_inputs(u_space_type, data_order);
  initialize_random(u_space_type);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------

  Model g_u_model;
  transform_model(iteratedModel, g_u_model); // retain distribution bounds

  // Evaluates true model
  Iterator u_space_sampler;
  if (numSamplesOnModel) { // generate new data
	  // default pattern is fixed for consistency in any outer loop,
	  // but gets overridden in cases of unstructured grid refinement.
	  bool vary_pattern = false;
	  construct_lhs(u_space_sampler, g_u_model,
	    probDescDB.get_ushort("method.sample_type"), numSamplesOnModel,
	    randomSeed, probDescDB.get_string("method.random_number_generator"),
	    vary_pattern, ACTIVE);
	}

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for SCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse;
  String approx_type = "function_train"; // Modify DakotaApproximation get_approx()
  UShortArray approx_order; // empty
  ActiveSet sc_set = g_u_model.current_response().active_set(); // copy
  sc_set.request_values(3); // stand-alone mode: surrogate grad evals at most
  String empty_str; // build data import not supported for structured grids
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
                                              sc_set, approx_type, approx_order, corr_type, corr_order, data_order,
                                              outputLevel, pt_reuse, importBuildPointsFile, importBuildFormat,
                                              importBuildActiveOnly,
                                              probDescDB.get_string("method.export_approx_points_file"),
                                              probDescDB.get_ushort("method.export_approx_format")), false);

  initialize_u_space_model();

  // -------------------------------
  // Construct expSampler, if needed
  // -------------------------------
  // ...
  // follow construct_expansion_sampler from NonDExpansion.cpp which is called in NonDPolynomialChaos.cpp
  construct_expansion_sampler(importApproxPointsFile, importApproxFormat, 
                              importApproxActiveOnly);
}


NonDC3FunctionTrain::~NonDC3FunctionTrain()
{

}

    
void NonDC3FunctionTrain::
resolve_inputs(short& u_space_type, short& data_order)
{
  // May want this eventually to manage different transformation options...
    
  data_order = 1; // no deriv enhancement for now...
}

void NonDC3FunctionTrain::initialize_u_space_model()
{
  // // Commonly used approx settings (e.g., order, outputLevel, useDerivs) are
  // // passed through the DataFitSurrModel ctor chain.  Additional data needed
  // // by OrthogPolyApproximation are passed using Pecos::BasisConfigOptions.
  // // Note: passing useDerivs again is redundant with the DataFitSurrModel ctor.
  SharedC3ApproxData* shared_data_rep = (SharedC3ApproxData*)
    uSpaceModel.shared_approximation().data_rep();

  // // For PCE, the approximation and integration bases are the same.  We (always)
  // // construct it for the former and (conditionally) pass it in to the latter.
  shared_data_rep->construct_basis(natafTransform.u_types(),
                                   iteratedModel.aleatory_distribution_parameters());

  size_t model_index    = probDescDB.get_db_model_node(); // for restoration
  String model_ptr_name = probDescDB.get_string("method.c3function_train.model_param_spec");
  // String model_ptr_name  = "FT";
  probDescDB.set_db_model_nodes(model_ptr_name);

  size_t start_order  = probDescDB.get_sizet("model.c3function_train.start_order");
  size_t max_order    = probDescDB.get_sizet("model.c3function_train.max_order");
  size_t start_rank   = probDescDB.get_sizet("model.c3function_train.start_rank");
  size_t kick_rank    = probDescDB.get_sizet("model.c3function_train.kick_rank");
  size_t max_rank     = probDescDB.get_sizet("model.c3function_train.max_rank");
  bool   adapt_rank   = probDescDB.get_bool("model.c3function_train.adapt_rank");
  size_t cross_max_iter = probDescDB.get_sizet("model.c3function_train.max_cross_iterations");
  double solver_tol   = probDescDB.get_real("model.c3function_train.solver_tolerance");
  double rounding_tol = probDescDB.get_real("model.c3function_train.rounding_tolerance");
  //size_t max_iters  = probDescDB.get_int("model.max_iterations");
  size_t verbose      = probDescDB.get_sizet("model.c3function_train.verbosity");
  
  probDescDB.set_db_model_nodes(model_index); // restore

  shared_data_rep->set_parameter("start_poly_order",&start_order);
  shared_data_rep->set_parameter("max_poly_order",  &max_order);
  shared_data_rep->set_parameter("start_rank",      &start_rank);
  shared_data_rep->set_parameter("kick_rank",       &kick_rank);
  shared_data_rep->set_parameter("max_rank",        &max_rank);
  shared_data_rep->set_parameter("adapt_rank",      &adapt_rank);
  shared_data_rep->set_parameter("solver_tol",      &solver_tol);
  shared_data_rep->set_parameter("rounding_tol",    &rounding_tol);
  shared_data_rep->set_parameter("max_cross_iterations", &cross_max_iter);
  //shared_data_rep->set_parameter("max_iterations", &max_iters);
  shared_data_rep->set_parameter("verbose",         &verbose);

  std::cout << "solver_tol = " << solver_tol << "\n";
  std::cout << "adapt_rank = " << adapt_rank << "\n";
  
  // // if all variables mode, initialize key to random variable subset
  // NOT SURE WHAT TO DO BELOW --AG
  // if (numContDesVars || numContEpistUncVars || numContStateVars) {
  //   Pecos::BitArray random_vars_key(numContinuousVars);
  //   size_t i, num_cdv_cauv = numContDesVars + numContAleatUncVars;
  //   for (i=0; i<numContinuousVars; ++i)
  //     random_vars_key[i] = (i >= numContDesVars && i < num_cdv_cauv);
  //   shared_data_rep->random_variables_key(random_vars_key);
  // }

  // share natafTransform instance with u-space sampler
  Iterator& u_space_sampler = uSpaceModel.subordinate_iterator();
  if (!u_space_sampler.is_null())
    ((NonD*)u_space_sampler.iterator_rep())->
      initialize_random_variables(natafTransform); // shared rep

}
    
void NonDC3FunctionTrain::print_results(std::ostream& s)
{

  if (//iteratedModel.subordinate_models(false).size() == 1 &&
      iteratedModel.truth_model().solution_levels() > 1) {
    s << "<<<<< Samples per solution level:\n";
    print_multilevel_evaluation_summary(s, NLev);
    s << "<<<<< Equivalent number of high fidelity evaluations: "
      << equivHFEvals << std::endl;
  }

  NonDExpansion::print_results(s);
}
  

int NonDC3FunctionTrain::
qoi_eval(size_t num_samp, const double* var_sets, double* qoi_sets, void* args)
{
  size_t num_cv  = c3Instance->numContinuousVars,
         num_fns = c3Instance->numFunctions;

  bool asynch_flag = c3Instance->iteratedModel.asynch_flag();

  RealVector cv_i(num_cv, false);
  for (size_t i=0; i<num_samp; ++i) {
    copy_data(var_sets+num_cv*i, num_cv, cv_i);
    c3Instance->iteratedModel.continuous_variables(cv_i);
    if (asynch_flag)
      c3Instance->iteratedModel.evaluate_nowait();
    else {
      c3Instance->iteratedModel.evaluate();
      // pack Dakota resp data into qoi_sets...
      const RealVector& fns_i = c3Instance->iteratedModel.current_response().function_values();
      copy_data(fns_i, qoi_sets+num_fns*i, num_fns);
    }
  }

  if (asynch_flag) {
    const IntResponseMap& resp_map = c3Instance->iteratedModel.synchronize();
    // pack Dakota resp data into qoi_sets...
    IntRespMCIter r_cit; size_t i;
    for (r_cit=resp_map.begin(), i=0; r_cit!=resp_map.end(); ++r_cit, ++i) {
      const RealVector& fns_i = r_cit->second.function_values();
      copy_data(fns_i, qoi_sets+num_fns*i, num_fns);
    }
  }

  return 0;
}


void NonDC3FunctionTrain::post_run(std::ostream& s)
{
    // Statistics are generated here and output in print_results() below
    // if (statsFlag) // calculate statistics on allResponses
    //     compute_statistics(allSamples, allResponses);

    Analyzer::post_run(s);
}

void NonDC3FunctionTrain::compute_analytic_statistics()
{
// *** TO DO: push these into C3Approximation::compute_total_effects();
//     rely on NonDExpansion::compute_analytic_statistics()
// --> need to pass in vbdOrderLimit (PCE accesses via
//     data_rep->expConfigOptions.vbdOrderLimit)

// Also why is there a non-virtual Analyzer::print_sobol_indices() ?
  
  /*
    NonDExpansion::compute_analytic_statistics();

    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    C3Approximation* poly_approx_rep;
    // my own global sensitivities
    for (size_t ii = 0; ii < numFunctions; ii++){
        poly_approx_rep = (C3Approximation*)poly_approxs[ii].approx_rep();
        if (vbdFlag) {
            if ((vbdOrderLimit != 1) && (vbdOrderLimit != 0)){
                poly_approx_rep->compute_all_sobol_indices(vbdOrderLimit); 
            }
            else{
                poly_approx_rep->compute_all_sobol_indices(numContStateVars);

            }
        }
    }
  */
}

void NonDC3FunctionTrain::print_moments(std::ostream& s)
{
  s << std::scientific << std::setprecision(write_precision);

  // std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();
  size_t i, j, width = write_precision+7;

  s << "\nMoment statistics for each response function:\n";
  // Handle cases of both expansion/numerical moments or only one or the other:
  //   both exp/num: SC and PCE with numerical integration
  //   exp only:     PCE with unstructured grids (regression, exp sampling)
  // Also handle numerical exception of negative variance in either exp or num
  bool exception = false, prev_exception = false;

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();  
  for (size_t i=0; i<numFunctions; ++i) {
      C3Approximation* poly_approx_rep_i = (C3Approximation*)poly_approxs[i].approx_rep();
       if (i==0 || !prev_exception){
           s << std::setw(width+15) << "Mean" << std::setw(width+1) << "Std Dev"
             << std::setw(width+1)  << "Skewness" << std::setw(width+2)
             << "Kurtosis\n";
       }
       RealVector moments = poly_approx_rep_i->moments();
       s <<  fn_labels[i] << '\n' << std::setw(14) << "analytical: ";
       s << ' ' << std::setw(width) << moments[0]
         << ' ' << std::setw(width) << sqrt(moments[1])
         << ' ' << std::setw(width) << poly_approx_rep_i->skewness()
         << ' ' << std::setw(width) << poly_approx_rep_i->kurtosis()-3;
      s << '\n';
  }
}

struct SPrintArgs
{
    std::ostream *s;
    StringMultiArrayConstView * cv_labels;
    double variance;
    
};
    
void print_c3_sobol_indices(double value, size_t ninteract, size_t * interactions, void * arg)
{
    if (ninteract > 1){
        struct SPrintArgs * pa = (struct SPrintArgs *)arg;

        StringMultiArrayConstView cv_label = *(pa->cv_labels);
    
        String label;
        for (size_t jj = 0; jj < ninteract; jj++){
            label += cv_label[interactions[jj]] + " ";
        }
        *(pa->s) << "                     " << std::setw(write_precision+7) << value/pa->variance;
        *(pa->s) << ' ' << label << '\n';
    }
    
}
    
void NonDC3FunctionTrain::print_sobol_indices(std::ostream& s)
{
    s << "\nGlobal sensitivity indices for each response function:\n";

    const StringArray& fn_labels = iteratedModel.response_labels();

    StringMultiArrayConstView cv_labels = iteratedModel.continuous_variable_labels();

    // print sobol indices per response function
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    for (size_t i=0; i<numFunctions; ++i) {
        C3Approximation* poly_approx_rep_i = (C3Approximation*)poly_approxs[i].approx_rep();
        
        // Print Main and Total effects
        s << fn_labels[i] << " Sobol' indices:\n" << std::setw(38) << "Main"
          << std::setw(19) << "Total\n";
        
        RealVector moments = poly_approx_rep_i->moments();
        Real var = moments(1);
        
        for (size_t j=0; j<numContinuousVars; ++j) {
            s << "                     "   <<        std::setw(write_precision+7)
              << poly_approx_rep_i->main_sobol_index(j)/var << ' ' << std::setw(write_precision+7)
              << poly_approx_rep_i->total_sobol_index(j) << ' ' << cv_labels[j] << '\n';
        }

	/*  Similarly, integrate this into NonDExpansion workflow
        // Print Interaction effects
        if (vbdOrderLimit != 1) { 
            s << std::setw(39) << "Interaction\n";
            StringMultiArrayConstView cv_labels
                = iteratedModel.continuous_variable_labels();
            
            struct SPrintArgs pa;
            pa.s = &s;
            pa.cv_labels = &cv_labels;
            pa.variance = var;

            poly_approx_rep_i->sobol_iterate_apply(print_c3_sobol_indices,&pa);
        }
	*/
    }
}

} // namespace Dakota
