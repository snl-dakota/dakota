/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        Analyzer
//- Description:  Base class for NonD, DACE, and ParamStudy branches of the
//-               iterator hierarchy.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaAnalyzer.H 7035 2010-10-22 21:45:39Z mseldre $

#ifndef DAKOTA_ANALYZER_H
#define DAKOTA_ANALYZER_H

#include "data_types.h"
#include "DakotaIterator.H"
#include "ParamResponsePair.H"
//#include "DakotaResponse.H"


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
  const ResponseArray&  all_responses() const;

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  Analyzer();
  /// standard constructor
  Analyzer(Model& model);
  /// alternate constructor for instantiations "on the fly" with a Model
  Analyzer(NoDBBaseConstructor, Model& model);
  /// alternate constructor for instantiations "on the fly" without a Model
  Analyzer(NoDBBaseConstructor);
  /// destructor
  ~Analyzer();

  //
  //- Heading: Virtual functions
  //

  /// sets varyPattern in derived classes that support it
  virtual void vary_pattern(bool pattern_flag);

  /// Returns one block of samples (ndim * num_samples)
  virtual void get_parameter_sets(Model& model);

  /// update model's current variables with data from sample
  virtual void update_model_from_sample(Model& model, const Real* sample_vars);
  /// update model's current variables with data from vars
  virtual void update_model_from_variables(Model& model, const Variables& vars);

  //
  //- Heading: Virtual member function redefinitions
  //

  void pre_output();

  void print_results(std::ostream& s);

  const Variables&      variables_results() const;
  const Response&       response_results()  const;
  const VariablesArray& variables_array_results();
  const ResponseArray&  response_array_results();

  void response_results_active_set(const ActiveSet& set);

  bool compact_mode() const;

  //
  //- Heading: Member functions
  //

  /// perform function evaluations to map parameter sets (allVariables)
  /// into response sets (allResponses)
  void evaluate_parameter_sets(Model& model, bool log_resp_flag,
			       bool log_best_flag);

  void variance_based_decomp(int ncont, int ndiscint, int ndiscreal,
			     int num_samples);
 
  /// convenience function for reading variables/responses (used in
  /// derived classes post_input)
  void read_variables_responses(int num_evals, size_t num_vars);

  /// Printing of VBD results
  void print_sobol_indices(std::ostream& s, const RealVectorArray& S,
		 const RealVectorArray& T) const;

  /// convert samples array to variables array; e.g., allSamples to allVariables
  void sample_to_variables(const Real* sample_c_vars, Variables& vars);
  /// convert samples array to variables array; e.g., allSamples to allVariables
  void samples_to_variables_array(const RealMatrix& sample_matrix,
				  VariablesArray& vars_array);
  /// convert variables array to samples array; e.g., allVariables to allSamples
  void variables_array_to_samples(const VariablesArray& vars_array,
				  RealMatrix& sample_matrix);

  //
  //- Heading: Data
  //

  /// switch for allSamples (compact mode) instead of allVariables (normal mode)
  bool compactMode;

  /// array of all variables to be evaluated in evaluate_parameter_sets()
  VariablesArray allVariables;
  /// compact alternative to allVariables
  RealMatrix allSamples;
  /// array of all responses to be computed in evaluate_parameter_sets()
  ResponseArray allResponses;
  /// array of headers to insert into output while evaluating allVariables
  StringArray allHeaders;

  // Data needed for update_best() so that param studies can be used in
  // strategies such as MultilevelOptStrategy

  size_t numObjFns;   ///< number of objective functions
  size_t numLSqTerms; ///< number of least squares terms

  /// number of solutions to retain in best variables/response arrays
  size_t numBest;
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
  /// tolerance for omitting output of small VBD indices  Real vbdDropTol;
  Real vbdDropTol;
};


inline Analyzer::Analyzer()
{ }


inline Analyzer::~Analyzer() { }


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


inline const ResponseArray& Analyzer::all_responses() const
{ return allResponses; }


inline const Variables& Analyzer::variables_results() const
{ return bestVarsRespMap.begin()->second.prp_parameters(); }


inline const VariablesArray& Analyzer::variables_array_results()
{
  //multi_map_to_variables_array(bestVarsRespMap, bestVariablesArray);
  bestVariablesArray.resize(bestVarsRespMap.size());
  RealPairPRPMultiMap::const_iterator cit; size_t i;
  for (cit=bestVarsRespMap.begin(), i=0; cit!=bestVarsRespMap.end(); ++cit, ++i)
    bestVariablesArray[i] = cit->second.prp_parameters();
  return bestVariablesArray;
}


inline const Response& Analyzer::response_results() const
{ return bestVarsRespMap.begin()->second.prp_response(); }


inline const ResponseArray& Analyzer::response_array_results()
{
  //multi_map_to_response_array(bestVarsRespMap, bestResponseArray);
  bestResponseArray.resize(bestVarsRespMap.size());
  RealPairPRPMultiMap::const_iterator cit; size_t i;
  for (cit=bestVarsRespMap.begin(), i=0; cit!=bestVarsRespMap.end(); ++cit, ++i)
    bestResponseArray[i] = cit->second.prp_response();
  return bestResponseArray;
}


inline void Analyzer::response_results_active_set(const ActiveSet& set)
{ bestVarsRespMap.begin()->second.active_set(set); }


inline bool Analyzer::compact_mode() const
{ return compactMode; }

} // namespace Dakota

#endif
