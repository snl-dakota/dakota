/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ApproximationInterface
//- Description:  derived Interface class that manages approximate function
//-               evaluations
//- Owner:        Mike Eldred
//- Version: $Id: ApproximationInterface.hpp 7034 2010-10-22 20:16:32Z mseldre $

#ifndef APPROXIMATION_INTERFACE_H
#define APPROXIMATION_INTERFACE_H

#include "SharedApproxData.hpp"
#include "DakotaApproximation.hpp"
#include "DakotaInterface.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"


namespace Dakota {


/// Derived class within the interface class hierarchy for supporting
/// approximations to simulation-based results.

/** ApproximationInterface provides an interface class for building a
    set of global/local/multipoint approximations and performing
    approximate function evaluations using them.  It contains a list
    of Approximation objects, one for each response function. */

class ApproximationInterface: public Interface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// primary constructor
  ApproximationInterface(ProblemDescDB& problem_db, const Variables& am_vars,
			 bool am_cache, const String& am_interface_id,
			 const StringArray& fn_labels);
  /// alternate constructor for instantiations on the fly
  ApproximationInterface(const String& approx_type,
			 const UShortArray& approx_order,
			 const Variables& am_vars, bool am_cache,
			 const String& am_interface_id, size_t num_fns,
			 short data_order, short output_level);
  /// destructor
  ~ApproximationInterface();

protected:

  //
  //- Heading: Methods (protected due to letter-envelope idiom)
  //

  /// the function evaluator: provides an approximate "mapping" from
  /// the variables to the responses using functionSurfaces
  void map(const Variables& vars, const ActiveSet& set, Response& response,
	   bool asynch_flag = false);

  /// returns the minimum number of samples required to build the
  /// functionSurfaces
  int minimum_points(bool constraint_flag) const;

  /// returns the recommended number of samples recommended to build the
  /// functionSurfaces
  int recommended_points(bool constraint_flag) const;

  void active_model_key(const UShortArray& key);
  void clear_model_keys();

  void surrogate_model_key(const UShortArray& key);
  void truth_model_key(const UShortArray& key);
  
  void approximation_function_indices(const IntSet& approx_fn_indices);

  void link_multilevel_approximation_data();

  void update_approximation(const Variables& vars,
			    const IntResponsePair& response_pr);
  void update_approximation(const RealMatrix& samples,
			    const IntResponseMap& resp_map);
  void update_approximation(const VariablesArray& vars_array,
			    const IntResponseMap& resp_map);

  void append_approximation(const Variables& vars,
			    const IntResponsePair& response_pr);
  void append_approximation(const RealMatrix& samples,
			    const IntResponseMap& resp_map);
  void append_approximation(const VariablesArray& vars_array,
			    const IntResponseMap& resp_map);

  void build_approximation(const RealVector&  c_l_bnds,
			   const RealVector&  c_u_bnds,
			   const IntVector&  di_l_bnds,
			   const IntVector&  di_u_bnds,
			   const RealVector& dr_l_bnds,
			   const RealVector& dr_u_bnds);

  void export_approximation();

  void rebuild_approximation(const BoolDeque& rebuild_deque);
  void pop_approximation(bool save_data);
  void push_approximation();
  bool push_available();
  void finalize_approximation();

  void combine_approximation();
  void combined_to_active(bool clear_combined = true);

  void clear_inactive();
  void clear_current_active_data();
  void clear_active_data();

  SharedApproxData& shared_approximation();
  std::vector<Approximation>& approximations();
  const Pecos::SurrogateData&
    approximation_data(size_t fn_index, size_t d_index = _NPOS);

  const RealVectorArray& approximation_coefficients(bool normalized = false);
  void approximation_coefficients(const RealVectorArray& approx_coeffs,
				  bool normalized = false);

  const RealVector& approximation_variances(const Variables& vars);

  Real2DArray cv_diagnostics(const StringArray& metrics, unsigned num_folds);
  Real2DArray challenge_diagnostics(const StringArray& metric_types,
				    const RealMatrix& challenge_pts,
				    const RealVector& challenge_resps);

  // mimic asynchronous operations for those iterators which call
  // asynch_compute_response and synchronize/synchronize_nowait on an
  // approximateModel
  const IntResponseMap& synchronize();
  const IntResponseMap& synchronize_nowait();

private:

  //
  //- Heading: Convenience functions
  //

  /// add variables/response data to functionSurfaces using a mixture
  /// of shallow and deep copies
  void mixed_add(const Variables& vars, const Response& response, bool anchor);
  /// add variables/response data to functionSurfaces using a mixture
  /// of shallow and deep copies
  void mixed_add(const Real* c_vars, const Response& response, bool anchor);
  /// add variables/response data to functionSurfaces using a shallow copy
  void shallow_add(const Variables& vars, const Response& response,
		   bool anchor);

