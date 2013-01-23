/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

  /// write a ParamResponsePair object in tabular format to an std::ostream
  void write_tabular(std::ostream& s)   const;

  /// read a ParamResponsePair object from the binary restart stream
  void read(BiStream& s);
  /// write a ParamResponsePair object to the binary restart stream
  void write(BoStream& s)          const;

  /// read a ParamResponsePair object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a ParamResponsePair object to a packed MPI buffer
  void write(MPIPackBuffer& s)     const;

  //
  //- Heading: Set and Inquire functions
  //

  /// return the evaluation identifier
  int eval_id()                             const;
  /// return the interface identifier from the response object
  const String& interface_id()              const;
  /// return the aggregate eval/interface identifier from the response object
  const IntStringPair& eval_interface_ids() const;
  /// return the parameters object
  const Variables& prp_parameters()         const;
  /// return the response object
  const Response&  prp_response()           const;
  /// set the response object
  void prp_response(const Response& response);
  /// return the active set object from the response object
  const ActiveSet& active_set()             const;
  /// set the active set object within the response object
  void active_set(const ActiveSet& set);

private:

  //
  //- Heading: Data
  //

  /// the set of parameters for the function evaluation
  Variables prPairParameters;
  /// the response set for the function evaluation
  Response  prPairResponse;

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
  prPairParameters( (deep_copy) ? vars.copy()     : vars     ),
  prPairResponse(   (deep_copy) ? response.copy() : response ),
  evalInterfaceIds(0, interface_id)
{ }


/** Uses of this constructor often do not share representations since
    deep copies are used when history mechanisms (e.g., data_pairs and
    beforeSynchCorePRPQueue) are involved. */
inline ParamResponsePair::
ParamResponsePair(const Variables& vars, const String& interface_id,
		  const Response& response, const int eval_id, bool deep_copy):
  prPairParameters( (deep_copy) ? vars.copy()     : vars     ),
  prPairResponse(   (deep_copy) ? response.copy() : response ),
  evalInterfaceIds(eval_id, interface_id)
{ }


inline ParamResponsePair::ParamResponsePair(const ParamResponsePair& pair):
  prPairParameters(pair.prPairParameters),
  prPairResponse(pair.prPairResponse),
  evalInterfaceIds(pair.evalInterfaceIds)
{ }


inline ParamResponsePair&
ParamResponsePair::operator=(const ParamResponsePair& pair)
{
  prPairParameters = pair.prPairParameters;
  prPairResponse   = pair.prPairResponse;
  evalInterfaceIds = pair.evalInterfaceIds;

  return *this;
}


inline ParamResponsePair::~ParamResponsePair()
{ }


inline int ParamResponsePair::eval_id() const
{ return evalInterfaceIds.first; }


inline const String& ParamResponsePair::interface_id() const
{ return evalInterfaceIds.second; }


inline const IntStringPair& ParamResponsePair::eval_interface_ids() const
{ return evalInterfaceIds; }


inline const Variables& ParamResponsePair::prp_parameters() const
{ return prPairParameters; }


inline const Response& ParamResponsePair::prp_response() const
{ return prPairResponse; }


inline void ParamResponsePair::prp_response(const Response& response)
{ prPairResponse = response; }


inline const ActiveSet& ParamResponsePair::active_set() const
{ return prPairResponse.active_set(); }


inline void ParamResponsePair::active_set(const ActiveSet& set)
{ prPairResponse.active_set(set); }


// The binary read and write operators are used to read from and write to the 
// binary restart file and the ASCII write operator is used to echo a pair
// read from the restart file to cout (in manage_restart() in main.cpp). The 
// ASCII read operator is not currently used. The MPIPackBuffer/MPIUnpackBuffer
// operators are used to pass a source point for the continuation algorithm.
inline void ParamResponsePair::read(std::istream& s)
{ s >> prPairParameters >> prPairResponse; }


inline void ParamResponsePair::write(std::ostream& s) const
{
  s << "Parameters:\n" << prPairParameters;
  if (!evalInterfaceIds.second.empty())
    Cout << "\nInterface identifier = " << evalInterfaceIds.second << '\n';
  s << "\nActive response data:\n"<< prPairResponse << std::endl;
}


inline void ParamResponsePair::write_tabular(std::ostream& s) const
{
  s << std::setw(8) << evalInterfaceIds.first << ' ';
  prPairParameters.write_tabular(s);
  prPairResponse.write_tabular(s);
}


/// std::istream extraction operator for ParamResponsePair
inline std::istream& operator>>(std::istream& s, ParamResponsePair& pair)
{ pair.read(s); return s; }


/// std::ostream insertion operator for ParamResponsePair
inline std::ostream& operator<<(std::ostream& s, const ParamResponsePair& pair)
{ pair.write(s); return s; }


inline void ParamResponsePair::read(BiStream& s)
{
  s >> prPairParameters >> evalInterfaceIds.second
    >> prPairResponse   >> evalInterfaceIds.first;
}


// BDS/MSE: The flush here is essential so that a complete restart file exists
// between function evaluations.  Otherwise an incomplete record can exist
// between evals which is vulnerable to a kill of the DAKOTA process.
inline void ParamResponsePair::write(BoStream& s) const
{
  s << prPairParameters << evalInterfaceIds.second
    << prPairResponse   << evalInterfaceIds.first; //<< flush;
  s.flush();
}


/// BiStream extraction operator for ParamResponsePair
inline BiStream& operator>>(BiStream& s, ParamResponsePair& pair)
{ pair.read(s); return s; }


/// BoStream insertion operator for ParamResponsePair
inline BoStream& operator<<(BoStream& s, const ParamResponsePair& pair)
{ pair.write(s); return s; }


/** interfaceId is omitted since master processor retains interface
    ids and communicates asv and response data only with slaves. */
inline void ParamResponsePair::read(MPIUnpackBuffer& s)
{ s >> prPairParameters >> prPairResponse >> evalInterfaceIds.first; }


/** interfaceId is omitted since master processor retains interface
    ids and communicates asv and response data only with slaves. */
inline void ParamResponsePair::write(MPIPackBuffer& s) const
{ s << prPairParameters << prPairResponse << evalInterfaceIds.first; }


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
  return (pair1.prPairParameters        == pair2.prPairParameters &&
	  pair1.evalInterfaceIds.second == pair2.evalInterfaceIds.second &&
	  pair1.prPairResponse          == pair2.prPairResponse);
}


/// inequality operator for ParamResponsePair
inline bool operator!=(const ParamResponsePair& pair1,
		       const ParamResponsePair& pair2)
{ return !(pair1 == pair2); }

} // namespace Dakota

#endif
