/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef SHARED_RESPONSE_DATA_H
#define SHARED_RESPONSE_DATA_H

#include "dakota_data_io.hpp"
#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"
#include "DataResponses.hpp"
#include <boost/serialization/access.hpp>
#include <boost/serialization/tracking.hpp>

namespace Dakota {

// forward declarations
class ProblemDescDB;


/// The representation of a SharedResponseData instance.  This representation,
/// or body, may be shared by multiple SharedResponseData handle instances.

/** The SharedResponseData/SharedResponseDataRep pairs utilize a
    handle-body idiom (Coplien, Advanced C++). */

class SharedResponseDataRep
{
  //
  //- Heading: Friends
  //

  friend class SharedResponseData;

  /// allow boost access to serialize this class
  friend class boost::serialization::access;

public:

  /// destructor must be public for shared_ptr
  ~SharedResponseDataRep();

private:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  SharedResponseDataRep();
  /// standard constructor
  SharedResponseDataRep(const ProblemDescDB& problem_db);
  /// alternate on-the-fly constructor
  SharedResponseDataRep(const ActiveSet& set);

  //
  //- Heading: Member functions
  //

  /// copy the data from srd_rep to the current representation
  void copy_rep(SharedResponseDataRep* srd_rep);

  /// serialize the core shared response data: write and read are
  /// symmetric for this class
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version);

  /// experimental operator== for use in unit testing
  bool operator==(const SharedResponseDataRep& other);

  /// populate functionLabels with scalar and unrolled field labels
  /// based on fieldLabels and group lengths
  void build_field_labels(const StringArray& labels_per_group);

  void resize_field_labels(const StringArray& old_full_labels,
			   size_t old_field_elements);

  /// update functionLabels with unrolled field labels based on fieldLabels and
  /// group lengths
  void update_field_labels();

  /// the primary function type as a friendly string, e.g., "objective_functions"
  std::string primary_fn_name() const;

  //
  //- Heading: Data
  //

  /// enumeration of BASE_RESPONSE, SIMULATION_RESPONSE, or EXPERIMENT_RESPONSE
  short responseType;

  /// data set type for primary response: generic, objective, calibration
  short primaryFnType;

  /// response identifier string from the input file
  String responsesId;

  /// fine-grained (unrolled) set of response function identifiers
  /// used to improve output readability; length Response::functionValues
  StringArray functionLabels;
  
  /// labels for each primary response field
  StringArray priFieldLabels;
  
  /// simulation variance
  RealVector simulationVariance;
  
  /// number of scalar responses (scalar primary + scalar constraints)
  size_t numScalarResponses = 0;

  /// number of scalar primary reponses (secondary computed from difference)
  size_t numScalarPrimary = 0;

  /// length of each primary response field
  IntVector priFieldLengths;

  /// number of independent coordinates, e.g., x, t, for each field f(x,t)
  IntVector coordsPerPriField;

  /// descriptors for metadata fields (empty if none)
  StringArray metadataLabels;
};


inline SharedResponseDataRep::SharedResponseDataRep():
  // copy and reshape immediately apply copy_rep, so don't initialize anything:
  //:responseType(BASE_RESPONSE)//, // overridden in derived class ctors
  primaryFnType(GENERIC_FNS)
  //responsesId("NO_SPECIFICATION"), numScalarResponses(0),
{ /* empty ctor */}


inline SharedResponseDataRep::~SharedResponseDataRep()
{ /* empty dtor */ }


/// Container class encapsulating variables data that can be shared
/// among a set of Response instances.

/** An array of Response objects (e.g., Analyzer::allResponse) contains
    repeated configuration data (id's, labels, counts).  SharedResponseData 
    employs a handle-body idiom to allow this shared data to be managed 
    in a single object with many references to it, one per Response 
    object in the array.  This allows scaling to larger sample sets. */

class SharedResponseData
{
public:

  //
  //- Heading: Friends
  //

  /// allow boost access to serialize this class
  friend class boost::serialization::access;


  //
  //- Heading: Constructors, destructor, and operators
  //

