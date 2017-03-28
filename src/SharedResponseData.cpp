/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedResponseData
//- Description:  Class implementation
//- Owner:        Mike Eldred

// #define REFCOUNT_DEBUG 1
// #define SERIALIZE_DEBUG 1

#include "SharedResponseData.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaActiveSet.hpp"
#include "dakota_data_util.hpp"
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
  functionLabels(problem_db.get_sa("responses.labels"))
{
  // scalar response data types:
  size_t num_scalar_resp_fns
    = problem_db.get_sizet("responses.num_scalar_responses");
  numScalarResponses = (num_scalar_resp_fns) ? num_scalar_resp_fns :
    problem_db.get_sizet(
      "responses.num_scalar_nonlinear_inequality_constraints") +
    problem_db.get_sizet(
      "responses.num_scalar_nonlinear_equality_constraints")   +
    std::max(problem_db.get_sizet("responses.num_scalar_objectives"),
	     problem_db.get_sizet("responses.num_scalar_calibration_terms"));

  // field response data types:
  size_t num_field_resp_fns
    = problem_db.get_sizet("responses.num_field_responses"),
  num_field_responses = (num_field_resp_fns) ? num_field_resp_fns :
    problem_db.get_sizet(
      "responses.num_field_nonlinear_inequality_constraints") +
    problem_db.get_sizet(
      "responses.num_field_nonlinear_equality_constraints")   +
    std::max(problem_db.get_sizet("responses.num_field_objectives"),
	     problem_db.get_sizet("responses.num_field_calibration_terms"));

  // aggregate response data types:
  size_t num_total_resp_fns
    = problem_db.get_sizet("responses.num_response_functions"),
  num_total_responses = (num_total_resp_fns) ? num_total_resp_fns :
    problem_db.get_sizet("responses.num_nonlinear_inequality_constraints") +
    problem_db.get_sizet("responses.num_nonlinear_equality_constraints")   +
    std::max(problem_db.get_sizet("responses.num_objective_functions"),
	     problem_db.get_sizet("responses.num_calibration_terms"));

  // update primary response type based on specification
  if (problem_db.get_sizet("responses.num_objective_functions") > 0) 
    primaryFnType = OBJECTIVE_FNS;
  else if (problem_db.get_sizet("responses.num_calibration_terms") > 0)
    primaryFnType = CALIB_TERMS;

  if (num_field_responses) {
    // require scalar spec and enforce total = scalar + field
    if (num_field_responses + numScalarResponses != num_total_responses) {
      Cerr << "Error: number of scalar (" << numScalarResponses
	   << ") and field (" << num_field_responses
	   << ") response functions must sum to total number ("
	   << num_total_responses << ") of response functions." << std::endl;
      abort_handler(-1);
    }

    // extract the fieldLabels from the functionLabels (one per field group)
    copy_data_partial(functionLabels, numScalarResponses, num_field_responses,
		      fieldLabels);
    // unroll field response groups to create individual function labels
    fieldRespGroupLengths = problem_db.get_iv("responses.lengths");
    if (num_field_responses != fieldRespGroupLengths.length()) {
      Cerr << "Error: For each field response, you must specify " 
           << "the length of that field.  The number of elements " 
           << "in the 'lengths' vector must " 
           << "equal the number of field responses."  << std::endl;
      abort_handler(-1);
    } 
    build_field_labels();
  } 
  else if (numScalarResponses) {
    // validate scalar spec versus total spec
    if (numScalarResponses != num_total_responses) {
      Cerr << "Error: number of scalar (" << numScalarResponses
	   << ") and field (0) response functions must sum to total number ("
	   << num_total_responses << ") of response functions." << std::endl;
      abort_handler(-1);
    }
  }
  else if (num_total_responses) {
    // interpret total spec as scalar spec (backwards compatibility)
    numScalarResponses = num_total_responses;
  }
  else
    Cerr << "Warning: total number of response functions is zero.  This is "
	 << "admissible in rare cases (e.g., nested overlays)." << std::endl;

#ifdef REFCOUNT_DEBUG
  Cout << "SharedResponseDataRep::SharedResponseDataRep(problem_db) "
       << "called to build body object." << std::endl;
#endif
}


SharedResponseDataRep::SharedResponseDataRep(const ActiveSet& set):
  responseType(BASE_RESPONSE), // overridden in derived class ctors
  primaryFnType(GENERIC_FNS),
  responsesId("NO_SPECIFICATION"),
  numScalarResponses(set.request_vector().size())
{
  // Build a default functionLabels array (currently only used for
  // bestResponse by NPSOLOptimizer's user-defined functions option).
  functionLabels.resize(numScalarResponses);
  build_labels(functionLabels, "f");

#ifdef REFCOUNT_DEBUG
  Cout << "SharedResponseDataRep::SharedResponseDataRep() called to build "
       << "empty body object." << std::endl;
#endif
}


/** Deep copies are used when recasting changes the nature of a
    Response set. */
