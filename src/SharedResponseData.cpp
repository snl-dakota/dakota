/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
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
  simulationVariance(problem_db.get_rv("responses.simulation_variance")),
  metadataLabels(problem_db.get_sa("responses.metadata_labels"))
{
  // scalar-specific response counts
  size_t num_scalar_primary = std::max
    ( problem_db.get_sizet("responses.num_scalar_objectives"),
      std::max ( problem_db.get_sizet("responses.num_scalar_calibration_terms"),
		 problem_db.get_sizet("responses.num_scalar_responses") )
      );
  size_t num_scalar_responses = num_scalar_primary +
    // secondary counts will always be zero as not exposed in input spec
    problem_db.get_sizet("responses.num_scalar_nonlinear_inequality_constraints") +
    problem_db.get_sizet("responses.num_scalar_nonlinear_equality_constraints");

  // field-specific response counts
  size_t num_field_primary = std::max
    ( problem_db.get_sizet("responses.num_field_objectives"),
      std::max ( problem_db.get_sizet("responses.num_field_calibration_terms"),
		 problem_db.get_sizet("responses.num_field_responses") )
      );
  size_t num_field_responses = num_field_primary +
    // secondary counts will always be zero as not exposed in input spec
    problem_db.get_sizet("responses.num_field_nonlinear_inequality_constraints") +
    problem_db.get_sizet("responses.num_field_nonlinear_equality_constraints");

  // parent (aggregate/total) response counts
  size_t num_total_primary = std::max
    ( problem_db.get_sizet("responses.num_objective_functions"),
      std::max( problem_db.get_sizet("responses.num_calibration_terms"),
		problem_db.get_sizet("responses.num_response_functions") )
      );
  size_t num_total_secondary =
    problem_db.get_sizet("responses.num_nonlinear_inequality_constraints") +
    problem_db.get_sizet("responses.num_nonlinear_equality_constraints");
  size_t num_total_responses = num_total_primary + num_total_secondary;

  // update primary response type based on user specified type
  if (problem_db.get_sizet("responses.num_objective_functions") > 0) 
    primaryFnType = OBJECTIVE_FNS;
  else if (problem_db.get_sizet("responses.num_calibration_terms") > 0)
    primaryFnType = CALIB_TERMS;

  const StringArray& user_labels = problem_db.get_sa("responses.labels");

  if (num_field_responses) {
    // validate required apportionment of total = scalar + field
    if (num_scalar_primary + num_field_primary != num_total_primary) {
      Cerr << "Error: number of scalar (" << num_scalar_primary
	   << ") and field (" << num_field_primary
	   << ") " << primary_fn_name() << " must sum to total number ("
	   << num_total_primary << ") of " << primary_fn_name() <<"." << std::endl;
      abort_handler(-1);
    }
    // TODO: If/when field constraints are allowed, validate apportionment
    numScalarPrimary = num_scalar_primary;
    // can't use num_scalar_responses, as constraints are only specified via total
    numScalarResponses = num_scalar_primary + num_total_secondary;

    priFieldLengths = problem_db.get_iv("responses.lengths");
    if (num_field_primary != priFieldLengths.length()) {
      Cerr << "Error: For each field in " << primary_fn_name()
	   << ", you must specify the length of that field."
	   << "\n  The number of elements in the 'lengths' vector must "
           << "equal the number (" << num_field_primary << ") of field "
	   << primary_fn_name() << "."  << std::endl;
      abort_handler(-1);
    } 
    build_field_labels(user_labels);
  } 
  else if (num_scalar_responses) {
    // no fields present, but scalar apportionment given; must agree with total
    if (num_scalar_primary != num_total_primary) {
      Cerr << "Error: number of scalar (" << num_scalar_primary
	   << ") and field (0) " << primary_fn_name()
	   << " must sum to total number ("
	   << num_total_primary << ") of " << primary_fn_name() << "."
	   << std::endl;
      abort_handler(-1);
    }
    // TODO: If/when field constraints are allowed, validate apportionment
    numScalarPrimary = num_scalar_primary;
    // can't use num_scalar_responses, as constraints are only specified via total
    numScalarResponses = num_scalar_primary + num_total_secondary;
    functionLabels = user_labels;
  }
  else if (num_total_responses) {
    // only top-level keywords were specified
    // interpret total spec as scalar spec (backwards compatibility)
    numScalarPrimary = num_total_primary;
    numScalarResponses = num_total_responses;
    functionLabels = user_labels;
  }
  else
    Cerr << "Warning: total number of response functions is zero.  This is "
	 << "admissible in rare cases (e.g., nested overlays)." << std::endl;
  
  // BMA: In reviewing, this may be wrong for cases with fields or constraints
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

  simulationVariance 	= srd_rep->simulationVariance;

  numScalarResponses    = srd_rep->numScalarResponses;
  numScalarPrimary      = srd_rep->numScalarPrimary;
  priFieldLengths       = srd_rep->priFieldLengths;

  coordsPerPriField     = srd_rep->coordsPerPriField;

  metadataLabels        = srd_rep->metadataLabels;
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
  if (version >= 1)
    ar & metadataLabels;
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
	  coordsPerPriField == other.coordsPerPriField &&
	  metadataLabels == other.metadataLabels);
}