  /// default constructor
  SharedResponseData();
  /// standard constructor
  explicit SharedResponseData(const ProblemDescDB& problem_db);
  /// alternate on-the-fly constructor (explicit disallows its use for implicit
  /// type conversion so that instantiations of Response(set) are invalid)
  explicit SharedResponseData(const ActiveSet& set);
  /// copy constructor
  SharedResponseData(const SharedResponseData& srd);
  /// destructor
  ~SharedResponseData();

  /// assignment operator
  SharedResponseData& operator=(const SharedResponseData& srd);
 
  /// experimental operator== for use in unit testing
  bool operator==(const SharedResponseData& other);


  //
  //- Heading: member functions
  //

  /// number of scalar responses: primary_scalar + nonlinear
  /// constraints; note that these are non-contiguous when primary
  /// fields are present.
  size_t num_scalar_responses() const;

  /// number of scalar primary responses
  /// (objectives, calibration terms, generic)
  size_t num_scalar_primary() const;

  /// number of primary fields (primary field groups)
  size_t num_field_response_groups() const;
  /// total number of response groups (number scalars + number pri field groups)
  size_t num_response_groups() const;
  /// total number of primary field functions (elements);
  /// 1-norm of priFieldLengths
  size_t num_field_functions() const;

  /// number of primary functions (pri scalars + 1-norm of priFieldLengths)
  size_t num_primary_functions() const;

  /// total number of response functions (pri scalars + 1-norm of
  /// priFieldLengths + secondary scalars)
  size_t num_functions() const;

  /// length of each primary field
  const IntVector& field_lengths() const;
  /// set field lengths (e.g., if experiment different from simulation)
  void field_lengths(const IntVector& field_lens);
  /// number of independent coordinates for each primary field
  const IntVector& num_coords_per_field() const;

  /// return a response function identifier string
  const String& function_label(size_t i) const;
  /// return the response function identifier strings
  const StringArray& function_labels() const;
  /// return the response function identifier strings
  StringArray& function_labels();
  /// set a response function identifier string
  void function_label(const String& label, size_t i);
  /// set the response function identifier strings
  void function_labels(const StringArray& labels);

  /// return the coarse (per-group) primary field response labels
  const StringArray& field_group_labels() const;
  /// set the coarse primary field group labels (must agree with number fields)
  void field_group_labels(const StringArray& field_labels);

  /// return the response identifier
  const String& responses_id() const;

  /// return the response type: {BASE,SIMULATION,EXPERIMENT}_RESPONSE
  short response_type() const;
  /// set the response type: {BASE,SIMULATION,EXPERIMENT}_RESPONSE
  void response_type(short type);

  /// get the primary function type (generic, objective, calibration)
  short primary_fn_type() const;
  /// set the primary function type (generic, objective, calibration)
  void primary_fn_type(short type);

  /// retrieve simulation variance
  const RealVector& simulation_error() const;

  /// get labels for metadata fields
  const StringArray& metadata_labels() const;
  /// set labels for metadata fields
  void metadata_labels(const StringArray& md_labels);

  /// read metadata labels from annotated (neutral) file
  void read_annotated(std::istream& s, size_t num_md);

  /// create a deep copy of the current object and return by value
  SharedResponseData copy() const;
  /// reshape the data, disconnecting a shared rep if necessary
  void reshape(size_t num_fns);
  /// reshape the shared metadata (labels only at this time)
  void reshape_metadata(size_t num_meta);
  /// reshape the response labels using inflation/deflation if possible
  void reshape_labels(StringArray& resp_labels, size_t num_fns);

  /// return true if empty handle with null representation
  bool is_null() const;

  /// how many handles (including this) are sharing this
  /// representation (body); for debugging/testing only
  long reference_count() const;

private:

  /// serialize through the pointer, which requires object tracking:
  /// write and read are symmetric for this class
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version);

  //
  //- Heading: Private data members
  //

  /// pointer to the body (handle-body idiom)
  std::shared_ptr<SharedResponseDataRep> srdRep;
};


