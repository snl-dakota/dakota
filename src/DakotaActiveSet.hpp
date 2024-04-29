/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_ACTIVE_SET_H
#define DAKOTA_ACTIVE_SET_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"
#include "MPIPackBuffer.hpp"
//#include <boost/serialization/export.hpp>

namespace Dakota {

/// Container class for active set tracking information.  Contains the
/// active set request vector and the derivative variables vector.

/** The ActiveSet class is a small class whose initial design function
    is to avoid having to pass the ASV and DVV separately.  It is not
    part of a class hierarchy and does not employ reference-counting/
    representation-sharing idioms (e.g., handle-body). */

class ActiveSet
{
  //
  //- Heading: Friends
  //

  /// equality operator
  friend bool operator==(const ActiveSet& set1, const ActiveSet& set2);
  /// inequality operator
  friend bool operator!=(const ActiveSet& set1, const ActiveSet& set2);

public:

  //
  //- Heading: Constructors and destructor
  //

  ActiveSet();                                      ///< default constructor
  ActiveSet(size_t num_fns, size_t num_deriv_vars); ///< standard constructor
  ActiveSet(size_t num_fns);                        ///< partial constructor
  ActiveSet(const ShortArray& asv, const SizetArray& dvv); ///< alt constructor
  ActiveSet(const ActiveSet& set);                  ///< copy constructor
  ~ActiveSet();                                     ///< destructor

  //
  //- Heading: operators
  //

  /// assignment operator
  ActiveSet& operator=(const ActiveSet& set);

  //
  //- Heading: Member functions
  //

  /// reshape requestVector and derivVarsVector
  void reshape(size_t num_fns, size_t num_deriv_vars);
  /// reshape requestVector
  void reshape(size_t num_fns);

  /// return the request vector
  const ShortArray& request_vector() const;
  /// return the request vector
  ShortArray& request_vector();
  /// set the request vector
  void request_vector(const ShortArray& rv);
  /// set all request vector values
  void request_values(const short rv_val);
  /// set all request vector values in a range
  void request_values(const short rv_val, size_t start, size_t end);
  /// get the value of an entry in the request vector
  short request_value(const size_t index) const;
  /// set the value of an entry in the request vector
  void request_value(const short rv_val, const size_t index);

  /// return the derivative variables vector
  const SizetArray& derivative_vector() const;
  /// return the derivative variables vector
  SizetArray& derivative_vector();
  /// set the derivative variables vector from a SizetArray
  void derivative_vector(const SizetArray& dvv);
  /// set the derivative variables vector from a SizetMultiArrayConstView
  void derivative_vector(SizetMultiArrayConstView dvv);
  /// set the derivative variables vector values
  void derivative_start_value(size_t dvv_start_val);

  /// read an active set object from an std::istream
  void read(std::istream& s);
  /// write an active set object to an std::ostream
  void write(std::ostream& s) const;
  /// write an active set object to an std::ostream in annotated format
  void write_annotated(std::ostream& s) const;

  /// read an active set object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write an active set object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

private:

  friend class boost::serialization::access;

  /// implementation of Boost serialize for ActiveSet
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version);

  //
  //- Heading: Data
  //
 
  /// the vector of response requests
  /** It uses a 0 value for inactive functions and sums 1 (value),
      2 (gradient), and 4 (Hessian) for active functions. */
  ShortArray requestVector;
  /// the vector of variable ids used for computing derivatives
  /** These ids will generally identify either the active continuous
      variables or the inactive continuous variables. */
  SizetArray derivVarsVector;
};


inline ActiveSet::ActiveSet()
{ }


inline ActiveSet::ActiveSet(const ShortArray& asv, const SizetArray& dvv)
{ requestVector = asv; derivVarsVector = dvv; }


inline ActiveSet::ActiveSet(const ActiveSet& set)
{ requestVector = set.requestVector; derivVarsVector = set.derivVarsVector; }


inline ActiveSet::~ActiveSet()
{ }


inline ActiveSet& ActiveSet::operator=(const ActiveSet& set)
{
  requestVector   = set.requestVector;
  derivVarsVector = set.derivVarsVector;
  return *this;
}


