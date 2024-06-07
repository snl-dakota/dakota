/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_ANALYZER_H
#define DAKOTA_ANALYZER_H

#include "dakota_data_types.hpp"
#include "DakotaIterator.hpp"
#include "ParamResponsePair.hpp"
//#include "DakotaResponse.hpp"


namespace Dakota {

/// Base class for NonD, DACE, and ParamStudy branches of the iterator
/// hierarchy.

/** The Analyzer class provides common data and functionality for
    various types of systems analysis, including nondeterministic
    analysis, design of experiments, and parameter studies. */

class Analyzer: public Iterator
{
public:

  //
  //- Heading: Virtual member function redefinitions
  //

  const VariablesArray& all_variables();
  const RealMatrix&     all_samples();
  const IntResponseMap& all_responses() const;
  bool resize();

  size_t num_samples() const;

  //
  //- Heading: Virtual functions
  //

  /// sets varyPattern in derived classes that support it
  virtual void vary_pattern(bool pattern_flag);


protected:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  Analyzer();
  /// standard constructor
  Analyzer(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for instantiations "on the fly" with a Model
  Analyzer(unsigned short method_name, Model& model);
  /// alternate constructor for instantiations "on the fly" with a Model
  Analyzer(unsigned short method_name, Model& model,
	   const ShortShortPair& view_override);
  /// alternate constructor for instantiations "on the fly" without a Model
  Analyzer(unsigned short method_name);
  /// destructor
  ~Analyzer();

  //
  //- Heading: Virtual functions
  //

  /// Generate one block of numSamples samples (ndim * num_samples),
  /// populating allSamples; ParamStudy is the only class that
  /// specializes to use allVariables
  virtual void get_parameter_sets(Model& model);

  // Can we alleviate the need to pass the number of samples?

  /// Generate one block of numSamples samples (ndim * num_samples),
  /// populating design_matrix
  virtual void get_parameter_sets(Model& model, const size_t num_samples, 
				  RealMatrix& design_matrix);

  /// update model's current variables with data from sample
  virtual void update_model_from_sample(Model& model, const Real* sample_vars);
  /// update model's current variables with data from vars
  virtual void update_model_from_variables(Model& model, const Variables& vars);

  /// convert column of samples array to variables; derived classes
  /// may reimplement for more than active continuous variables
  virtual void sample_to_variables(const Real* sample_vars, Variables& vars);

  //
  //- Heading: Virtual member function redefinitions
  //

  void update_from_model(const Model& model);

  void initialize_run();
  void pre_run();
  void post_run(std::ostream& s);
  void finalize_run();

  void pre_output();

  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  const Model& algorithm_space_model() const;

  const Variables&      variables_results() const;
  const Response&       response_results()  const;
  const VariablesArray& variables_array_results();
  const ResponseArray&  response_array_results();

  void response_results_active_set(const ActiveSet& set);

  bool compact_mode() const;
  bool returns_multiple_points() const;

  //
  //- Heading: Member functions
  //

  /// perform function evaluations to map parameter sets (allVariables)
  /// into response sets (allResponses)
  void evaluate_parameter_sets(Model& model, bool log_resp_flag = true,
			       bool log_best_flag = false);

  /// perform function evaluations to map a keyed batch of parameter sets
  /// (allVariablesMap[key]) into a corresponding batch of response sets
  /// (allResponsesMap[key])
  void evaluate_batch(Model& model, int batch_id, bool log_best_flag = false);
  /// perform function evaluations to map a keyed batch of parameter sets
  /// (allVariablesMap[key]) into a corresponding batch of response sets
  /// (allResponsesMap[key])
  const IntIntResponse2DMap& synchronize_batches(Model& model,
						 bool log_best_flag = false);
  /// since synchronize returns the aggregation of all evaluated batches,
  /// we use a separate call to indicate when processing of this data is
  /// complete and bookkeeping can be cleared
  void clear_batches();

  /// generate replicate parameter sets for use in variance-based decomposition
  void get_vbd_parameter_sets(Model& model, size_t num_samples);

  /// archive model evaluation points
  virtual void archive_model_variables(const Model&, size_t idx) const
    { /* no-op */ }

  /// archive model evaluation responses
  virtual void archive_model_response(const Response&, size_t idx) const
    { /* no-op */ }

  /// convenience function for reading variables/responses (used in
  /// derived classes post_input)
  void read_variables_responses(int num_evals, size_t num_vars);

  /// convert samples array to variables array; e.g., allSamples to allVariables
  void samples_to_variables_array(const RealMatrix& sample_matrix,
				  VariablesArray& vars_array);
  /// convert the active continuous variables into a column of allSamples
  virtual void variables_to_sample(const Variables& vars, Real* sample_c_vars);

  /// convert variables array to samples array; e.g., allVariables to allSamples
  void variables_array_to_samples(const VariablesArray& vars_array,
				  RealMatrix& sample_matrix);

  //
  //- Heading: Data
  //

  // Isolate complexity by letting Model::currentVariables/currentResponse
  // manage details.  Then Iterator only needs the following:
  size_t numFunctions;          ///< number of response functions
  size_t numContinuousVars;     ///< number of active continuous vars
  size_t numDiscreteIntVars;    ///< number of active discrete integer vars
  size_t numDiscreteStringVars; ///< number of active discrete string vars
  size_t numDiscreteRealVars;   ///< number of active discrete real vars

  /// switch for allSamples (compact mode) instead of allVariables (normal mode)
  bool compactMode;

  /// array of all variables to be evaluated in evaluate_parameter_sets()
  VariablesArray allVariables;
  /// compact alternative to allVariables
  RealMatrix allSamples;
  /// array of all responses to be computed in evaluate_parameter_sets()
  IntResponseMap allResponses;

  /// alternate container for Variables samples supporting evaluate_batch() and
  /// synchronize_batches(), a 2D map with outer batch_id and inner eval_id
  IntIntVariables2DMap batchVariablesMap;
  /// alternate container for RealVector samples supporting evaluate_batch()
  /// and synchronize_batches(), a 2D map with outer batch_id and inner eval_id
  IntIntRealVector2DMap batchSamplesMap;
  /// alternate container for Response samples supporting evaluate_batch() and
  /// synchronize_batches(), a 2D map with outer batch_id and inner eval_id
  IntIntResponse2DMap batchResponsesMap;

  /// array of headers to insert into output while evaluating allVariables
  StringArray allHeaders;

  // Data needed for update_best() so that param studies can be used in
  // strategies such as MultilevelOptStrategy

  size_t numObjFns;   ///< number of objective functions
  size_t numLSqTerms; ///< number of least squares terms

  /// map which stores best set of solutions
  RealPairPRPMultiMap bestVarsRespMap;

  /// flag indicating the activation of variance-bsaed decomposition
  /// for computing Sobol' indices, via either PCE or sampling
  bool vbdFlag;

  /// tolerance for omitting output of small VBD indices computed via
  /// either PCE or sampling
  Real vbdDropTol;

private:

  //
  //- Heading: Convenience functions
  //

  /// layer a RecastModel on top of iteratedModel to enact a view override
  void recast_model_view(const ShortShortPair& view_override);

  /// compares current evaluation to best evaluation and updates best
  void compute_best_metrics(const Response& response,
			    std::pair<Real,Real>& metrics);
  /// compares current evaluation to best evaluation and updates best
  void update_best(const Variables& vars, int eval_id,
		   const Response& response);
  /// compares current evaluation to best evaluation and updates best
  void update_best(const Real* sample_c_vars, int eval_id,
		   const Response& response);

  /// for current response, update best result and archive
  void log_response(const Model& model, IntResponseMap& resp_map, size_t i,
		    bool log_resp_flag, bool log_best_flag);
  /// update best result and archive all results
  void log_response_map(const RealMatrix& samples,
			const IntResponseMap& resp_map, bool log_best_flag);
  /// update best result and archive all results
  void log_response_map(const VariablesArray& vars_array,
			const IntResponseMap& resp_map, bool log_best_flag);
  /// update best result and archive all results
  void log_response_map(const IntRealVectorMap& rv_map,
			const IntResponseMap& resp_map, bool log_best_flag);
  /// update best result and archive all results
  void log_response_map(const IntVariablesMap& vars_map,
			const IntResponseMap& resp_map, bool log_best_flag);

  //
  //- Heading: Data
  //

  /// write precision as specified by the user
  int writePrecision;
};


inline Analyzer::Analyzer()
{ }


inline Analyzer::~Analyzer() { }


/** Return current number of evaluation points.  Since the calculation
    of samples, collocation points, etc. might be costly, provide a default
    implementation here that backs out from the maxEvalConcurrency. */
inline size_t Analyzer::num_samples() const
{ return maxEvalConcurrency / iteratedModel.derivative_concurrency(); }


inline const VariablesArray& Analyzer::all_variables()
{
  // BMA TODO: Consider different accessor or option parameter for conversion
  if (compactMode) samples_to_variables_array(allSamples, allVariables);
  return allVariables;
}


inline const RealMatrix& Analyzer::all_samples()
{
  //  if (!compactMode) variables_array_to_samples(allVariables, allSamples);
  return allSamples;
}


inline const IntResponseMap& Analyzer::all_responses() const
{ return allResponses; }


inline void Analyzer::clear_batches()
{
  batchResponsesMap.clear(); batchSamplesMap.clear(); batchVariablesMap.clear();
  //allResponses.clear(); allSamples.clear(); allVariables.clear();
}


/** default definition that gets redefined in selected derived Minimizers */
inline const Model& Analyzer::algorithm_space_model() const
{ return iteratedModel; }


inline const Variables& Analyzer::variables_results() const
{ return bestVarsRespMap.begin()->second.variables(); }


inline const VariablesArray& Analyzer::variables_array_results()
{
  //multi_map_to_variables_array(bestVarsRespMap, bestVariablesArray);
  bestVariablesArray.resize(bestVarsRespMap.size());
  RealPairPRPMultiMap::const_iterator cit; size_t i;
  for (cit=bestVarsRespMap.begin(), i=0; cit!=bestVarsRespMap.end(); ++cit, ++i)
    bestVariablesArray[i] = cit->second.variables();
  return bestVariablesArray;
}


inline const Response& Analyzer::response_results() const
{ return bestVarsRespMap.begin()->second.response(); }


inline const ResponseArray& Analyzer::response_array_results()
{
  //multi_map_to_response_array(bestVarsRespMap, bestResponseArray);
  bestResponseArray.resize(bestVarsRespMap.size());
  RealPairPRPMultiMap::const_iterator cit; size_t i;
  for (cit=bestVarsRespMap.begin(), i=0; cit!=bestVarsRespMap.end(); ++cit, ++i)
    bestResponseArray[i] = cit->second.response();
  return bestResponseArray;
}

inline bool Analyzer::returns_multiple_points() const
{ return true; }


inline void Analyzer::response_results_active_set(const ActiveSet& set)
{ bestVarsRespMap.begin()->second.active_set(set); }


inline bool Analyzer::compact_mode() const
{ return compactMode; }


inline void Analyzer::
log_response(const Model& model, IntResponseMap& resp_map, size_t i,
	     bool log_resp_flag, bool log_best_flag)
{
  const Response& resp = model.current_response();
  int eval_id = model.evaluation_id();
  if (log_best_flag) update_best(model.current_variables(), eval_id, resp);
  if (log_resp_flag) resp_map[eval_id] = resp.copy();
  archive_model_response(resp, i);
}


inline void Analyzer::
log_response_map(const RealMatrix& samples, const IntResponseMap& resp_map,
		 bool log_best_flag)
{
  bool db_act = resultsDB.active();
  if (!log_best_flag && !db_act) return;

  IntRespMCIter r_cit;  size_t i;
  for (i=0, r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++i, ++r_cit) {
    if (log_best_flag) update_best(samples[i], r_cit->first, r_cit->second);
    if (db_act)        archive_model_response(r_cit->second, i);
  }
}


inline void Analyzer::
log_response_map(const VariablesArray& vars_array,
		 const IntResponseMap& resp_map, bool log_best_flag)
{
  bool db_act = resultsDB.active();
  if (!log_best_flag && !db_act) return;

  IntRespMCIter r_cit;  size_t i;
  for (i=0, r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++i, ++r_cit) {
    if (log_best_flag) update_best(vars_array[i], r_cit->first, r_cit->second);
    if (db_act)        archive_model_response(r_cit->second, i);
  }
}


inline void Analyzer::
log_response_map(const IntRealVectorMap& rv_map,
		 const IntResponseMap& resp_map, bool log_best_flag)
{
  bool db_act = resultsDB.active();
  if (!log_best_flag && !db_act) return;

  IntRVMCIter v_cit;  IntRespMCIter r_cit;  size_t i;
  for (i=0, v_cit=rv_map.begin(), r_cit=resp_map.begin();
       v_cit!=rv_map.end() && r_cit!=resp_map.end(); ++i, ++v_cit, ++r_cit) {
    if (log_best_flag)
      update_best(v_cit->second.values(), r_cit->first, r_cit->second);
    if (db_act)
      archive_model_response(r_cit->second, i);
  }
}


inline void Analyzer::
log_response_map(const IntVariablesMap& vars_map,
		 const IntResponseMap& resp_map, bool log_best_flag)
{
  bool db_act = resultsDB.active();
  if (!log_best_flag && !db_act) return;

  IntVarsMCIter v_cit;  IntRespMCIter r_cit;  size_t i;
  for (i=0, v_cit=vars_map.begin(), r_cit=resp_map.begin();
       v_cit!=vars_map.end() && r_cit!=resp_map.end(); ++i, ++v_cit, ++r_cit) {
    if (log_best_flag) update_best(v_cit->second, r_cit->first, r_cit->second);
    if (db_act)        archive_model_response(r_cit->second, i);
  }
}

} // namespace Dakota

#endif
