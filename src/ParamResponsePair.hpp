/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ParamResponsePair
//- Description:  A container for a variables object, a response object,
//-               and interface and evaluation ids.
//-
//- Owner:        Mike Eldred
//- Version: $Id

#ifndef PARAM_RESPONSE_PAIR_H
#define PARAM_RESPONSE_PAIR_H

#include "dakota_data_types.hpp"
#include "dakota_data_io.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"

namespace Dakota {


/// Container class for a variables object, a response object, and an
/// evaluation id.

/** ParamResponsePair provides a container class for association of
    the input for a particular function evaluation (a variables
    object) with the output from this function evaluation (a response
    object), along with an evaluation identifier.  This container
    defines the basic unit used in the data_pairs cache, in restart
    file operations, and in a variety of scheduling algorithm queues.
    With the advent of STL, replacement of arrays of this class with
    map<> and pair<> template constructs may be possible (using
    map<pair<int,String>, pair<Variables,Response> >, for example),
    assuming that deep copies, I/O, alternate constructors, etc., can
    be adequately addressed.  Boost tuple<> may also be a candidate. */

class ParamResponsePair
{
  //
  //- Heading: Friends
  //

  /// equality operator
  friend bool operator==(const ParamResponsePair& pair1, 
			 const ParamResponsePair& pair2);
  /// inequality operator
  friend bool operator!=(const ParamResponsePair& pair1, 
			 const ParamResponsePair& pair2);

  /// allow boost access to serialize this class
  friend class boost::serialization::access;

public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// default constructor
  ParamResponsePair();
  /// alternate constructor for temporaries
  ParamResponsePair(const Variables& vars, const String& interface_id,
		    const Response& response, bool deep_copy = false);
  /// standard constructor for history uses
  ParamResponsePair(const Variables& vars, const String& interface_id,
		    const Response& response, const int eval_id,
		    bool deep_copy = true);
  /// copy constructor
  ParamResponsePair(const ParamResponsePair& pair);

  /// destructor
  ~ParamResponsePair();

  /// assignment operator
  ParamResponsePair& operator=(const ParamResponsePair& pair);

  //
  //- Heading: Member functions
  //

  /// read a ParamResponsePair object from an std::istream
  void read(std::istream& s);
  /// write a ParamResponsePair object to an std::ostream
  void write(std::ostream& s)           const;

  /// read a ParamResponsePair object in annotated format from an std::istream
  void read_annotated(std::istream& s);
  /// write a ParamResponsePair object in annotated format to an std::ostream
  void write_annotated(std::ostream& s) const;

  /// write a ParamResponsePair object in tabular format (all
  /// variables active/inactive) to an std::ostream
  void write_tabular(std::ostream& s, unsigned short tabular_format) const;

  /// write PRP labels in tabular format to an std::ostream
  void write_tabular_labels(std::ostream& s,
			    unsigned short tabular_format) const;

  /// read a ParamResponsePair object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a ParamResponsePair object to a packed MPI buffer
  void write(MPIPackBuffer& s)     const;

  //
  //- Heading: Set and Inquire functions
  //

  /// return the evaluation identifier
  int eval_id() const;
  /// set the evaluation identifier
  void eval_id(int id);
  /// return the interface identifier from evalInterfaceIds
  const String& interface_id() const;
  /// set the interface identifier within evalInterfaceIds
  void interface_id(const String& id);
  /// return the aggregate eval/interface identifier from the response object
  const IntStringPair& eval_interface_ids() const;
  /// return the parameters object
  const Variables& variables() const;
  /// set the parameters object
  void variables(const Variables& vars);
  /// return the response object
  const Response& response() const;
  /// set the response object
  void response(const Response& resp);
  /// return the active set object from the response object
  const ActiveSet& active_set() const;
  /// set the active set object within the response object
  void active_set(const ActiveSet& set);

private:

  /// serialize the PRP: write and read are symmetric for this class
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version);

  //
  //- Heading: Data
  //

  /// the set of parameters for the function evaluation
  Variables prpVariables;
  /// the response set for the function evaluation
  Response  prpResponse;

  /// the evalInterfaceIds aggregate
  /** the function evaluation identifier (assigned from Interface::evalIdCntr)
      is paired with the interface used to generate the response object.  Used
      in PRPCache id_vars_set_compare to prevent duplicate detection on results
      from different interfaces.  evalInterfaceIds belongs here rather than in
      Response since some Response objects involve consolidation of several fn
      evals (e.g., Model::synchronize_derivatives()) that are not, in total,
      generated by a single interface.  The prPair, on the other hand, is
      used for storage of all low level fn evals that get evaluated in
      ApplicationInterface::map(). */
  IntStringPair evalInterfaceIds;
};


inline ParamResponsePair::ParamResponsePair()
{ }


/** Uses of this constructor often employ the standard Variables and
    Response copy constructors to share representations since this
    constructor is commonly used for search_pairs (which are local
    instantiations that go out of scope prior to any changes to
    values; i.e., they are not used for history). */
inline ParamResponsePair::
ParamResponsePair(const Variables& vars, const String& interface_id, 
		  const Response& response, bool deep_copy):
  prpVariables( (deep_copy) ? vars.copy()     : vars     ),
  prpResponse(  (deep_copy) ? response.copy() : response ),
  evalInterfaceIds(0, interface_id)
{ }