inline SharedResponseData::SharedResponseData()
{ /* empty ctor */ }


inline SharedResponseData::SharedResponseData(const ProblemDescDB& problem_db):
  srdRep(new SharedResponseDataRep(problem_db))
{ /* empty ctor */ }


inline SharedResponseData::SharedResponseData(const ActiveSet& set):
  srdRep(new SharedResponseDataRep(set))
{ /* empty ctor */ }


inline SharedResponseData::SharedResponseData(const SharedResponseData& srd)
{
  // share the representation (body)
  srdRep = srd.srdRep;
}


inline SharedResponseData& SharedResponseData::
operator=(const SharedResponseData& srd)
{
  // share the inbound representation (body) by copying the pointer
  srdRep = srd.srdRep;
  return *this;
}


inline SharedResponseData::~SharedResponseData()
{ /* empty dtor in case we add virtual functions */ }


inline size_t SharedResponseData::num_scalar_responses() const
{ return srdRep->numScalarResponses; }


inline size_t SharedResponseData::num_scalar_primary() const
{ return srdRep->numScalarPrimary; }


inline size_t SharedResponseData::num_field_response_groups() const
{ return srdRep->priFieldLengths.length(); }


inline size_t SharedResponseData::num_response_groups() const
{ return srdRep->numScalarResponses + srdRep->priFieldLengths.length(); }


inline size_t SharedResponseData::num_field_functions() const
{ return srdRep->priFieldLengths.normOne(); }


inline size_t SharedResponseData::num_primary_functions() const
{ return srdRep->numScalarPrimary + srdRep->priFieldLengths.normOne(); }


inline size_t SharedResponseData::num_functions() const
{ return srdRep->numScalarResponses + srdRep->priFieldLengths.normOne(); }


inline const IntVector& SharedResponseData::field_lengths() const 
{ return srdRep->priFieldLengths; }


inline const IntVector& SharedResponseData::num_coords_per_field() const
{ return srdRep->coordsPerPriField; }
  

inline const String& SharedResponseData::responses_id() const
{ return srdRep->responsesId; }


inline short SharedResponseData::response_type() const
{ return srdRep->responseType; }


// TODO: Seems this could require disconnecting/new rep?
inline void SharedResponseData::response_type(short type)
{ srdRep->responseType = type; }


inline short SharedResponseData::primary_fn_type() const
{ return srdRep->primaryFnType; }

inline const RealVector& SharedResponseData::simulation_error() const
{
  return srdRep->simulationVariance;
}


inline const String& SharedResponseData::function_label(size_t i) const
{ return srdRep->functionLabels[i]; }


inline const StringArray& SharedResponseData::function_labels() const
{ return srdRep->functionLabels; }


inline StringArray& SharedResponseData::function_labels()
{ return srdRep->functionLabels; }


inline void SharedResponseData::function_label(const String& label, size_t i)
{ srdRep->functionLabels[i] = label; }


inline void SharedResponseData::function_labels(const StringArray& labels)
{ srdRep->functionLabels = labels; }


inline const StringArray& SharedResponseData::field_group_labels() const
{ return srdRep->priFieldLabels; }


inline const StringArray& SharedResponseData::metadata_labels() const
{ return srdRep->metadataLabels; }


inline void SharedResponseData::metadata_labels(const StringArray& md_labels)
{ srdRep->metadataLabels = md_labels; }


inline void SharedResponseData::reshape_metadata(size_t num_meta)
{ reshape_labels(srdRep->metadataLabels, num_meta); }


inline void SharedResponseData::read_annotated(std::istream& s, size_t num_md)
{
  s >> srdRep->functionLabels;
  srdRep->metadataLabels.resize(num_md);
  s >> srdRep->metadataLabels;
}


inline bool SharedResponseData::is_null() const
{ return (srdRep == NULL); }


// SharedResponseData-related free fucntions

