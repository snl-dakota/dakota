/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SharedResponseData
//- Description: Encapsulation of data that can be shared among Response
//-              instances.
//- Owner:       Mike Eldred
//- Version: $Id: SharedResponseData.hpp 6784 2010-05-18 21:10:35Z mseldre $

#ifndef SHARED_RESPONSE_DATA_H
#define SHARED_RESPONSE_DATA_H

#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"
#include "DataResponses.hpp"
#include <boost/shared_ptr.hpp>
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

  /// build/update the unrolled field labels based on fieldLabels and
  /// group lengths
  void build_field_labels();

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
  
  /// labels for each field group
  StringArray fieldLabels;

  /// number of scalar responses
  size_t numScalarResponses;
  /// index of field lengths for field data 
  IntVector fieldRespGroupLengths;

  /// dimensions of each function
  IntVector numCoordsPerField;
  
};


inline SharedResponseDataRep::SharedResponseDataRep():
  // copy and reshape immediately apply copy_rep, so don't initialize anything:
  //:responseType(BASE_RESPONSE)//, // overridden in derived class ctors
  primaryFnType(GENERIC_FNS)
  //responsesId("NO_SPECIFICATION"), numScalarResponses(0),
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedResponseDataRep::SharedResponseDataRep() called to build "
       << "empty body object." << std::endl;
#endif
}


inline SharedResponseDataRep::~SharedResponseDataRep()
{ }


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

  /// number of scalar responses 
  size_t num_scalar_responses() const;
  /// number of field response groups
  size_t num_field_response_groups() const;
  /// total number of response groups (scalars + field groups)
  size_t num_response_groups() const;
  /// total number of field functions (1-norm of fieldRespGroupLengths)
  size_t num_field_functions() const;
  /// total number of response functions (scalars + 1-norm of
  /// fieldRespGroupLengths)
  size_t num_functions() const;

  /// index of field lengths for field data 
  const IntVector& field_lengths() const;
  /// set field lengths (if experiment different from simulation)
  void field_lengths(const IntVector& field_lengths);
  /// dimensions of each function
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

  /// return the coarse (per-group) field response labels
  const StringArray& field_group_labels();
  /// set the coarse field group labels (must agree with number fields)
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

  /// create a deep copy of the current object and return by value
  SharedResponseData copy() const;
  /// reshape the data, disconnecting a shared rep if necessary
  void reshape(size_t num_fns);

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
  boost::shared_ptr<SharedResponseDataRep> srdRep;
};


inline SharedResponseData::SharedResponseData()
{ 
#ifdef REFCOUNT_DEBUG
  Cout << "SharedResponseData::SRD(); default constructed handle.\n";
  Cout << "  srdRep use_count = " << srdRep.use_count() << std::endl;
#endif
}

inline SharedResponseData::SharedResponseData(const ProblemDescDB& problem_db):
  srdRep(new SharedResponseDataRep(problem_db))
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedResponseData::SRD(PDB) problem DB constructed handle.\n";
  Cout << "  srdRep use_count = " << srdRep.use_count() << std::endl;
#endif
}

inline SharedResponseData::SharedResponseData(const ActiveSet& set):
  srdRep(new SharedResponseDataRep(set))
{ 
#ifdef REFCOUNT_DEBUG
  Cout << "SharedResponseData::SRD(AS) ActiveSet constructed handle.\n";
  Cout << "  srdRep use_count = " << srdRep.use_count() << std::endl;
#endif
}


inline SharedResponseData::SharedResponseData(const SharedResponseData& srd)
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedResponseData::SRD(SRD) copy constructor.\n";
  Cout << "  srdRep use_count before = " << srdRep.use_count() << std::endl;
#endif
  // share the representation (body)
  srdRep = srd.srdRep;
#ifdef REFCOUNT_DEBUG
  Cout << "  srdRep use_count after  = " << srdRep.use_count() << std::endl;
#endif
}


inline SharedResponseData& SharedResponseData::
operator=(const SharedResponseData& srd)
{
#ifdef REFCOUNT_DEBUG
  Cout << "SharedResponseData::operator=.\n";
  Cout << "  srdRep use_count before = " << srdRep.use_count() << std::endl;
#endif
  // share the inbound representation (body) by copying the pointer
  srdRep = srd.srdRep;
#ifdef REFCOUNT_DEBUG
  Cout << "  srdRep use_count after  = " << srdRep.use_count() << std::endl;
#endif
  return *this;
}


inline SharedResponseData::~SharedResponseData()
{ 
  /* empty dtor in case we add virtual functions */
#ifdef REFCOUNT_DEBUG
  Cout << "SharedResponseData::~SRD called.\n";
  Cout << "  srdRep use_count = " << srdRep.use_count() << std::endl;
#endif
}


inline size_t SharedResponseData::num_scalar_responses() const
{ return srdRep->numScalarResponses; }


inline size_t SharedResponseData::num_field_response_groups() const
{ return srdRep->fieldRespGroupLengths.length(); }


inline size_t SharedResponseData::num_response_groups() const
{ return srdRep->numScalarResponses + srdRep->fieldRespGroupLengths.length(); }


inline size_t SharedResponseData::num_field_functions() const
{ return srdRep->fieldRespGroupLengths.normOne(); }


inline size_t SharedResponseData::num_functions() const
{ return srdRep->numScalarResponses + srdRep->fieldRespGroupLengths.normOne(); }


inline const IntVector& SharedResponseData::field_lengths() const 
{ return srdRep->fieldRespGroupLengths; }


inline const IntVector& SharedResponseData::num_coords_per_field() const
{ return srdRep->numCoordsPerField; }
  

inline const String& SharedResponseData::responses_id() const
{ return srdRep->responsesId; }


inline short SharedResponseData::response_type() const
{ return srdRep->responseType; }


// TODO: Seems this could require disconnecting/new rep?
inline void SharedResponseData::response_type(short type)
{ srdRep->responseType = type; }


inline short SharedResponseData::primary_fn_type() const
{ return srdRep->primaryFnType; }


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


inline const StringArray& SharedResponseData::field_group_labels()
{ return srdRep->fieldLabels; }


inline bool SharedResponseData::is_null() const
{ return (srdRep == NULL); }

} // namespace Dakota


// Since we may serialize this class through a temporary, force
// serialization mode and no tracking.  We allow tracking on
// SharedResponseDataRep as we want to serialize each unique pointer
// exactly once (may need to revisit this).
BOOST_CLASS_IMPLEMENTATION(Dakota::SharedResponseData, 
 			   boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(Dakota::SharedResponseData, 
  		     boost::serialization::track_never)

#endif