void SharedResponseDataRep::copy_rep(SharedResponseDataRep* srd_rep)
{
  responseType          = srd_rep->responseType;
  primaryFnType         = srd_rep->primaryFnType;
  responsesId           = srd_rep->responsesId;

  functionLabels        = srd_rep->functionLabels;
  fieldLabels           = srd_rep->fieldLabels;

  numScalarResponses    = srd_rep->numScalarResponses;
  fieldRespGroupLengths = srd_rep->fieldRespGroupLengths;

  numCoordsPerField     = srd_rep->numCoordsPerField;
}


template<class Archive>
void SharedResponseDataRep::serialize(Archive& ar, const unsigned int version)
{
  ar & responseType;
  ar & primaryFnType;
  ar & responsesId;
  // TODO: archive unrolled minimal labels if possible
  ar & functionLabels;
  ar & fieldLabels;
  ar & numScalarResponses;
  ar & fieldRespGroupLengths;
  ar & numCoordsPerField;
#ifdef SERIALIZE_DEBUG  
  Cout << "Serializing SharedResponseDataRep:\n"
       << responseType << '\n'
       << responsesId << '\n'
       << functionLabels
       << numScalarResponses
       << fieldRespGroupLengths
       << std::endl;
#endif
}


bool SharedResponseDataRep::operator==(const SharedResponseDataRep& other)
{
  return (responseType == other.responseType &&
	  primaryFnType == other.primaryFnType &&
	  responsesId == other.responsesId &&
	  functionLabels == other.functionLabels &&
	  fieldLabels == other.fieldLabels &&
	  numScalarResponses == other.numScalarResponses &&
	  fieldRespGroupLengths == other.fieldRespGroupLengths &&
	  numCoordsPerField == other.numCoordsPerField);
}


void SharedResponseDataRep::build_field_labels()
{
  size_t unroll_fns = numScalarResponses + fieldRespGroupLengths.normOne();
  if (functionLabels.size() != unroll_fns)
    functionLabels.resize(unroll_fns);  // unique label for each QoI

  // append _<field_entry_num> to the base label
  size_t unrolled_index = numScalarResponses;
  for (size_t i=0; i<fieldRespGroupLengths.length(); ++i)
    for (size_t j=0; j<fieldRespGroupLengths[i]; ++j)
      build_label(functionLabels[unrolled_index++], fieldLabels[i], j+1, "_");
}


/** Deep copies are used when recasting changes the nature of a
    Response set. */
SharedResponseData SharedResponseData::copy() const
{
  // the handle class instantiates a new handle and a new body and copies
  // current attributes into the new body

#ifdef REFCOUNT_DEBUG
  Cout << "SharedResponseData::copy() called to generate a deep copy with no "
       << "representation sharing.\n";
  Cout << "  srdRep use_count before = " << srdRep.use_count() << std::endl;
#endif

  SharedResponseData srd; // new handle: srdRep=NULL
  if (srdRep) {
    srd.srdRep.reset(new SharedResponseDataRep());
    srd.srdRep->copy_rep(srdRep.get());
  }

#ifdef REFCOUNT_DEBUG
  Cout << "  srdRep use_count after  = " << srdRep.use_count() << '\n';
  Cout << "  new srd use_count after  = " << srd.srdRep.use_count() << std::endl;
#endif
 
  return srd;
}


void SharedResponseData::reshape(size_t num_fns)
{
  if (num_functions() != num_fns) {
    // separate sharing if needed
    //if (srdRep->referenceCount > 1) { // shared rep: separate
#ifdef REFCOUNT_DEBUG
    Cout << "SharedResponseData::reshape() called.\n"
	 << "  srdRep use_count before = " << srdRep.use_count() << std::endl;
#endif
    boost::shared_ptr<SharedResponseDataRep> old_rep = srdRep;
    srdRep.reset(new SharedResponseDataRep()); // create new srdRep
    srdRep->copy_rep(old_rep.get());           // copy old data to new
#ifdef REFCOUNT_DEBUG
    Cout << "  srdRep use_count after  = " << srdRep.use_count() << '\n'
	 << "  old_rep use_count after = " << old_rep.use_count() << std::endl;
#endif
    //}

    // reshape function labels
    srdRep->functionLabels.resize(num_fns);
    build_labels(srdRep->functionLabels, "f");
    // update scalar counts (update of field counts requires addtnl data)
    srdRep->numScalarResponses = num_fns - num_field_functions();
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
    srdRep->fieldRespGroupLengths = field_lens;

    // reshape function labels, using updated num_functions()
    srdRep->functionLabels.resize(num_functions());
    if (field_lens.length() != srdRep->fieldLabels.size()) {
      // can't use existing field labels (could happen in testing); use generic
      build_labels(srdRep->functionLabels, "f");
      // update the fieldLabels
      copy_data_partial(srdRep->functionLabels, num_scalar_responses(),
			num_field_response_groups(), srdRep->fieldLabels);
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
  srdRep->fieldLabels = field_labels;
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
#ifdef REFCOUNT_DEBUG
  Cout << "SRD serializing with pointer " << srdRep.get() << '\n'
       << "  srdRep use_count before = " << srdRep.use_count() << std::endl;
#endif
  // load will default construct and load through the pointer
  ar & srdRep;
#ifdef REFCOUNT_DEBUG
  Cout << "  srdRep pointer after  = " << srdRep.get() << std::endl;
  Cout << "  srdRep use_count after  = " << srdRep.use_count() << std::endl;
#endif
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
