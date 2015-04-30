/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file GenericKey.h
 * 
 * \deprecated definition of utilib::GenericKey class 
 */

#ifndef utilib_GenericKey_h
#define utilib_GenericKey_h

#include <utilib/std_headers.h>
#include <utilib/PackObject.h>

namespace utilib {

/// A generic key object for use with UTILIB abstract data types (e.g.
/// heaps, hash tables, etc).
template <class KEY, class DATA>
class GenericKey
{
public:

  /// Return true if \a key equals the current key.
  virtual bool operator==(const GenericKey<KEY,DATA>& key) const
		{return (Key == key.Key);}

  /// Return true if the current key is less than \a key.
  virtual bool operator<(const GenericKey<KEY,DATA>& key) const
		{return (Key < key.Key);}

  /// Return true if the current key is greater than \a key.
  virtual bool operator>(const GenericKey<KEY,DATA>& key) const
		{return (Key > key.Key);}

  /**
   * Compare \a key with the current key, returning a value that is 
   * consistent with the 
   * \if GeneratingLaTeX standard comparison semantics (see Section~\ref{compare} \endif
   * \if GeneratingHTML \ref compare "standard comparison semantics" \endif
   */
  virtual int compare(const GenericKey<KEY,DATA>& val) const
		{return (Key < val.key() ? -1 : Key > val.key() ? 1 : 0);}

  /// Return a reference to the data.
  DATA& data()
		{return Data;}

  /// Return a reference to the key.
  KEY&  key()
		{return Key;}

  /// Return a const reference to the key.
  const KEY&  key() const
		{return Key;}

protected:

  /// The key.
  KEY Key;

  /// The data.
  DATA Data;
};

} // namespace utilib

/// Out-stream operator for writing the contents of a GenericKey
template <class KEY, class DATA>
inline std::ostream& operator<<(std::ostream& os, const utilib::GenericKey<KEY,DATA>& obj)
{ os << obj.Key << " " << obj.Data; return(os); }

/// Out-stream operator for writing the contents of a GenericKey
template <class KEY, class DATA>
inline utilib::PackBuffer& operator<<(utilib::PackBuffer& os,
                                        const utilib::GenericKey<KEY,DATA>& obj)
{ os << obj.Key << obj.Data; return(os); }

/// In-stream operator for reading the contents of a GenericKey
template <class KEY, class DATA>
inline std::istream& operator>>(std::istream& is, utilib::GenericKey<KEY,DATA>& obj)
{ is >> obj.Key >> obj.Data; return(is); }

/// In-stream operator for reading the contents of a GenericKey
template <class KEY, class DATA>
inline utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is,
                                        utilib::GenericKey<KEY,DATA>& obj)
{ is >> obj.Key >> obj.Data; return(is); }

#endif
