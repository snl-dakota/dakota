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
 * \file PackObject.h
 *
 * Defines the utilib::PackObject class.
 *
 * \author William E. Hart
 */

#ifndef utilib_PackObject_h
#define utilib_PackObject_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/PackBuf.h>
#include <utilib/exception_mngr.h>

namespace utilib {

/**
 * A generic object that can be packed and unpacked.
 */
class PackObject
{
public:
  /// Empty virtual destructor to silence compiler warnings
  virtual ~PackObject() {}

  /// Write this object to a stream
  virtual void write(std::ostream&) const
	{EXCEPTION_MNGR(std::runtime_error, "Undefined method.");}

  /// Read this object from a stream
  virtual void read(std::istream&)
	{EXCEPTION_MNGR(std::runtime_error, "Undefined method.");}

  /// Write this object to a buffer
  virtual void write(PackBuffer&) const
	{EXCEPTION_MNGR(std::runtime_error, "Undefined method.");}

  /// Read this object from a buffer
  virtual void read(UnPackBuffer&)
	{EXCEPTION_MNGR(std::runtime_error, "Undefined method.");}

};

} // namespace utilib

/// Out-stream operator for writing the contents of a PackObject
inline std::ostream& operator<<(std::ostream& os, const utilib::PackObject& obj)
{ obj.write(os); return(os); }

/// Out-stream operator for writing the contents of a PackObject
inline utilib::PackBuffer& operator<<(utilib::PackBuffer& os,
					const utilib::PackObject& obj)
{ obj.write(os); return(os); }

/// In-stream operator for reading the contents of a PackObject
inline std::istream& operator>>(std::istream& is, utilib::PackObject& obj)
{ obj.read(is); return(is); }

/// In-stream operator for reading the contents of a PackObject
inline utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is,
					utilib::PackObject& obj)
{ obj.read(is); return(is); }


#endif
