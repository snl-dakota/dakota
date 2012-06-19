/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ApproximationInterface
//- Description:  Abstract base class for approximation classes used
//-               to provide approximate function evaluations
//- Owner:        Mike Eldred
//- Version: $Id: ApproximationInterface.H 7034 2010-10-22 20:16:32Z mseldre $

#ifndef APPROXIMATION_INTERFACE_H
#define APPROXIMATION_INTERFACE_H

#include "DakotaApproximation.H"
#include "DakotaInterface.H"
#include "DakotaVariables.H"
#include "DakotaResponse.H"


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
			 size_t num_fns);
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
	   const bool asynch_flag = false);

  /// returns the minimum number of samples required to build the
  /// functionSurfaces
  int minimum_points(bool constraint_flag) const;

  /// returns the recommended number of samples recommended to build the
  /// functionSurfaces
  int recommended_points(bool constraint_flag) const;

  void approximation_function_indices(const IntSet& approx_fn_indices);

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

  void rebuild_approximation(const BoolDeque& rebuild_deque);
  void pop_approximation(bool save_surr_data);
  void restore_approximation();
  bool restore_available();
  void finalize_approximation();

  void store_approximation();
  void combine_approximation(short corr_type);

  void clear_current();
  void clear_all();

  std::vector<Approximation>& approximations();
  const Pecos::SurrogateData& approximation_data(size_t index);

  const RealVectorArray& approximation_coefficients();
  void approximation_coefficients(const RealVectorArray& approx_coeffs);

  const RealVector& approximation_variances(const Variables& vars);

  // mimic asynchronous operations for those iterators which call
  // asynch_compute_response and synchronize/synchronize_nowait on an
  // approximateModel
  const IntResponseMap& synch();
  const IntResponseMap& synch_nowait();

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

  /// update the popCount within each of the functionSurfaces based on the
  /// active set definitions within resp_map
  void update_pop_counts(const IntResponseMap& resp_map);

  //
  //- Heading: Data
  //

  /// for incomplete approximation sets, this array specifies the
  /// response function subset that is approximated
  IntSet approxFnIndices;

  /// list of approximations, one per response function
  /** This formulation allows the use of mixed approximations (i.e.,
      different approximations used for different response functions),
      although the input specification is not currently general enough
      to support it. */
  std::vector<Approximation> functionSurfaces;

  /// array of approximation coefficient vectors, one vector per
  /// response function
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

  /// set of diagnostic metrics
  StringArray diagnosticSet;

  /// copy of the actualModel variables object used to simplify conversion 
  /// among differing variable views
  Variables actualModelVars;
  /// indicates usage of an evaluation cache by the actualModel
  bool actualModelCache;
  /// the interface id from the actualModel used for ordered PRPCache lookups
  String actualModelInterfaceId;

  /// bookkeeping map to catalogue responses generated in map() for use
  /// in synch() and synch_nowait().  This supports pseudo-asynchronous
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
    rec_points = 
      std::max(rec_points, 
	       functionSurfaces[*cit].recommended_points(constraint_flag));
  return rec_points;
}


inline void ApproximationInterface::
approximation_function_indices(const IntSet& approx_fn_indices)
{ approxFnIndices = approx_fn_indices; }


inline void ApproximationInterface::clear_current()
{
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); it++)
    functionSurfaces[*it].clear_current();
}


inline void ApproximationInterface::clear_all()
{
  for (ISIter it=approxFnIndices.begin(); it!=approxFnIndices.end(); it++)
    functionSurfaces[*it].clear_all();
}


inline std::vector<Approximation>& ApproximationInterface::approximations()
{ return functionSurfaces; }


inline const Pecos::SurrogateData& ApproximationInterface::
approximation_data(size_t index)
{
  if (!approxFnIndices.count(index)) {
    Cerr << "Error: index passed to ApproximationInterface::approximation_data"
	 << "() does not correspond to an approximated function." << std::endl;
    abort_handler(-1);
  }
  return functionSurfaces[index].approximation_data();
}

} // namespace Dakota

#endif
