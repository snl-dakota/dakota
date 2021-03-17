/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedResponseData
//- Description:  Class implementation
//- Owner:        Mike Eldred

// #define SERIALIZE_DEBUG 1

#include "SharedResponseData.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaActiveSet.hpp"
#include "dakota_data_util.hpp"
#include "dakota_data_io.hpp"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>


static const char rcsId[]="@(#) $Id: SharedResponseData.cpp 6886 2010-08-02 19:13:01Z mseldre $";

BOOST_CLASS_EXPORT(Dakota::SharedResponseDataRep)
BOOST_CLASS_EXPORT(Dakota::SharedResponseData)

namespace Dakota {


SharedResponseDataRep::
SharedResponseDataRep(const ProblemDescDB& problem_db):
  responseType(BASE_RESPONSE), // overridden in derived class ctors
  primaryFnType(GENERIC_FNS),
  responsesId(problem_db.get_string("responses.id")), 
  functionLabels(problem_db.get_sa("responses.labels")),
  simulationVariance(problem_db.get_rv("responses.simulation_variance"))
{
  // scalar-specific response data types
  // BMA TODO: review
  // don't assign member variable until sure below? NOTE: Could assign here since would be zero otherwise
  size_t num_scalar_primary = std::max
    ( problem_db.get_sizet("responses.num_scalar_objectives"),
      std::max ( problem_db.get_sizet("responses.num_scalar_calibration_terms"),
		 problem_db.get_sizet("responses.num_scalar_responses") )
      );
  size_t num_scalar_responses = num_scalar_primary +
    problem_db.get_sizet("responses.num_scalar_nonlinear_inequality_constraints") +
    problem_db.get_sizet("responses.num_scalar_nonlinear_equality_constraints");

  // field-specific response data types
  size_t num_field_primary = std::max
    ( problem_db.get_sizet("responses.num_field_objectives"),
      std::max ( problem_db.get_sizet("responses.num_field_calibration_terms"),
		 problem_db.get_sizet("responses.num_field_responses") )
      );
  size_t num_field_responses = num_field_primary +
    problem_db.get_sizet("responses.num_field_nonlinear_inequality_constraints") +
    problem_db.get_sizet("responses.num_field_nonlinear_equality_constraints");

  // aggregate response data types
  size_t num_total_primary = std::max
    ( problem_db.get_sizet("responses.num_objective_functions"),
      std::max( problem_db.get_sizet("responses.num_calibration_terms"),
		problem_db.get_sizet("responses.num_response_functions") )
      );
  size_t num_total_responses = num_total_primary +
    problem_db.get_sizet("responses.num_nonlinear_inequality_constraints") +
    problem_db.get_sizet("responses.num_nonlinear_equality_constraints");

  // update primary response type based on specification
  if (problem_db.get_sizet("responses.num_objective_functions") > 0) 
    primaryFnType = OBJECTIVE_FNS;
  else if (problem_db.get_sizet("responses.num_calibration_terms") > 0)
    primaryFnType = CALIB_TERMS;

  if (num_field_responses) {
    // BMA: Don't think the scalar spec is required; they could be all fields
    // BMA: This error message could differentiate pri/sec responses
    // require scalar spec and enforce total = scalars + fields
    if (num_field_responses + num_scalar_responses != num_total_responses) {
      Cerr << "Error: number of scalar (" << numScalarResponses
	   << ") and field (" << num_field_responses
	   << ") response functions must sum to total number ("
	   << num_total_responses << ") of response functions." << std::endl;
      abort_handler(-1);
    }
    numScalarPrimary = num_scalar_primary;
    numScalarResponses = num_scalar_responses;

    // extract the priFieldLabels from the functionLabels (one per field group)
    copy_data_partial(functionLabels, numScalarResponses, num_field_responses,
		      priFieldLabels);
    // unroll field response groups to create individual function labels
    priFieldLengths = problem_db.get_iv("responses.lengths");
    if (num_field_responses != priFieldLengths.length()) {
      Cerr << "Error: For each field response, you must specify " 
           << "the length of that field.  The number of elements " 
           << "in the 'lengths' vector must " 
           << "equal the number of field responses."  << std::endl;
      abort_handler(-1);
    } 
    build_field_labels();
  } 
  else if (numScalarResponses) {
    // no fields present...
    // validate scalar spec versus total spec
    if (numScalarResponses != num_total_responses) {
      Cerr << "Error: number of scalar (" << numScalarResponses
	   << ") and field (0) response functions must sum to total number ("
	   << num_total_responses << ") of response functions." << std::endl;
      abort_handler(-1);
    }
    numScalarPrimary = num_scalar_primary;
    numScalarResponses = num_scalar_responses;
  }
  else if (num_total_responses) {
    // only top-level keywords were specified
    // interpret total spec as scalar spec (backwards compatibility)
    numScalarPrimary = num_total_primary;
    numScalarResponses = num_total_responses;
  }
  else
    Cerr << "Warning: total number of response functions is zero.  This is "
	 << "admissible in rare cases (e.g., nested overlays)." << std::endl;
  
  if (  simulationVariance.length() != 0 && simulationVariance.length() != 1 && 
        simulationVariance.length() != num_total_responses) {
    Cerr << "Error: simulation_variance must have length equal to 1 or "
         << "the total number of calibration terms." << std::endl;
    abort_handler(-1);
  }
}


SharedResponseDataRep::SharedResponseDataRep(const ActiveSet& set):
  responseType(BASE_RESPONSE), // overridden in derived class ctors
  primaryFnType(GENERIC_FNS),
  responsesId("NO_SPECIFICATION"),
  numScalarResponses(set.request_vector().size()),
  // should we assume they are all primary? seems we have to
  numScalarPrimary(numScalarResponses)
{
  // Build a default functionLabels array (currently only used for
  // bestResponse by NPSOLOptimizer's user-defined functions option).
  functionLabels.resize(numScalarResponses);
  build_labels(functionLabels, "f");
}


/** Deep copies are used when recasting changes the nature of a
    Response set. */
void SharedResponseDataRep::copy_rep(SharedResponseDataRep* srd_rep)
{
  responseType          = srd_rep->responseType;
  primaryFnType         = srd_rep->primaryFnType;
  responsesId           = srd_rep->responsesId;

  functionLabels        = srd_rep->functionLabels;
  priFieldLabels        = srd_rep->priFieldLabels;

  numScalarResponses    = srd_rep->numScalarResponses;
  numScalarPrimary      = srd_rep->numScalarPrimary;
  priFieldLengths       = srd_rep->priFieldLengths;

  coordsPerPriField     = srd_rep->coordsPerPriField;

  simulationVariance 	= srd_rep->simulationVariance;
}


template<class Archive>
void SharedResponseDataRep::serialize(Archive& ar, const unsigned int version)
{
  ar & responseType;
  ar & primaryFnType;
  ar & responsesId;
  // TODO: archive unrolled minimal labels if possible
  ar & functionLabels;
  ar & priFieldLabels;
  ar & numScalarResponses;
  ar & numScalarPrimary;
  ar & priFieldLengths;
  ar & coordsPerPriField;
#ifdef SERIALIZE_DEBUG  
  Cout << "Serializing SharedResponseDataRep:\n"
       << responseType << '\n'
       << responsesId << '\n'
       << functionLabels
       << numScalarResponses
       << priFieldLengths
       << std::endl;
#endif
}


bool SharedResponseDataRep::operator==(const SharedResponseDataRep& other)
{
  return (responseType == other.responseType &&
	  primaryFnType == other.primaryFnType &&
	  responsesId == other.responsesId &&
	  functionLabels == other.functionLabels &&
	  priFieldLabels == other.priFieldLabels &&
	  numScalarResponses == other.numScalarResponses &&
	  numScalarPrimary == other.numScalarPrimary &&
	  priFieldLengths == other.priFieldLengths &&
	  coordsPerPriField == other.coordsPerPriField);
}


void SharedResponseDataRep::build_field_labels()
{
  size_t unroll_fns = numScalarResponses + priFieldLengths.normOne();
  if (functionLabels.size() != unroll_fns)
    functionLabels.resize(unroll_fns);  // unique label for each QoI

  // BMA TODO: Fix as nonlinear constraints need to follow primary fields

  // append _<field_entry_num> to the base label
  size_t unrolled_index = numScalarResponses;
  for (size_t i=0; i<priFieldLengths.length(); ++i)
    for (size_t j=0; j<priFieldLengths[i]; ++j)
      build_label(functionLabels[unrolled_index++], priFieldLabels[i], j+1, "_");
}

/** Deep copies are used when recasting changes the nature of a
    Response set. */
SharedResponseData SharedResponseData::copy() const
{
  // the handle class instantiates a new handle and a new body and copies
  // current attributes into the new body
  SharedResponseData srd; // new handle: srdRep=NULL
  if (srdRep) {
    srd.srdRep.reset(new SharedResponseDataRep());
    srd.srdRep->copy_rep(srdRep.get());
  }

  return srd;
}


void SharedResponseData::reshape(size_t num_fns)
{
  if (num_functions() != num_fns) {
    // separate sharing if needed
    //if (srdRep.use_count() > 1) { // shared rep: separate
    boost::shared_ptr<SharedResponseDataRep> old_rep = srdRep;
    srdRep.reset(new SharedResponseDataRep()); // create new srdRep
    srdRep->copy_rep(old_rep.get());           // copy old data to new
    //}

    // reshape function labels
    reshape_labels(srdRep->functionLabels, num_fns);
    // BMA TODO: may need to cache more info to do this, or may not be possible
    // update scalar counts (update of field counts requires addtnl data)
    srdRep->numScalarResponses = num_fns - num_field_functions();
  }
}


void SharedResponseData::
reshape_labels(StringArray& resp_labels, size_t num_new)
{
  // Replicate or prune response labels to meet new size.
  // Could consider adding / deleting additional level annotation,
  // e.g. "response_fn_i" --> "response_fn_i_lev_j".
  size_t num_curr = resp_labels.size();
  bool overwrite = false;
  if (num_new > num_curr) {
    if (num_new % num_curr) // not a growth factor
      overwrite = true;
    else { // inflate using set replication (no added annotation)
      resp_labels.resize(num_new);
      for (size_t i=num_curr; i<num_new; ++i)
	resp_labels[i] = resp_labels[i % num_curr];
    }
  }
  else if (num_curr > num_new) {
    if (num_curr % num_new)
      overwrite = true;
    else // deflate by concatenation (no annotation to remove)
      resp_labels.resize(num_new);
  }

  if (overwrite) { // last resort
    resp_labels.resize(num_new);
    build_labels(resp_labels, "f");
  }
}


// TODO: unify with reshape
void SharedResponseData::field_lengths(const IntVector& field_lens) 
{ 
  // no change in number of scalar functions
  // when the field lengths change, need a new rep
  if (field_lengths() != field_lens) {
    boost::shared_ptr<SharedResponseDataRep> old_rep = srdRep;
    srdRep.reset(new SharedResponseDataRep());  // create new srdRep
    srdRep->copy_rep(old_rep.get());            // copy old data to new
    
    // update the field lengths
    srdRep->priFieldLengths = field_lens;

    // reshape function labels, using updated num_functions()
    srdRep->functionLabels.resize(num_functions());
    if (field_lens.length() != srdRep->priFieldLabels.size()) {
      // can't use existing field labels (could happen in testing); use generic
      build_labels(srdRep->functionLabels, "f");
      // update the priFieldLabels
      copy_data_partial(srdRep->functionLabels, num_scalar_responses(),
			num_field_response_groups(), srdRep->priFieldLabels);
    }
    else {
      // no change in number of fields; use existing labels for build
      srdRep->build_field_labels();
    }
  }
}


void SharedResponseData::field_group_labels(const StringArray& field_labels)
{
  if (field_labels.size() != num_field_response_groups()) {
    Cerr << "\nError: Attempt to set " << field_labels.size() 
	 << " labels on Response with " << num_field_response_groups()
	 << " fields." << std::endl;
    abort_handler(-1);
  }
  srdRep->priFieldLabels = field_labels;
  // rebuild unrolled functionLabels for field values (no size change)
  srdRep->build_field_labels();
}


void SharedResponseData::primary_fn_type(short type) 
{ 
  // when the primary type changes, need a new rep
  if (srdRep->primaryFnType != type) {
    boost::shared_ptr<SharedResponseDataRep> old_rep = srdRep;
    srdRep.reset(new SharedResponseDataRep());  // create new srdRep
    srdRep->copy_rep(old_rep.get());            // copy old data to new
    srdRep->primaryFnType = type;
  }
}


bool SharedResponseData::operator==(const SharedResponseData& other)
{
  // test pointer equality
  //  return(srdRep->get() == other.srdRep->get());
  // test data equality
  return(*srdRep == *other.srdRep);
}


template<class Archive>
void SharedResponseData::serialize(Archive& ar, const unsigned int version)
{
  // load will default construct and load through the pointer
  ar & srdRep;
}


long SharedResponseData::reference_count() const
{  return srdRep.use_count();  }


// explicit instantions needed due to serialization through pointer,
// which won't instantate the above template?
template void SharedResponseData::serialize<boost::archive::binary_iarchive>
(boost::archive::binary_iarchive& ar, const unsigned int version);

template void SharedResponseData::serialize<boost::archive::binary_oarchive>
(boost::archive::binary_oarchive& ar, const unsigned int version);



} // namespace Dakota