//inline bool operator==(const ActiveSet& set1, const ActiveSet& set2)
//{
//  return (set1.requestVector   == set2.requestVector &&
//	    set1.derivVarsVector == set2.derivVarsVector);
//}


//inline bool operator!=(const ActiveSet& set1, const ActiveSet& set2)
//{ return !(set1 == set2); }


inline void ActiveSet::reshape(size_t num_fns)
{
  size_t curr_fns = requestVector.size();
  if (curr_fns != num_fns) {
    requestVector.resize(num_fns);
    if (curr_fns && num_fns > curr_fns) // inflate: repeat pattern
      for (size_t i=curr_fns; i<num_fns; ++i)
	requestVector[i] = requestVector[i % curr_fns];
  }
}


inline void ActiveSet::reshape(size_t num_fns, size_t num_deriv_v)
{
  reshape(num_fns);
  if (derivVarsVector.size() != num_deriv_v) {
    derivVarsVector.resize(num_deriv_v);
    // DVV value management has not been necessary since Iterator sets
    // ActiveSet which propagates through Model to Response
    //if (curr_deriv_v && num_deriv_v > curr_deriv_v) // inflate
    //  continue sequence?
  }
}


inline const ShortArray& ActiveSet::request_vector() const
{ return requestVector; }


inline ShortArray& ActiveSet::request_vector()
{ return requestVector; }


inline void ActiveSet::request_vector(const ShortArray& rv)
{ requestVector = rv; }


inline void ActiveSet::request_values(const short rv_val)
{ requestVector.assign(requestVector.size(), rv_val); }


inline void ActiveSet::
request_values(const short rv_val, size_t start, size_t end)
{
  for (size_t i=start; i<end; ++i)
    requestVector[i] = rv_val;
}


inline short ActiveSet::request_value(const size_t index) const
{ return requestVector[index]; }


inline void ActiveSet::request_value(const short rv_val, const size_t index)
{ requestVector[index] = rv_val; }


inline const SizetArray& ActiveSet::derivative_vector() const
{ return derivVarsVector; }


inline SizetArray& ActiveSet::derivative_vector()
{ return derivVarsVector; }


inline void ActiveSet::derivative_vector(const SizetArray& dvv)
{ derivVarsVector = dvv; }


inline void ActiveSet::derivative_vector(SizetMultiArrayConstView dvv)
{ copy_data(dvv, derivVarsVector); }


inline void ActiveSet::derivative_start_value(size_t dvv_start_val)
{
  size_t i, dvv_len = derivVarsVector.size();
  for (i=0; i<dvv_len; ++i)
    derivVarsVector[i] = dvv_start_val+i;
}


inline void ActiveSet::read(std::istream& s)
{ s >> requestVector >> derivVarsVector; }


inline void ActiveSet::write(std::ostream& s) const
{ s << requestVector << derivVarsVector; }


inline void ActiveSet::write_annotated(std::ostream& s) const
{
  array_write_annotated(s,   requestVector, false);
  array_write_annotated(s, derivVarsVector, false);
}


inline void ActiveSet::read(MPIUnpackBuffer& s)
{ s >> requestVector >> derivVarsVector; }


inline void ActiveSet::write(MPIPackBuffer& s) const
{ s << requestVector << derivVarsVector; }


/// std::istream extraction operator for ActiveSet.  Calls read(std::istream&).
inline std::istream& operator>>(std::istream& s, ActiveSet& set)
{ set.read(s); return s; }


/// std::ostream insertion operator for ActiveSet.  Calls write(std::istream&).
inline std::ostream& operator<<(std::ostream& s, const ActiveSet& set)
{ set.write(s); return s; }


/// MPIUnpackBuffer extraction operator for ActiveSet.
/// Calls read(MPIUnpackBuffer&).
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, ActiveSet& set)
{ set.read(s); return s; }


/// MPIPackBuffer insertion operator for ActiveSet. Calls write(MPIPackBuffer&).
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const ActiveSet& set)
{ set.write(s); return s; }


/// inequality operator for ActiveSet
inline bool operator!=(const ActiveSet& set1, const ActiveSet& set2)
{ return !(set1 == set2); }

} // namespace Dakota


// Since we may serialize this class through a temporary, disallow tracking
BOOST_CLASS_TRACKING(Dakota::ActiveSet, 
		     boost::serialization::track_never)

#endif