  /// populate continuous variables within vars from sample_c_vars
  void sample_to_variables(const Real* sample_c_vars, size_t num_cv,
			   Variables& vars);

  /// append to the stack of pop counts within each of the functionSurfaces
  /// based on the active set definition within a single incoming response
  void update_pop_counts(const IntResponsePair& response_pr);
  /// append to the stack of pop counts within each of the functionSurfaces
  /// based on the active set definitions within a map of incoming responses
  void update_pop_counts(const IntResponseMap& resp_map);

  /// following add() and pop_count() operations which may enumerate
  /// multiple keys, restore the active approxData to the nominal key
  void restore_data_key();

  /// Load approximation test points from user challenge points file
  void read_challenge_points();

  //
  //- Heading: Data
  //

  /// counter for giving unique names to approximation interfaces
  static size_t approxIdNum;
  /// for incomplete approximation sets, this array specifies the
  /// response function subset that is approximated
  IntSet approxFnIndices;

  /// data that is shared among all functionSurfaces
  SharedApproxData sharedData;
  /// list of approximations, one per response function
  /** This formulation allows the use of mixed approximations (i.e.,
      different approximations used for different response functions),
      although the input specification is not currently general enough
      to support it. */
  std::vector<Approximation> functionSurfaces;

  /// array of approximation coefficient vectors, one per response function
  RealVectorArray functionSurfaceCoeffs;
  /// vector of approximation variances, one value per response function
  RealVector functionSurfaceVariances;

  // vector of approximation scalings from approx_scale_offset.in.  Provides
  // a capability to reuse existing surrogates with modified design goals.
  //RealVector approxScale;
  // vector of approximation offsets from approx_scale_offset.in.  Provides
  // a capability to reuse existing surrogates with modified design goals.
  //RealVector approxOffset;

  //bool graph3DFlag; // controls 3D graphics of approximation surfaces

  /// data file for user-supplied challenge data (per interface, since may
  /// contain multiple responses)
  String challengeFile;
  /// tabular format of the challenge points file
  unsigned short challengeFormat;
  /// whether to validate variable labels in header
  bool challengeUseVarLabels;
  /// whether to import active only
  bool challengeActiveOnly;
  /// container for the challenge points data (variables only)
  RealMatrix challengePoints;
  /// container for the challenge points data (responses only)
  RealMatrix challengeResponses;

  /// copy of the actualModel variables object used to simplify conversion 
  /// among differing variable views
  Variables actualModelVars;
  /// indicates usage of an evaluation cache by the actualModel
  bool actualModelCache;
  /// the interface id from the actualModel used for ordered PRPCache lookups
  String actualModelInterfaceId;

  /// bookkeeping map to catalogue responses generated in map() for use in
  /// synchronize() and synchronize_nowait(). This supports pseudo-asynchronous
  /// operations (approximate responses are always computed synchronously,
  /// but asynchronous virtual functions are supported through bookkeeping).
  IntResponseMap beforeSynchResponseMap;
};


inline ApproximationInterface::~ApproximationInterface() { }


inline int ApproximationInterface::minimum_points(bool constraint_flag) const
{
  // minimum number of points required over all approximations (even though
  // different approximation types are not yet supported).  Recompute this at
  // the time needed, since it may vary (depending on presence of constraints).
  int min_points = 0;
  for (ISCIter cit=approxFnIndices.begin(); cit!=approxFnIndices.end(); cit++)
    min_points = std::max(min_points,
			  functionSurfaces[*cit].min_points(constraint_flag));
  return min_points;
}


inline int ApproximationInterface::
recommended_points(bool constraint_flag) const
{
  // recommended number of points required over all approximations (even though
  // different approximation types are not yet supported).  Recompute this at
  // the time needed, since it may vary (depending on presence of constraints).
  int rec_points = 0;
  for (ISCIter cit=approxFnIndices.begin(); cit!=approxFnIndices.end(); cit++)
    rec_points = std::max(rec_points, 
      functionSurfaces[*cit].recommended_points(constraint_flag));
  return rec_points;
}


inline void ApproximationInterface::active_model_key(const UShortArray& key)
{
  sharedData.active_model_key(key);

  // functionSurfaces access active key at run time through shared data; 
  // however they each contain their own approxData which must be updated.
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    functionSurfaces[*it].active_model_key(key);
}


inline void ApproximationInterface::clear_model_keys()
{
  sharedData.clear_model_keys();

  // No Approximation currently requires a default key assignment at construct
  // time: all key assignments can be performed at run time.  However, they
  // each contain their own approxData which must be cleared.
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    // Approximation::approxData instances: clear all keys
    functionSurfaces[*it].clear_model_keys();
}


inline void ApproximationInterface::surrogate_model_key(const UShortArray& key)
{
  //sharedData.surrogate_data_index(d_index); // if also passed
  sharedData.surrogate_model_key(key);
}