/** Uses of this constructor often do not share representations since
    deep copies are used when history mechanisms (e.g., data_pairs and
    beforeSynchCorePRPQueue) are involved. */
inline ParamResponsePair::
ParamResponsePair(const Variables& vars, const String& interface_id,
		  const Response& response, const int eval_id, bool deep_copy):
  prpVariables( (deep_copy) ? vars.copy()     : vars     ),
  prpResponse(  (deep_copy) ? response.copy() : response ),
  evalInterfaceIds(eval_id, interface_id)
{ }


inline ParamResponsePair::ParamResponsePair(const ParamResponsePair& pair):
  prpVariables(pair.prpVariables), prpResponse(pair.prpResponse),
  evalInterfaceIds(pair.evalInterfaceIds)
{ }


inline ParamResponsePair&
ParamResponsePair::operator=(const ParamResponsePair& pair)
{
  prpVariables     = pair.prpVariables;
  prpResponse      = pair.prpResponse;
  evalInterfaceIds = pair.evalInterfaceIds;

  return *this;
}


inline ParamResponsePair::~ParamResponsePair()
{ }


inline int ParamResponsePair::eval_id() const
{ return evalInterfaceIds.first; }


inline void ParamResponsePair::eval_id(int id)
{ evalInterfaceIds.first = id; }


inline const String& ParamResponsePair::interface_id() const
{ return evalInterfaceIds.second; }


inline void ParamResponsePair::interface_id(const String& id)
{ evalInterfaceIds.second = id; }


inline const IntStringPair& ParamResponsePair::eval_interface_ids() const
{ return evalInterfaceIds; }


inline const Variables& ParamResponsePair::variables() const
{ return prpVariables; }


inline void ParamResponsePair::variables(const Variables& vars)
{ prpVariables = vars; }


inline const Response& ParamResponsePair::response() const
{ return prpResponse; }


inline void ParamResponsePair::response(const Response& resp)
{ prpResponse = resp; }


inline const ActiveSet& ParamResponsePair::active_set() const
{ return prpResponse.active_set(); }


inline void ParamResponsePair::active_set(const ActiveSet& set)
{ prpResponse.active_set(set); }


// The binary read and write operators are used to read from and write to the 
// binary restart file and the ASCII write operator is used to echo a pair
// read from the restart file to cout (in manage_restart() in main.cpp). The 
// ASCII read operator is not currently used. The MPIPackBuffer/MPIUnpackBuffer
// operators are used to pass a source point for the continuation algorithm.
inline void ParamResponsePair::read(std::istream& s)
{ s >> prpVariables >> prpResponse; }


inline void ParamResponsePair::write(std::ostream& s) const
{
  s << "Parameters:\n" << prpVariables;
  if (!evalInterfaceIds.second.empty())
    Cout << "\nInterface identifier = " << evalInterfaceIds.second << '\n';
  s << "\nActive response data:\n"<< prpResponse << std::endl;
}


/// std::istream extraction operator for ParamResponsePair
inline std::istream& operator>>(std::istream& s, ParamResponsePair& pair)
{ pair.read(s); return s; }


/// std::ostream insertion operator for ParamResponsePair
inline std::ostream& operator<<(std::ostream& s, const ParamResponsePair& pair)
{ pair.write(s); return s; }


/** interfaceId is omitted since master processor retains interface
    ids and communicates asv and response data only with slaves. */
inline void ParamResponsePair::read(MPIUnpackBuffer& s)
{ s >> prpVariables >> prpResponse >> evalInterfaceIds.first; }


/** interfaceId is omitted since master processor retains interface
    ids and communicates asv and response data only with slaves. */
inline void ParamResponsePair::write(MPIPackBuffer& s) const
{ s << prpVariables << prpResponse << evalInterfaceIds.first; }


/// MPIUnpackBuffer extraction operator for ParamResponsePair
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, ParamResponsePair& pair)
{ pair.read(s); return s; }


/// MPIPackBuffer insertion operator for ParamResponsePair
inline MPIPackBuffer& operator<<(MPIPackBuffer& s,const ParamResponsePair& pair)
{ pair.write(s); return s; }


/// equality operator for ParamResponsePair
inline bool operator==(const ParamResponsePair& pair1,
		       const ParamResponsePair& pair2)
{
  // equality check includes interfaceId; evalId need not match
  return (pair1.prpVariables            == pair2.prpVariables &&
	  pair1.evalInterfaceIds.second == pair2.evalInterfaceIds.second &&
	  pair1.prpResponse             == pair2.prpResponse);
}


/// inequality operator for ParamResponsePair
inline bool operator!=(const ParamResponsePair& pair1,
		       const ParamResponsePair& pair2)
{ return !(pair1 == pair2); }

} // namespace Dakota


// Since we may serialize this class through a temporary, force
// serialization mode and no tracking
BOOST_CLASS_IMPLEMENTATION(Dakota::ParamResponsePair, 
			   boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(Dakota::ParamResponsePair, 
		     boost::serialization::track_never)


#endif
