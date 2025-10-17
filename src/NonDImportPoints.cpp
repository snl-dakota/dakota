/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "NonDImportPoints.hpp"
#include "NonDSampling.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"

namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDImportPoints::NonDImportPoints(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib, std::shared_ptr<Model> model):
  NonDSampling(problem_db, parallel_lib, model), numResponseFunctions(0),
  vbdViaSamplingMethod(VBD_BINNED),
  vbdViaSamplingNumBins(probDescDB.get_int("method.vbd_via_sampling_num_bins")),
  importPointsFile(problem_db.get_string("method.import_points_file")),
  importPointsFormat(problem_db.get_ushort("method.import_points_format")),
  importUseVariableLabels(problem_db.get_bool("method.import_points.use_variable_labels")),
  importActiveVariablesOnly(problem_db.get_bool("method.import_points.active_only") )
{

  if (model->primary_fn_type() == GENERIC_FNS)
    numResponseFunctions = model->num_primary_fns();

  if ((vbdFlag == true) && 
      (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars)){
        Cerr << "\nError: discrete variables are not supported for "
        << "binned variance based decomposition.\n";
        abort_handler(METHOD_ERROR); 
  }

  initialize_final_statistics();

}


NonDImportPoints::~NonDImportPoints()
{ }


void NonDImportPoints::pre_run()
{

  if (subIteratorFlag){
    Cerr << "\nError: nested studies are not supported for import_points.\n";
    abort_handler(METHOD_ERROR);
  }

  import_points();

  NonDSampling::pre_run();


}

void NonDImportPoints::
import_points()
{
  // Temporary objects to use to read correct size vars/resp; use copies
  // so that read_data_tabular() does not alter state of vars/resp objects
  // in Models (especially important for non-active variables).
  Variables vars = iteratedModel->current_variables().copy(); 
  Response resp = iteratedModel->current_response().copy();
  PRPList prp_list;
  
  const std::string context_msg = "import_points";
  bool verbose = (outputLevel > NORMAL_OUTPUT);
  TabularIO::read_data_tabular(importPointsFile, context_msg, 
           vars, resp, prp_list, importPointsFormat, verbose, 
           importUseVariableLabels, importActiveVariablesOnly);

  numSamples = prp_list.size();
  int num_vars = numContinuousVars   + numDiscreteIntVars 
               + numDiscreteRealVars + numDiscreteStringVars;
  allSamples.shapeUninitialized(num_vars, numSamples);

  size_t i; PRPLIter prp_it;
  Variables iter_vars; Response iter_resp;
  for ( i=0, prp_it=prp_list.begin(); i<numSamples; ++i, ++prp_it ) {

   ParamResponsePair& pr = *prp_it;
   iter_vars = pr.variables(); iter_resp = pr.response();

   variables_to_sample(iter_vars, allSamples[i]);
   allResponses[pr.eval_id()] = iter_resp;

  }

  if (verbose) {
    Cout << "\nRead " << numSamples << " samples from file " << importPointsFile << "\n";
  }

}

void NonDImportPoints::post_run(std::ostream& s)
{

   if(vbdFlag) {
     // TNP TODO: hard-set the sampling method to binned.
      nonDSampCorr.compute_vbd_stats_via_sampling(vbdViaSamplingMethod,
                                                  vbdViaSamplingNumBins,
                                                  numFunctions,
                                                  numContinuousVars + numDiscreteIntVars + numDiscreteRealVars + numDiscreteStringVars,
                                                  numSamples,
                                                  allSamples,
                                                  allResponses);

      nonDSampCorr.archive_sobol_indices(run_identifier(),
                                         resultsDB,
                                         iteratedModel->current_variables().ordered_labels(ACTIVE_VARS),
                                         ModelUtils::response_labels(*iteratedModel),
                                         vbdDropTol); // set in DakotaAnalyzer constructor
    }

    compute_statistics(allSamples, allResponses);

    Analyzer::post_run(s);

}

void NonDImportPoints::print_results(std::ostream& s, short results_state)
{

    if (vbdFlag){
      nonDSampCorr.print_sobol_indices(s,
                                       iteratedModel->current_variables().ordered_labels(ACTIVE_VARS),
                                       ModelUtils::response_labels(*iteratedModel),
                                       vbdDropTol); // set in DakotaAnalyzer constructor
    }
    compute_statistics(allSamples, allResponses);
    int actual_samples = allSamples.numCols();
    print_header_and_statistics(s, actual_samples);
} 

} // namespace Dakota
