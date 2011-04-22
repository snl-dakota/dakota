/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Non-class:   PRPMultiIndex
//- Description: Global funcs, types, etc. for lookup of PRPairs in eval cache
//- Checked by:
//- Version: $Id$

#ifndef PRP_MULTI_INDEX_H
#define PRP_MULTI_INDEX_H

#include "system_defs.h"
#include "data_types.h"
#include "ParamResponsePair.H"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <algorithm>

namespace bmi = boost::multi_index;

namespace Dakota {

// --------------------------------------------------------
// Comparison functions shared by PRPCache/PRPQueue
// --------------------------------------------------------

/// search function for a particular ParamResponsePair within a PRPList based
/// on ActiveSet content (request vector and derivative variables vector)

/** a global function to compare the ActiveSet of a particular database_pr
    (presumed to be in the global history list) with a passed in ActiveSet
    (search_set). */
inline bool 
set_compare(const ParamResponsePair& database_pr, const ActiveSet& search_set)
{
  // Check each entry of ASV for presence of all requests in the stored data.
  // A match is now detected when the search_asv is a SUBSET of the stored_asv
  // (only exact matches were interpreted as duplicates previously).  This 
  // extension is widely applicable, but was first implemented to eliminate 
  // duplication in Model::estimate_derivatives() when the gradient evaluation 
  // contains inactive fns. whereas a previous line search evaluation at the 
  // same X had no inactive fns.
  const ActiveSet&  stored_set = database_pr.active_set();
  const ShortArray& stored_asv = stored_set.request_vector();
  const ShortArray& search_asv = search_set.request_vector();
  size_t i, asv_len = search_asv.size();
  if ( stored_asv.size() != asv_len )
    return false;
  bool deriv_flag = false;
  for (i=0; i<asv_len; ++i) {
    short search_bits = search_asv[i];
    if (search_bits & 6)
      deriv_flag = true;
    // bit-wise AND used to check if each of the search bits is
    // present in the stored_asv value
    if ( (stored_asv[i] & search_bits) != search_bits )
      return false;
  }

  // check if each of the search derivative variables is present in stored_dvv
  if (deriv_flag) {
    const SizetArray& stored_dvv = stored_set.derivative_vector();
    const SizetArray& search_dvv = search_set.derivative_vector();
    size_t dvv_len = search_dvv.size();
    for (i=0; i<dvv_len; ++i)
      if ( std::find(stored_dvv.begin(), stored_dvv.end(), search_dvv[i])
           == stored_dvv.end() )
	return false;
  }

  return true;
}


// ------------------------------------------------------
// Comparison and hashing functions for PRPCache/PRPQueue
// ------------------------------------------------------

/// search function for a particular ParamResponsePair within a PRPMultiIndex

/** a global function to compare the interface id and variables of a
    particular database_pr (presumed to be in the global history list) with
    a passed in key of interface id and variables provided by search_pr. */
inline bool id_vars_exact_compare(const ParamResponsePair& database_pr,
				  const ParamResponsePair& search_pr)
{
  // First check interface id strings.  If a different interface was used, then
  // we must assume that the results are not interchangeable (differing model
  // fidelity).
  if ( search_pr.interface_id() != database_pr.interface_id() )
    return false;

  // For Boost hashing, need exact binary equality instead of the
  // tolerance-based operator== for Variables
  //if ( search_vars != stored_vars )
  if ( !binary_equal_to( search_pr.prp_parameters(),
			 database_pr.prp_parameters() ) )
    return false;

  // For Boost hashing, a post-processing step is used to manage the ActiveSet
  // logic as shown below in id_vars_set_compare

  return true;
}


/// hash_value for ParamResponsePairs stored in a PRPMultiIndex
inline std::size_t hash_value(const ParamResponsePair& prp)
{
  // hash using interface ID string.
  std::size_t seed = 0;
  boost::hash_combine(seed, prp.interface_id());

  // Now, hash values of variables using Variables hash_value friend function
  boost::hash_combine(seed, prp.prp_parameters());

  return seed;
}


// --------------------------------------
// structs and typedefs for PRPMultiIndex
// --------------------------------------
// define structs used below in PRPMultiIndex typedef

/// wrapper to delegate to the ParamResponsePair hash_value function
struct partial_prp_hash {
  /// access operator
  std::size_t operator()(const ParamResponsePair& prp) const
  { return hash_value(prp); } // ONLY idInterface & Vars used for hash_value
};

/// predicate for comparing ONLY the idInterface and Vars attributes of PRPair
struct partial_prp_equality {
  /// access operator
  bool operator()(const ParamResponsePair& database_pr,
                  const ParamResponsePair& search_pr) const
  { return id_vars_exact_compare(database_pr, search_pr); }
};


// tags
struct ordered {};
struct hashed  {};
//struct random  {};


/// Boost Multi-Index Container for caching ParamResponsePairs
typedef bmi::multi_index_container<Dakota::ParamResponsePair, bmi::indexed_by<
  // sorted beginning with lowest evalId/interfaceId value
  bmi::ordered_unique<bmi::tag<ordered>,
		      bmi::const_mem_fun<Dakota::ParamResponsePair,
		      const IntStringPair&,
		      &Dakota::ParamResponsePair::eval_interface_ids> >,
  // hashed using partial_prp_hash and compared using partial_prp_equality
  bmi::hashed_non_unique<bmi::tag<hashed>,
			 bmi::identity<Dakota::ParamResponsePair>,
                         partial_prp_hash, partial_prp_equality> > >
PRPMultiIndexCache;

typedef PRPMultiIndexCache PRPCache;
typedef PRPCache::index_iterator<ordered>::type       PRPCacheOIter;
typedef PRPCache::index_const_iterator<ordered>::type PRPCacheOCIter;
typedef PRPCache::index_iterator<hashed>::type        PRPCacheHIter;
typedef PRPCache::index_const_iterator<hashed>::type  PRPCacheHCIter;
typedef PRPCacheOIter  PRPCacheIter;  // default cache iterator <0>
typedef PRPCacheOCIter PRPCacheCIter; // default cache const iterator <0>

// begin()/end() default to index 0.  These macros support other indices.

/// hashed definition of cache begin
inline PRPCacheHIter hashedCacheBegin(PRPCache& prp_cache)
{ return prp_cache.get<hashed>().begin(); }
/// hashed definition of cache end
inline PRPCacheHIter hashedCacheEnd(PRPCache& prp_cache)
{ return prp_cache.get<hashed>().end(); }


/// Boost Multi-Index Container for queueing ParamResponsePairs
typedef bmi::multi_index_container<Dakota::ParamResponsePair, bmi::indexed_by<
  // random access for index-based operator[] access
  //bmi::random_access<bmi::tag<random> >,
  // sorted (by evalInterfaceIds) for fast key-based lookups
  bmi::ordered_unique<bmi::tag<ordered>,
		      bmi::const_mem_fun<Dakota::ParamResponsePair, int,
		      &Dakota::ParamResponsePair::eval_id> >,
  // hashed using partial_prp_hash and compared using partial_prp_equality
  bmi::hashed_non_unique<bmi::tag<hashed>,
			 bmi::identity<Dakota::ParamResponsePair>,
                         partial_prp_hash, partial_prp_equality> > >
PRPMultiIndexQueue;

typedef PRPMultiIndexQueue PRPQueue;
//typedef PRPQueue::index_iterator<random>::type        PRPQueueRIter;
//typedef PRPQueue::index_const_iterator<random>::type  PRPQueueRCIter;
typedef PRPQueue::index_iterator<ordered>::type       PRPQueueOIter;
typedef PRPQueue::index_const_iterator<ordered>::type PRPQueueOCIter;
typedef PRPQueue::index_iterator<hashed>::type        PRPQueueHIter;
typedef PRPQueue::index_const_iterator<hashed>::type  PRPQueueHCIter;
typedef PRPQueueOIter  PRPQueueIter;  // default queue iterator <0>
typedef PRPQueueOCIter PRPQueueCIter; // default queue const iterator <0>

// begin()/end() default to index 0.  These macros support other indices.

/// hashed definition of queue begin
inline PRPQueueHIter hashedQueueBegin(PRPQueue& prp_queue)
{ return prp_queue.get<hashed>().begin(); }
/// hashed definition of queue end
inline PRPQueueHIter hashedQueueEnd(PRPQueue& prp_queue)
{ return prp_queue.get<hashed>().end(); }


// ------------------------------------
// lookup_by_val for PRPMultiIndexCache
// ------------------------------------

/// find a ParamResponsePair based on the interface id, variables, and
/// ActiveSet search data within search_pr.

/** Lookup occurs in two steps: (1) PRPMultiIndexCache lookup based on
    strict equality in interface id and variables, and (2) set_compare()
    post-processing based on ActiveSet subset logic. */
inline PRPCacheHIter
lookup_by_val(PRPMultiIndexCache& prp_cache, const ParamResponsePair& search_pr)
{
  PRPCacheHIter prp_hash_it0, prp_hash_it1;
  boost::tuples::tie(prp_hash_it0, prp_hash_it1)
    = prp_cache.get<hashed>().equal_range(search_pr);

  // equal_range returns a small sequence of possibilities resulting from
  // hashing with ONLY interfaceId and variables.  Post-processing is then
  // applied to this sequence using set_compare().
  while (prp_hash_it0 != prp_hash_it1) {
    if (set_compare(*prp_hash_it0, search_pr.active_set()))
      return prp_hash_it0;
    ++prp_hash_it0;
  }
  return prp_cache.get<hashed>().end();
}


/// alternate overloaded form returns bool and sets found_pr by wrapping
/// lookup_by_val(PRPMultiIndexCache&, ParamResponsePair&)
inline bool 
lookup_by_val(PRPMultiIndexCache& prp_cache, const ParamResponsePair& search_pr,
	      ParamResponsePair& found_pr)
{
  PRPCacheHIter prp_hash_it = lookup_by_val(prp_cache, search_pr);
  if (prp_hash_it != prp_cache.get<hashed>().end()) {
    found_pr = *prp_hash_it;
    return true;
  }
  else
    return false;
}


/// find the evaluation id of a ParamResponsePair within a PRPMultiIndexCache
/// based on interface id, variables, and ActiveSet search data
inline PRPCacheHIter
lookup_by_val(PRPMultiIndexCache& prp_cache, const String& search_interface_id,
	      const Variables& search_vars,  const ActiveSet& search_set)
{
  Response search_resp(search_set);
  ParamResponsePair search_pr(search_vars, search_interface_id, search_resp);
  return lookup_by_val(prp_cache, search_pr);
}


/// find a ParamResponsePair within a PRPMultiIndexCache based on interface id,
/// variables, and ActiveSet search data
inline bool 
lookup_by_val(PRPMultiIndexCache& prp_cache, const String& search_interface_id,
	      const Variables& search_vars, const ActiveSet& search_set,
	      ParamResponsePair& found_pr)
{
  PRPCacheHIter prp_hash_it
    = lookup_by_val(prp_cache, search_interface_id, search_vars, search_set);
  if (prp_hash_it != prp_cache.get<hashed>().end()) {
    found_pr = *prp_hash_it;
    return true;
  }
  else
    return false;
}


/// find the evaluation id of a ParamResponsePair within a PRPMultiIndexCache
/// based on interface id, variables, and ActiveSet search data
inline bool
lookup_by_val(PRPMultiIndexCache& prp_cache, const String& search_interface_id,
	      const Variables& search_vars, const ActiveSet& search_set,
	      int& found_eval_id)
{
  PRPCacheHIter prp_hash_it
    = lookup_by_val(prp_cache, search_interface_id, search_vars, search_set);
  if (prp_hash_it != prp_cache.get<hashed>().end()) {
    found_eval_id = prp_hash_it->eval_id();
    return true;
  }
  else
    return false;
}


/// find the response of a ParamResponsePair within a PRPMultiIndexCache
/// based on interface id, variables, and ActiveSet search data
inline bool 
lookup_by_val(PRPMultiIndexCache& prp_cache, const String& search_interface_id,
	      const Variables& search_vars, const ActiveSet& search_set,
	      Response& found_resp)
{
  PRPCacheHIter prp_hash_it
    = lookup_by_val(prp_cache, search_interface_id, search_vars, search_set);
  if (prp_hash_it != prp_cache.get<hashed>().end()) {
    found_resp = prp_hash_it->prp_response();
    return true;
  }
  else
    return false;
}


// ------------------------------------
// lookup_by_ids for PRPMultiIndexCache
// ------------------------------------
/// find a ParamResponsePair within a PRPMultiIndexCache based on search_ids
/// (i.e. std::pair<eval_id,interface_id>) search data
inline PRPCacheOIter
lookup_by_ids(PRPMultiIndexCache& prp_cache, const IntStringPair& search_ids)
{ return prp_cache.get<ordered>().find(search_ids); }


/// find a ParamResponsePair within a PRPMultiIndexCache based on
/// eval_interface_ids
inline bool
lookup_by_ids(PRPMultiIndexCache& prp_cache,
              const IntStringPair& search_eval_interface_ids,
              ParamResponsePair& found_pr)
{
  PRPCacheOIter prp_iter = lookup_by_ids(prp_cache, search_eval_interface_ids);
  if (prp_iter != prp_cache.get<ordered>().end()) {
    found_pr = *prp_iter;
    return true;
  }
  else
    return false;
}


/// find a ParamResponsePair within a PRPMultiIndexCache based on
/// eval_interface_ids from the ParamResponsePair search data
inline bool 
lookup_by_ids(PRPMultiIndexCache& prp_cache, const ParamResponsePair& search_pr,
              ParamResponsePair& found_pr)
{ return lookup_by_ids(prp_cache, search_pr.eval_interface_ids(), found_pr); }


// ------------------------------------
// lookup_by_val for PRPMultiIndexQueue
// ------------------------------------

/// find a ParamResponsePair based on the interface id, variables, and
/// ActiveSet search data within search_pr.

/** Lookup occurs in two steps: (1) PRPMultiIndexQueue lookup based on
    strict equality in interface id and variables, and (2) set_compare()
    post-processing based on ActiveSet subset logic. */
inline PRPQueueHIter
lookup_by_val(PRPMultiIndexQueue& prp_queue, const ParamResponsePair& search_pr)
{
  PRPQueueHIter prp_hash_it0, prp_hash_it1;
  boost::tuples::tie(prp_hash_it0, prp_hash_it1)
    = prp_queue.get<hashed>().equal_range(search_pr);

  // equal_range returns a small sequence of possibilities resulting from
  // hashing with ONLY interfaceId and variables.  Post-processing is then
  // applied to this sequence using set_compare().
  while (prp_hash_it0 != prp_hash_it1) {
    if (set_compare(*prp_hash_it0, search_pr.active_set()))
      return prp_hash_it0;
    ++prp_hash_it0;
  }
  return prp_queue.get<hashed>().end();
}


/// alternate overloaded form returns bool and sets found_pr by wrapping
/// lookup_by_val(PRPMultiIndexQueue&, ParamResponsePair&)
inline bool 
lookup_by_val(PRPMultiIndexQueue& prp_queue, const ParamResponsePair& search_pr,
	      ParamResponsePair& found_pr)
{
  PRPQueueHIter prp_hash_it = lookup_by_val(prp_queue, search_pr);
  if (prp_hash_it != prp_queue.get<hashed>().end()) {
    found_pr = *prp_hash_it;
    return true;
  }
  else
    return false;
}


/// find the evaluation id of a ParamResponsePair within a PRPMultiIndexQueue
/// based on interface id, variables, and ActiveSet search data
inline PRPQueueHIter
lookup_by_val(PRPMultiIndexQueue& prp_queue, const String& search_interface_id,
	      const Variables& search_vars,  const ActiveSet& search_set)
{
  Response search_resp(search_set);
  ParamResponsePair search_pr(search_vars, search_interface_id, search_resp);
  return lookup_by_val(prp_queue, search_pr);
}


/// find a ParamResponsePair within a PRPMultiIndexQueue based on interface id,
/// variables, and ActiveSet search data
inline bool 
lookup_by_val(PRPMultiIndexQueue& prp_queue, const String& search_interface_id,
	      const Variables& search_vars, const ActiveSet& search_set,
	      ParamResponsePair& found_pr)
{
  PRPQueueHIter prp_hash_it
    = lookup_by_val(prp_queue, search_interface_id, search_vars, search_set);
  if (prp_hash_it != prp_queue.get<hashed>().end()) {
    found_pr = *prp_hash_it;
    return true;
  }
  else
    return false;
}


/// find the evaluation id of a ParamResponsePair within a PRPMultiIndexQueue
/// based on interface id, variables, and ActiveSet search data
inline bool
lookup_by_val(PRPMultiIndexQueue& prp_queue, const String& search_interface_id,
	      const Variables& search_vars, const ActiveSet& search_set,
	      int& found_eval_id)
{
  PRPQueueHIter prp_hash_it
    = lookup_by_val(prp_queue, search_interface_id, search_vars, search_set);
  if (prp_hash_it != prp_queue.get<hashed>().end()) {
    found_eval_id = prp_hash_it->eval_id();
    return true;
  }
  else
    return false;
}


/// find the response of a ParamResponsePair within a PRPMultiIndexQueue
/// based on interface id, variables, and ActiveSet search data
inline bool 
lookup_by_val(PRPMultiIndexQueue& prp_queue, const String& search_interface_id,
	      const Variables& search_vars, const ActiveSet& search_set,
	      Response& found_resp)
{
  PRPQueueHIter prp_hash_it
    = lookup_by_val(prp_queue, search_interface_id, search_vars, search_set);
  if (prp_hash_it != prp_queue.get<hashed>().end()) {
    found_resp = prp_hash_it->prp_response();
    return true;
  }
  else
    return false;
}


// ----------------------------------------
// lookup_by_eval_id for PRPMultiIndexQueue
// ----------------------------------------
/// find a ParamResponsePair within a PRPMultiIndexQueue based on search_id
/// (i.e. integer eval_id) search data
inline PRPQueueOIter
lookup_by_eval_id(PRPMultiIndexQueue& prp_queue, const int& search_id)
{ return prp_queue.get<ordered>().find(search_id); }


/// find a ParamResponsePair within a PRPMultiIndexQueue based on eval_id
inline bool
lookup_by_eval_id(PRPMultiIndexQueue& prp_queue, const int& search_id,
		  ParamResponsePair& found_pr)
{
  PRPQueueOIter prp_iter = lookup_by_eval_id(prp_queue, search_id);
  if (prp_iter != prp_queue.get<ordered>().end()) {
    found_pr = *prp_iter;
    return true;
  }
  else
    return false;
}


/// find a ParamResponsePair within a PRPMultiIndexQueue based on
/// eval_id from the ParamResponsePair search data
inline bool 
lookup_by_eval_id(PRPMultiIndexQueue& prp_queue,
		  const ParamResponsePair& search_pr,
		  ParamResponsePair& found_pr)
{ return lookup_by_eval_id(prp_queue, search_pr.eval_id(), found_pr); }

} // namespace Dakota

#endif // PRP_MULTI_INDEX_H

