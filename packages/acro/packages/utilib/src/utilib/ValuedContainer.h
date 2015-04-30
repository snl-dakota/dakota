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
 * \file ValuedContainer.h
 *
 * Defines the utilib::ValuedContainer class. 
 */

#ifndef utilib_ValuedContainer_h
#define utilib_ValuedContainer_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/PackObject.h>

namespace utilib {

/// A container class that contains a value that can be extracted.  For example,
/// this is useful to sort objects that have a value with auxiliary 
/// information.
template <class ValueT, class InfoT>
class ValuedContainer : public PackObject
{
public:

  /// Empty constructor
  ValuedContainer() : PackObject() {}

  /// Constructor
  explicit ValuedContainer(ValueT& value_, InfoT& info_) 
     : PackObject(),
       value(value_),
       info(info_)
  {}

  /// Copy constructor
  ValuedContainer(const ValuedContainer<ValueT,InfoT>& item)
     : PackObject()
  { value = item.value; info = item.info; }

  /// The value of this object.
  ValueT value;

  /// A data structure for auxiliary data associated with this value.
  InfoT info;

  /// Returns the value of this object.
  operator ValueT() const
	  {return value;}

  /// Compares two ValuedContainer objects.
  int compare(const ValuedContainer<ValueT,InfoT>& item) const
	{
	if (value < item.value) return -1;
	if (value > item.value) return  1;
	return 0;
	}

    /// Write this object to a buffer
    void write(PackBuffer& os) const
    { os << value << info; }

    /// Read this object from a buffer
    void read(UnPackBuffer& is)
    { is >> value >> info; }
 
};

}

/// A stream operator for writing a ValuedContainer object
template <class ValueT, class InfoT>
std::ostream& operator<<(std::ostream& os, const utilib::ValuedContainer<ValueT,InfoT>& item)
{
os << item.value << " " << item.info;
return os;
}


#endif
