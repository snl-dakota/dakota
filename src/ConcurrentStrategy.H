/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ConcurrentStrategy
//- Description: Parallel branch and bound strategy for mixed 
//-              continuous/discrete applications
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: ConcurrentStrategy.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef CONCURRENT_STRATEGY_H
#define CONCURRENT_STRATEGY_H

#include "DakotaStrategy.H"
#include "DakotaModel.H"
#include "DakotaIterator.H"
#include "data_io.h"


namespace Dakota {

/// Strategy for multi-start iteration or pareto set optimization.

/** This strategy maintains two concurrent iterator capabilities.
    First, a general capability for running an iterator multiple times
    from different starting points is provided (often used for
    multi-start optimization, but not restricted to optimization).
    Second, a simple capability for mapping the "pareto frontier" (the
    set of optimal solutions in multiobjective formulations) is
    provided.  This pareto set is mapped through running an optimizer
    multiple times for different sets of multiobjective weightings. */

class ConcurrentStrategy: public Strategy
{
public:

  //
  //- Heading: Constructors and destructor
  //

  ConcurrentStrategy(ProblemDescDB& problem_db); ///< constructor
  ~ConcurrentStrategy();                         ///< destructor

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// Performs the concurrent strategy by executing selectedIterator
  /// on userDefinedModel multiple times in parallel for different
  /// settings within the iterator or model.
  void run_strategy();

  void initialize_iterator(int job_index);
  void pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer);
  void pack_results_buffer(MPIPackBuffer& send_buffer, int job_index);
  void unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index);
  void update_local_results(int job_index);

private:

  //
  //- Heading: Convenience member functions
  //

  /// called by unpack_parameters_buffer(MPIUnpackBuffer) and
  /// initialize_iterator(int) to update userDefinedModel and selectedIterator
  void initialize_iterator(const RealVector& param_set);

  /// prints the concurrent iteration results summary (called by run_strategy())
  void print_results() const;

  //
  //- Heading: Data members
  //

  Model    userDefinedModel; ///< the model used by the iterator
  Iterator selectedIterator; ///< the iterator used by the concurrent strategy

  bool multiStartFlag; ///< distinguishes multi-start from Pareto-set

  /// the initial continuous variables for restoring the starting
  /// point in the Pareto set strategy
  RealVector initialPt;

  /// an array of parameter set vectors (either multistart variable
  /// sets or pareto multi-objective/least squares weighting sets) to
  /// be performed.
  RealVectorArray parameterSets;
  /// 1-d array of ParamResponsePair results corresponding to numIteratorJobs
  PRPArray prpResults;
};


inline void ConcurrentStrategy::
initialize_iterator(const RealVector& param_set)
{
  if (multiStartFlag)
    userDefinedModel.continuous_variables(param_set);
  else {
    userDefinedModel.continuous_variables(initialPt); // reset
    userDefinedModel.primary_response_fn_weights(param_set);
  }
}


inline void ConcurrentStrategy::initialize_iterator(int job_index)
{ initialize_iterator(parameterSets[job_index]); }


inline void ConcurrentStrategy::
pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index)
{ send_buffer << parameterSets[job_index]; }


inline void ConcurrentStrategy::
unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer)
{
  RealVector param_set;
  recv_buffer >> param_set;
  initialize_iterator(param_set);
}


inline void ConcurrentStrategy::
pack_results_buffer(MPIPackBuffer& send_buffer, int job_index)
{ send_buffer << prpResults[job_index]; }


inline void ConcurrentStrategy::
unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{ recv_buffer >> prpResults[job_index]; }


inline void ConcurrentStrategy::update_local_results(int job_index)
{
  ParamResponsePair prp_star(selectedIterator.variables_results(),
    userDefinedModel.interface_id(), selectedIterator.response_results(),
    job_index+1); // deep copy
  prpResults[job_index] = prp_star;
}

} // namespace Dakota

#endif
