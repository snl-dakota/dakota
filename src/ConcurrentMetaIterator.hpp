/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ConcurrentMetaIterator
//- Description: Class for concurrent iteration: pareto_set and multi_start
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: ConcurrentMetaIterator.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef CONCURRENT_META_ITERATOR_H
#define CONCURRENT_META_ITERATOR_H

#include "MetaIterator.hpp"
#include "dakota_data_io.hpp"
#include "ParamResponsePair.hpp"


namespace Dakota {

/// Meta-iterator for multi-start iteration or pareto set optimization.

/** This meta-iterator maintains two concurrent iterator capabilities.
    First, a general capability for running an iterator multiple times
    from different starting points is provided (often used for
    multi-start optimization, but not restricted to optimization).
    Second, a simple capability for mapping the "pareto frontier" (the
    set of optimal solutions in multiobjective formulations) is
    provided.  This pareto set is mapped through running an optimizer
    multiple times for different sets of multiobjective weightings. */

class ConcurrentMetaIterator: public MetaIterator
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  ConcurrentMetaIterator(ProblemDescDB& problem_db);
  /// alternate constructor
  ConcurrentMetaIterator(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~ConcurrentMetaIterator();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// Performs the concurrent iteration by executing selectedIterator
  /// on iteratedModel multiple times in parallel for different
  /// settings within the iterator or model.
  void core_run();
  void print_results(std::ostream& s);

  void initialize_iterator(int job_index);
  void pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer);
  void pack_results_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index);
  void update_local_results(int job_index);

  const Model& algorithm_space_model() const;

private:

  //
  //- Heading: Convenience member functions
  //

  /// shared constructor code
  void initialize(int param_set_len);

  /// called by unpack_parameters_buffer(MPIUnpackBuffer) and
  /// initialize_iterator(int) to update iteratedModel and selectedIterator
  void initialize_iterator(const RealVector& param_set);

  /// initialize the iterated Model prior to Iterator instantiation
  /// and define param_set_len
  void initialize_model(int& param_set_len);

  //
  //- Heading: Data members
  //

  Iterator selectedIterator; ///< the iterator selected for concurrent iteration
  bool     lightwtCtor;      ///< indicates use of lightweight Iterator ctors

  /// the initial continuous variables for restoring the starting
  /// point in the Pareto set minimization
  RealVector initialPt;

  /// an array of parameter set vectors (either multistart variable
  /// sets or pareto multi-objective/least squares weighting sets) to
  /// be performed.
  RealVectorArray parameterSets;
  /// 1-d array of ParamResponsePair results corresponding to numIteratorJobs
  PRPArray prpResults;
};


inline const Model& ConcurrentMetaIterator::algorithm_space_model() const
{ return iteratedModel; }


inline void ConcurrentMetaIterator::initialize_model(int& param_set_len)
{
  if (methodName == PARETO_SET) {
    param_set_len = probDescDB.get_sizet("responses.num_objective_functions");
    // define dummy weights to trigger model recasting in iterator construction
    // (replaced at run-time with weight sets from specification)
    if (iteratedModel.primary_response_fn_weights().empty()) {
      RealVector initial_wts(param_set_len, false);
      initial_wts = 1./(Real)param_set_len;
      iteratedModel.primary_response_fn_weights(initial_wts); // trigger recast
    }
  }
  else
    param_set_len = iteratedModel.cv();
}


inline void ConcurrentMetaIterator::
initialize_iterator(const RealVector& param_set)
{
  if (methodName == MULTI_START)
    iteratedModel.continuous_variables(param_set);
  else {
    iteratedModel.continuous_variables(initialPt); // reset
    iteratedModel.primary_response_fn_weights(param_set);
  }
}


inline void ConcurrentMetaIterator::initialize_iterator(int job_index)
{ initialize_iterator(parameterSets[job_index]); }


inline void ConcurrentMetaIterator::
pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index)
{ send_buffer << parameterSets[job_index]; }


inline void ConcurrentMetaIterator::
unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer)
{
  RealVector param_set;
  recv_buffer >> param_set;
  initialize_iterator(param_set);
}


inline void ConcurrentMetaIterator::
pack_results_buffer(MPIPackBuffer& send_buffer, int job_index)
{ send_buffer << prpResults[job_index]; }


inline void ConcurrentMetaIterator::
unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{ recv_buffer >> prpResults[job_index]; }


inline void ConcurrentMetaIterator::update_local_results(int job_index)
{
  prpResults[job_index]
    = ParamResponsePair(selectedIterator.variables_results(),
			iteratedModel.interface_id(),
			selectedIterator.response_results(),
			job_index+1); // deep copy
}

} // namespace Dakota

#endif
