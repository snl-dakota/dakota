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
  /// destructor
  ~SharedResponseDataRep();

  //
  //- Heading: Member functions
  //

  /// copy the data from srd_rep to the current representation
  void copy_rep(SharedResponseDataRep* srd_rep);

  //
  //- Heading: Data
  //

  /// enumeration of BASE_RESPONSE, SIMULATION_RESPONSE, or EXPERIMENT_RESPONSE
  short responseType;
  /// response identifier string from the input file
  String responsesId;

  /// response function identifiers used to improve output readability
  StringArray functionLabels;

  /// number of scalar responses
  size_t numScalarResponses;
  /// index of field lengths for field data 
  IntVector fieldRespGroupLengths;

  /// dimensions of each function
  IntVectorArray numCoordsPerField;
  
  /// number of handle objects sharing srdRep
  int referenceCount;
};


inline SharedResponseDataRep::SharedResponseDataRep():
  // copy and reshape immediately apply copy_rep, so don't initialize anything:
  //responseType(BASE_RESPONSE), // overridden in derived class ctors
  //responsesId("NO_SPECIFICATION"), numScalarResponses(0),
  referenceCount(1)
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
  /// dimensions of each function
  const IntVectorArray& num_coords_per_field();

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

  /// return the response identifier
  const String& responses_id() const;

  /// return the response type: {BASE,SIMULATION,EXPERIMENT}_RESPONSE
  short response_type() const;
  /// set the response type: {BASE,SIMULATION,EXPERIMENT}_RESPONSE
  void response_type(short type);

  /// create a deep copy of the current object and return by value
  SharedResponseData copy() const;
  /// reshape the data, disconnecting a shared rep if necessary
  void reshape(size_t num_fns);

  /// return true if empty handle with null representation
  bool is_null() const;

private:

  //
  //- Heading: Private data members
  //
 
  /// pointer to the body (handle-body idiom)
  SharedResponseDataRep* srdRep;
};


inline SharedResponseData::SharedResponseData(): srdRep(NULL)
{ }


inline SharedResponseData::SharedResponseData(const ProblemDescDB& problem_db):
  srdRep(new SharedResponseDataRep(problem_db))
{ }


inline SharedResponseData::SharedResponseData(const ActiveSet& set):
  srdRep(new SharedResponseDataRep(set))
{ }


inline SharedResponseData::SharedResponseData(const SharedResponseData& srd)
{
  // Increment new (no old to decrement)
  srdRep = srd.srdRep;
  if (srdRep) // Check for an assignment of NULL
    ++srdRep->referenceCount;
}


inline SharedResponseData& SharedResponseData::
operator=(const SharedResponseData& srd)
{
  if (srdRep != srd.srdRep) { // normal case: old != new
    // Decrement old
    if (srdRep) // Check for NULL
      if ( --srdRep->referenceCount == 0 ) 
	delete srdRep;
    // Assign and increment new
    srdRep = srd.srdRep;
    if (srdRep) // Check for an assignment of NULL
      ++srdRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

  return *this;
}


inline SharedResponseData::~SharedResponseData()
{
  if (srdRep) { // Check for NULL
    --srdRep->referenceCount; // decrement
    if (srdRep->referenceCount == 0)
      delete srdRep;
  }
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


inline const IntVectorArray& SharedResponseData::num_coords_per_field()
{ return srdRep->numCoordsPerField; }
  

inline const String& SharedResponseData::responses_id() const
{ return srdRep->responsesId; }


inline short SharedResponseData::response_type() const
{ return srdRep->responseType; }


inline void SharedResponseData::response_type(short type)
{ srdRep->responseType = type; }


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


inline bool SharedResponseData::is_null() const
{ return (srdRep == NULL); }

} // namespace Dakota

#endif
