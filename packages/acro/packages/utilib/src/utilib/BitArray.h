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
 * \file BitArray.h
 *
 * Defines the \ref utilib::BitArray class
 */

#ifndef utilib_BitArray_h
#define utilib_BitArray_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/BitArrayBase.h>

namespace utilib {

using std::runtime_error;

/**
 * An array of binary values.
 *
 * The main elements of this
 * array have the same look and feel as a utilib::BasicArray object.  However,
 * this class uses a special, compact representation of the data elements,
 * which forces it to be a separate class.  This class does not impose
 * a restriction on the maximum length of the bit array.
 */
class BitArray : public BitArrayBase<0,int,BitArray>
{
public:

  #if !defined(DOXYGEN)
  typedef BitArrayBase<0,int,BitArray> base_t;
  #if !defined(UTILIB_SOLARIS_CC)
  using base_t::Len;
  #endif
  #endif

  /// Empty constructor.
   BitArray() : base_t() { }
  
  /// Explicit constructor.  Note -- this no longer takes a "char*"
  /// argument, but a "bitword*" (generally same as unsigned int).
  explicit
  BitArray(const size_type len, 
	   bitword* d=0, 
	   const EnumDataOwned o=DataNotOwned) :
    base_t(len,d,o)
   { }

  /// Copy constructor.
  BitArray(const BitArray& array) :
    base_t(array.size(),array.data(),DataOwned)
  { 
     // This equality test appears to be necessary to prevent icc/ld
     // from optimizing away the serialization registration
     BitArray::registrations_complete == true; 
  }

  /// Disconnect from any prior data and copy the data from object \b array
  BitArray& operator=(const BitArray& array)
        {
        this->base_t::operator=(array);
        return *this;
        }

  /// Set a particular bit
  void set(const size_type ndx);

  /// Set all bits \c ON
  void set();

  /// Set the \a ndx-th bit to \c OFF
  void reset(const size_type ndx);

  /// Set all bits to \c OFF
  void reset();

  /// Flip bit value of the \a ndx-th bit
  void flip(const size_type ndx);
  
  /// Flip all bits
  void flip();

  /// Returns the number of bits that are set \c ON.
  size_type nbits() const;

  /// Perform a bitwise OR between two arrays
  friend void bitwise_or(BitArray& a1, BitArray& a2, BitArray& result);

  /// Perform a bitwise XOR between two arrays
  friend void bitwise_xor(BitArray& a1, BitArray& a2, BitArray& result);

  /// Perform a bitwise AND between two arrays
  friend void bitwise_and(BitArray& a1, BitArray& a2, BitArray& result);

  /** Return true (1) if the two BitArrays both have a 1 in at least one
      bit.  Equivalent to a bitwise AND and then comparing nbits() on 
      the result to 0, but much more efficient (e.g. can return as soon 
      as find agreement */
  int shared_one(BitArray& other) const;

protected:

  /// Count the bits in a buffer
  inline static void count_bits(size_type& counter,workingBitData stuff)
    {
      while(stuff)
	{
	  counter += (stuff & 1);
	  stuff  >>= 1;
	}
    }

private:
   static const volatile bool registrations_complete;
   static bool register_aux_functions();

   /// The BitArray serializer / deserializer
   static int serializer( SerialObject::elementList_t& serial, 
                          Any& data, bool serialize );
};


///
/// Inline method definitions
///
 
inline void BitArray::set(const size_type ndx)
{
#if (ArraySanityChecking==1)
if (ndx >= Len)
   EXCEPTION_MNGR(runtime_error, "BitArray::set(ndx) : iterator out of range. ndx=" << ndx << " len=" << Len);
#endif
  Data[ndx >> index_shift] |= 1 << (ndx & index_mask);
}

inline void BitArray::reset(const size_type ndx)
{
#if (ArraySanityChecking==1)
if (ndx >= Len)
   EXCEPTION_MNGR(runtime_error, "BitArray::reset(ndx) : iterator out of range. ndx=" << ndx << " len=" << Len);
#endif
  workingBitData mask = 1 << (ndx & index_mask);
  Data[ndx >> index_shift] &= ~mask;
}


inline void BitArray::flip(const size_type ndx)
{
#if (ArraySanityChecking==1)
if (ndx >= Len)
   EXCEPTION_MNGR(runtime_error, "BitArray::flip(ndx) : iterator out of range. ndx=" << ndx << " len=" << Len);
#endif
  Data[ndx >> index_shift] ^= 1 << (ndx & index_mask);
}

DEFINE_DEFAULT_ANY_PRINTER(BitArray);

} // namespace utilib

/// Set equal to a constant
inline utilib::BitArray& operator<<(utilib::BitArray& x, int val)
{
if (val == 0)
   x.reset();
else if (val == 1)
   x.set();
else
   EXCEPTION_MNGR(std::runtime_error, "operator<< : value " << val << " out of range.");
return x;
}


#endif