/// expand primary response specs in SerialDenseVectors, e.g. scales,
/// for fields no change on empty, expands 1 and num_groups, copies
/// num_elements
template<typename T>
void expand_for_fields_sdv(const SharedResponseData& srd, const T& src_array,
			   const String& src_desc, bool allow_by_element,
			   T& expanded_array)
{
  size_t src_size = src_array.length();
  if (src_size == 0)
    return;  // leave expanded_array empty

  size_t resp_groups = srd.num_scalar_primary() + srd.num_field_response_groups();
  size_t resp_elements = srd.num_scalar_primary() + srd.num_field_functions();

  expanded_array.sizeUninitialized(resp_elements);
  if (src_size == 1) {
    // TODO: consider leaving as length 1
    expanded_array = src_array[0];
  }
  else if (src_size == resp_groups) {
    // expand on per-group basis
    size_t src_ind = 0, elt_ind = 0;
    for (size_t i=0; i<srd.num_scalar_primary(); ++i, ++src_ind, ++elt_ind)
      expanded_array[i] = src_array[src_ind];
    for (size_t fi=0; fi<srd.num_field_response_groups(); ++fi, ++src_ind) {
      int fi_len = srd.field_lengths()[fi];
      for (size_t j=0; j<fi_len; ++j, ++elt_ind)
	expanded_array[elt_ind] = src_array[src_ind];
    }
  }
  else if (allow_by_element && src_size == resp_elements) {
    expanded_array.assign(src_array);
  }
  else {
    if (allow_by_element)
      Cerr << "Error: " << src_desc << " must have length 1, number of responses, or\n"
	   << "number of response elements (scalar + sum of field lengths);\n"
	   << "found length " << src_size << std::endl;
    else
      Cerr << "Error: " << src_desc << " must have length 1 or number of responses;\n"
	   << "found length " << src_size << std::endl;
    abort_handler(PARSE_ERROR);
  }
}

/// expand primary response specs in STL containers, e.g. scale types,
/// for fields no change on empty, expands 1 and num_groups, copies
/// num_elements
template<typename T>
void expand_for_fields_stl(const SharedResponseData& srd, const T& src_array,
			   const String& src_desc, bool allow_by_element,
			   T& expanded_array)
{
  size_t src_size = src_array.size();
  if (src_size == 0)
    return;  // leave expanded_array empty

  size_t resp_groups = srd.num_scalar_primary() + srd.num_field_response_groups();
  size_t resp_elements = srd.num_scalar_primary() + srd.num_field_functions();

  if (src_size == 1) {
    // TODO: consider leaving as length 1
    expanded_array.assign(resp_elements, src_array[0]);
  }
  else if (src_size == resp_groups) {
    // expand on per-group basis
    expanded_array.resize(resp_elements);
    size_t src_ind = 0, elt_ind = 0;
    for (size_t i=0; i<srd.num_scalar_primary(); ++i, ++src_ind, ++elt_ind)
      expanded_array[i] = src_array[src_ind];
    for (size_t fi=0; fi<srd.num_field_response_groups(); ++fi, ++src_ind) {
      int fi_len = srd.field_lengths()[fi];
      for (size_t j=0; j<fi_len; ++j, ++elt_ind)
	expanded_array[elt_ind] = src_array[src_ind];
    }
  }
  else if (allow_by_element && src_size == resp_elements) {
    expanded_array = src_array;
  }
  else {
    if (allow_by_element)
      Cerr << "Error: " << src_desc << " must have length 1, number of responses, or\n"
	   << "number of response elements (scalar + sum of field lengths);\n"
	   << "found length " << src_size << std::endl;
    else
      Cerr << "Error: " << src_desc << " must have length 1 or number of responses;\n"
	   << "found length " << src_size << std::endl;
    abort_handler(PARSE_ERROR);
  }
}


} // namespace Dakota


// Since we may serialize this class through a temporary, disallow tracking.
// We allow tracking on SharedResponseDataRep as we want to serialize each
// unique pointer exactly once (may need to revisit this).
BOOST_CLASS_TRACKING(Dakota::SharedResponseData, 
  		     boost::serialization::track_never)
// Version 1 adds metadata
BOOST_CLASS_VERSION(Dakota::SharedResponseDataRep, 1)

#endif