inline void ApproximationInterface::truth_model_key(const UShortArray& key)
{
  //sharedData.surrogate_data_index(d_index); // if also passed
  sharedData.truth_model_key(key);
}


/** Restore active key to leading key for first approxData
    (only updates model key if needed). */
inline void ApproximationInterface::restore_data_key()
{ active_model_key(sharedData.truth_model_key()); }


inline void ApproximationInterface::
approximation_function_indices(const IntSet& approx_fn_indices)
{ approxFnIndices = approx_fn_indices; }


inline void ApproximationInterface::link_multilevel_approximation_data()
{
  // define approx data keys and active index
  sharedData.link_multilevel_surrogate_data();

  // (create and) link SurrogateData instances
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    functionSurfaces[*it].link_multilevel_surrogate_data();
}


/** This function removes data provided by a previous append_approximation()
    call, possibly different numbers for each function, or as specified in
    pop_count, which is assumed to be the same for all functions. */
inline void ApproximationInterface::pop_approximation(bool save_data)
{
  sharedData.pop(save_data); // operation order not currently important

  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    Approximation& fn_surf = functionSurfaces[*it];
    // Approximation::approxData (1 or more keys for 1 or more instances)
    fn_surf.pop_data(save_data);
    // Approximation coefficients
    fn_surf.pop_coefficients(save_data);
  }
}


/** This function updates the coefficients for each Approximation based
    on data increments provided by {update,append}_approximation(). */
inline void ApproximationInterface::push_approximation()
{
  sharedData.pre_push(); // do shared aggregation first

  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    Approximation& fn_surf = functionSurfaces[*it];
    // Approximation::approxData (1 or more keys for 1 or more instances)
    fn_surf.push_data(); // uses shared restoration index
    // Approximation coefficients
    fn_surf.push_coefficients();
  }

  sharedData.post_push(); // do shared cleanup last
}


inline bool ApproximationInterface::push_available()
{ return sharedData.push_available(); }


inline void ApproximationInterface::finalize_approximation()
{
  sharedData.pre_finalize(); // do shared aggregation first

  size_t fn_index, key_index, num_keys;
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    Approximation& fn_surf = functionSurfaces[*it];
    // Approximation::approxData (1 or more keys for 1 or more instances)
    fn_surf.finalize_data(); // uses shared finalization index
    // Approximation coefficients
    fn_surf.finalize_coefficients();
  }

  sharedData.post_finalize(); // do shared cleanup last
}


inline void ApproximationInterface::combine_approximation()
{
  sharedData.pre_combine(); // shared aggregation first

  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    functionSurfaces[*it].combine_coefficients();

  sharedData.post_combine(); // shared cleanup last
}


inline void ApproximationInterface::combined_to_active(bool clear_combined)
{
  sharedData.combined_to_active(clear_combined); // shared aggregation first

  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it)
    functionSurfaces[*it].combined_to_active_coefficients(clear_combined);
}


inline void ApproximationInterface::clear_inactive()
{
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); ++it) {
    Approximation& fn_surf = functionSurfaces[*it];
    // Approximation::approxData instances:
    fn_surf.clear_inactive_data(); // only retain 1st of active data keys
    // Approximation coefficients
    fn_surf.clear_inactive_coefficients();
  }

  sharedData.clear_inactive(); // shared cleanup last
}


inline void ApproximationInterface::clear_current_active_data()
{
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); it++)
    functionSurfaces[*it].clear_current_active_data();
}


inline void ApproximationInterface::clear_active_data()
{
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); it++)
    functionSurfaces[*it].clear_active_data();
}


inline SharedApproxData& ApproximationInterface::shared_approximation()
{ return sharedData; }


inline std::vector<Approximation>& ApproximationInterface::approximations()
{ return functionSurfaces; }


inline const Pecos::SurrogateData& ApproximationInterface::
approximation_data(size_t fn_index, size_t d_index)
{
  if (approxFnIndices.find(fn_index) == approxFnIndices.end()) {
    Cerr << "Error: index passed to ApproximationInterface::approximation_data"
	 << "() does not correspond to an approximated function." << std::endl;
    abort_handler(APPROX_ERROR);
  }
  return functionSurfaces[fn_index].surrogate_data(d_index);
}


inline void ApproximationInterface::
sample_to_variables(const Real* sample_c_vars, size_t num_cv, Variables& vars)
{
  if (vars.cv() == num_cv)
    for (size_t i=0; i<num_cv; ++i)
      vars.continuous_variable(sample_c_vars[i], i);
  else if (vars.acv() == num_cv)
    for (size_t i=0; i<num_cv; ++i)
      vars.all_continuous_variable(sample_c_vars[i], i);
  else {
    Cerr << "Error: size mismatch in ApproximationInterface::"
	 << "sample_to_variables()" << std::endl;
    abort_handler(APPROX_ERROR);
  }
}

} // namespace Dakota

#endif