void SharedResponseDataRep::
build_field_labels(const StringArray& labels_per_group)
{
  size_t num_primary_fields = priFieldLengths.length();
  size_t num_field_functions = priFieldLengths.normOne();
  // extract the priFieldLabels from the functionLabels (one per field group)
  copy_data_partial(labels_per_group, numScalarPrimary, num_primary_fields,
		    priFieldLabels);

  size_t unroll_fns = numScalarResponses + num_field_functions;
  if (functionLabels.size() != unroll_fns)
    functionLabels.resize(unroll_fns);  // unique label for each QoI

  size_t unrolled_index = 0;
  for (size_t i=0; i < numScalarPrimary; ++i, ++unrolled_index)
    functionLabels[unrolled_index] = labels_per_group[i];

  for (size_t i=0; i<priFieldLengths.length(); ++i)
    for (size_t j=0; j<priFieldLengths[i]; ++j)
      build_label(functionLabels[unrolled_index++], priFieldLabels[i], j+1, "_");

  size_t num_scalar_secondary = numScalarResponses - numScalarPrimary;
  for (size_t i=0; i<num_scalar_secondary; ++i, ++unrolled_index)
    functionLabels[unrolled_index] =
      labels_per_group[numScalarPrimary + priFieldLengths.length() + i];
}

void SharedResponseDataRep::
resize_field_labels(const StringArray& old_full_labels, size_t old_field_elements)
{
  size_t unroll_fns = numScalarResponses + priFieldLengths.normOne();
  if (functionLabels.size() != unroll_fns)
    functionLabels.resize(unroll_fns);  // unique label for each QoI

  size_t unrolled_index = 0;
  for (size_t i=0; i < numScalarPrimary; ++i, ++unrolled_index)
    functionLabels[unrolled_index] = old_full_labels[i];

  for (size_t i=0; i<priFieldLengths.length(); ++i)
    for (size_t j=0; j<priFieldLengths[i]; ++j)
      build_label(functionLabels[unrolled_index++], priFieldLabels[i], j+1, "_");

  size_t num_scalar_secondary = numScalarResponses - numScalarPrimary;
  for (size_t i=0; i<num_scalar_secondary; ++i, ++unrolled_index)
    functionLabels[unrolled_index] =
      old_full_labels[numScalarPrimary + old_field_elements + i];
}

void SharedResponseDataRep::update_field_labels()
{
  size_t unrolled_index = numScalarPrimary;
  for (size_t i=0; i<priFieldLengths.length(); ++i)
    for (size_t j=0; j<priFieldLengths[i]; ++j)
      build_label(functionLabels[unrolled_index++], priFieldLabels[i], j+1, "_");
}


std::string SharedResponseDataRep::primary_fn_name() const
{
  switch(primaryFnType) {
  case OBJECTIVE_FNS:
    return "objective_functions";
  case CALIB_TERMS:
    return "calibration_terms";
  case GENERIC_FNS:
    return "response_functions";
  default:
    Cerr << "Error: unknown primary function type " << primaryFnType
	 << " in SharedResponseData." << std::endl;
      abort_handler(-1);
  }
  return "(unknown function type)";
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
    if (srdRep.use_count() > 1) { // separate srdRep from shared before reshape
      std::shared_ptr<SharedResponseDataRep> old_rep = srdRep;
      srdRep.reset(new SharedResponseDataRep()); // create new srdRep
      srdRep->copy_rep(old_rep.get());           // copy old data to new
    }

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
    if (num_curr == 0 || num_new % num_curr) // not a growth factor
      overwrite = true;
    else { // inflate using set replication (no added annotation)
      resp_labels.resize(num_new);
      for (size_t i=num_curr; i<num_new; ++i)
	resp_labels[i] = resp_labels[i % num_curr];
    }
  }
  else if (num_curr > num_new) {
    if (num_new == 0 || num_curr % num_new)
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
    // separate sharing if needed
    if (srdRep.use_count() > 1) { // separate rep from shared
      std::shared_ptr<SharedResponseDataRep> old_rep = srdRep;
      srdRep.reset(new SharedResponseDataRep());  // create new srdRep
      srdRep->copy_rep(old_rep.get());            // copy old data to new
    }

    // cache previous data for use at bottom
    bool same_field_groups
      = (field_lens.length() == srdRep->priFieldLabels.size());
    StringArray old_fn_labels;  size_t old_field_elements;
    if (same_field_groups) {
      old_fn_labels      = srdRep->functionLabels;
      old_field_elements = srdRep->priFieldLengths.normOne();
    }
 
    // update the field lengths
    srdRep->priFieldLengths = field_lens;
    // reshape function labels, using updated num_functions()
    srdRep->functionLabels.resize(num_functions());

    if (same_field_groups)
      // no change in number of field groups; use existing labels for build
      // need to preserve scalar labels, but update field labels
      srdRep->resize_field_labels(old_fn_labels, old_field_elements);
    else {
      // can't use existing field labels (could happen in testing); use generic
      build_labels(srdRep->functionLabels, "f");
      // update the priFieldLabels
      copy_data_partial(srdRep->functionLabels, num_scalar_responses(),
			num_field_response_groups(), srdRep->priFieldLabels);
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
  srdRep->update_field_labels();
}


void SharedResponseData::primary_fn_type(short type) 
{ 
  // when the primary type changes, need a new rep
  if (srdRep->primaryFnType != type) {
    std::shared_ptr<SharedResponseDataRep> old_rep = srdRep;
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
