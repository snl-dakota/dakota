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
 * \file TwoBitArray.h
 *
 * Defines the utilib::TwoBitArray class
 */

#ifndef utilib_TwoBitArray_h
#define utilib_TwoBitArray_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/BitArrayBase.h>

namespace utilib {


/**
 * An array of two-bit values.   The main elements of 
 * this array have the same look and feel as a utilib::BasicArray object.
 * However, 
 * this class uses a special, compact representation of the data elements, 
 * which forces it to be a separate class.  This class does not impose a 
 * restriction on the maximum length of the array.
 */
class TwoBitArray : public BitArrayBase<1,int,TwoBitArray> {

public:

  /// Empty constructor.
  TwoBitArray() : BitArrayBase<1,int,TwoBitArray>() { }
  
  /// Explicit constructor.  Note -- this no longer takes a "char*"
  /// argument, but a "bitword*" (generally same as unsigned int).
  explicit
  TwoBitArray(const size_type len, 
	      bitword* d=0, 
	      const EnumDataOwned o=DataNotOwned) :
    BitArrayBase<1,int,TwoBitArray>(len,d,o)
  { }

  /// Copy constructor.
  TwoBitArray(const TwoBitArray& array) :
    BitArrayBase<1,int,TwoBitArray>(array.Len,array.Data,DataOwned)
  { }

  /// Disconnect from any prior data and copy the data from object \b array
  TwoBitArray& operator=(const TwoBitArray& array)
        {
        this->BitArrayBase<1,int,TwoBitArray>::operator=(array);
        return *this;
        }

};

} // namespace utilib

#endif
