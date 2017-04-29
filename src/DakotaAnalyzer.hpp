/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Analyzer
//- Description:  Base class for NonD, DACE, and ParamStudy branches of the
//-               iterator hierarchy.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaAnalyzer.hpp 7035 2010-10-22 21:45:39Z mseldre $

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

  int num_samples() const;

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
  virtual void get_parameter_sets(Model& model, const int num_samples, 
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

  void print_results(std::ostream& s);

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
  void evaluate_parameter_sets(Model& model, bool log_resp_flag,
			       bool log_best_flag);

  /// generate replicate parameter sets for use in variance-based decomposition
  void get_vbd_parameter_sets(Model& model, int num_samples);

  /// compute VBD-based Sobol indices
  void compute_vbd_stats(const int num_samples, 
			 const IntResponseMap& resp_samples);

  /// convenience function for reading variables/responses (used in
  /// derived classes post_input)
  void read_variables_responses(int num_evals, size_t num_vars);

  /// Printing of VBD results
  void print_sobol_indices(std::ostream& s) const;

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
  /// array of headers to insert into output while evaluating allVariables
  StringArray allHeaders;

  // Data needed for update_best() so that param studies can be used in
  // strategies such as MultilevelOptStrategy

  size_t numObjFns;   ///< number of objective functions
  size_t numLSqTerms; ///< number of least squares terms

  /// map which stores best set of solutions
  RealPairPRPMultiMap bestVarsRespMap;

private:

  //
  //- Heading: Convenience functions
  //

  /// compares current evaluation to best evaluation and updates best
  void compute_best_metrics(const Response& response,
			    std::pair<Real,Real>& metrics);
  /// compares current evaluation to best evaluation and updates best
  void update_best(const Variables& vars, int eval_id,
		   const Response& response);
  /// compares current evaluation to best evaluation and updates best
  void update_best(const Real* sample_c_vars, int eval_id,
		   const Response& response);

  //
  //- Heading: Data
  //

  /// write precision as specified by the user
  int writePrecision;

  /// tolerance for omitting output of small VBD indices
  Real vbdDropTol;
  /// VBD main effect indices
  RealVectorArray S4;
  /// VBD total effect indices
  RealVectorArray T4;
};


inline Analyzer::Analyzer()
{ }


inline Analyzer::~Analyzer() { }


/** Return current number of evaluation points.  Since the calculation
    of samples, collocation points, etc. might be costly, provide a default
    implementation here that backs out from the maxEvalConcurrency. */
inline int Analyzer::num_samples() const
{ return maxEvalConcurrency / iteratedModel.derivative_concurrency(); }


inline const VariablesArray& Analyzer::all_variables()
{
  //  if (compactMode) samples_to_variables_array(allSamples, allVariables);
  return allVariables;
}


inline const RealMatrix& Analyzer::all_samples()
{
  //  if (!compactMode) variables_array_to_samples(allVariables, allSamples);
  return allSamples;
}


inline const IntResponseMap& Analyzer::all_responses() const
{ return allResponses; }


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

} // namespace Dakota

#endif
