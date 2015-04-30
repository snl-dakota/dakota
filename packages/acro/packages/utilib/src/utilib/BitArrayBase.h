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
 * \file BitArrayBase.h
 *
 * Defines the utilib::BitArrayBase class
 */

#ifndef utilib_BitArrayBase_h
#define utilib_BitArrayBase_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/_generic.h>
#include <utilib/math_basic.h>
#include <utilib/PackBuf.h>
#include <utilib/ArrayBase.h>
#include <utilib/exception_mngr.h>

namespace utilib {

/********************* Configuration section ****************/

#if !defined(DOXYGEN)
// This section configures the code to work by storing data either in 
// 8-bit bytes or 32-bit words.  32-bit words is the default at the
// moment.  With some obvious modifications, it could also be set up
// to store in 16-bit short unsigned ints or 64-bit long long ints 
// (if available)

#if !defined(BIT_ARRAYS_BY_BYTE) && !defined(BIT_ARRAYS_BY_WORD)
#define BIT_ARRAYS_BY_WORD 1
#endif

// A "bitword" is the datatype used to store the array.

#ifdef BIT_ARRAYS_BY_WORD
typedef unsigned int bitword;
#elif defined(BIT_ARRAYS_BY_BYTE)
typedef unsigned char bitword;
#endif

// This symbol should be log_2 of the number of bits in "bitword"

#ifdef BIT_ARRAYS_BY_WORD
#define LOG2BITWORDSIZE 5
#elif defined(BIT_ARRAYS_BY_BYTE)
#define LOG2BITWORDSIZE 3
#endif

#endif

/// The working registers in which we shift and mask stuff are unsigned ints
/// This should be an unsigned type at least as long as bitword, preferably
/// it should be the natural register length of the host machine.
typedef unsigned int workingBitData;

/******************* End configuration section **************/

template <int k, class T, class P> 
class BitArrayBase;


/// A handle to the value of a BitArray at a given point.  This class
/// enables the use of operator[] with BitArray objects.
template <int k, class T, class P> 
class BitArrayBaseHandle
{
public:
  typedef BitArrayBase<k, T, P>  base_t;
  typedef BitArrayBaseHandle<k, T, P>  this_t;

  BitArrayBaseHandle(size_t _ndx, base_t* _base)
     : ndx(_ndx), base(_base) 
  {}

  BitArrayBaseHandle(const this_t& rhs)
     : ndx(rhs.ndx), base(rhs.base)
  {}

  /// Return the value of the value indexed by ndx
  operator T() const
	{return base->get(ndx);}

  /// Return the value of the value indexed by ndx
  T value() const
	{return base->get(ndx);}

  /// Set the value of array at ndx
  T operator=(T val)
	{
	base->put(ndx, val);
	return val;
	}

  /// Set the value of array at ndx
  T operator=(const this_t& val)
        {
        T tmp = val.value();
        base->put(ndx, tmp);
        return tmp;
        }

  /// The index of the BitArrayBase that is being referenced by this handle.
  size_t ndx;

  /// A pointer to the BitArrayBase that is being used
  BitArrayBase<k,T,P>* base;
};


/**
 * An array of binary values.
 *
 * The main elements of this
 * array have the same look and feel as a SimpleArray object.  However,
 * this class uses a special, compact representation of the data elements,
 * which forces it to be a separate class.  This class does not impose
 * a restriction on the maximum length of the bit array.
 *
 * The size of an element is determined by the template parameter \a k,
 * which causes elements to have size 2^k.  Thus, k=0 gives an array of
 * single bits, k=1 gives an array of bit-pairs, and k=2 gives an array
 * of 4-bit nibbles.
 *
 * This version was completely rewritten by Jonathan Eckstein to use
 * the sharing linked-list sharing mechanism in the "ArrayBase" class.
 * There are other major changes too, in the name of efficiency and
 * eliminating code duplication.
 *
 * The class parameter \a T gives the datatype passed to and from the outside
 * world; typically this will be \a int.  The class parameter \a P identifies
 * is used to set the type of internal linked list pointers used by the
 * sharing mechanism.
 *
 * In particular, this implementation is "little-endian", that is the first
 * element packed into a byte is in the least significant digits.  That is
 * slightly more efficient, and hopefully won't break anything.
 */
template <int k, class T, class P> 
class BitArrayBase : public ArrayBase<bitword,P>
{
public:

  #if !defined(DOXYGEN)
  typedef ArrayBase<bitword,P> base_t;
  #if !defined(UTILIB_SOLARIS_CC)
  using base_t::Len;
  using base_t::Data;
  #endif
  #endif

  /// Empty constructor.
  BitArrayBase<k,T,P>() { construct(0); }

  /// Explicit constructor.
  explicit
  BitArrayBase<k,T,P>(const size_type     len,
		      bitword*            d=0,
		      const EnumDataOwned o=DataNotOwned) 
    { construct(len,d,o); }

  /// Copy constructor
  BitArrayBase<k,T,P>(const BitArrayBase<k,T,P>& array)
     : ArrayBase<bitword,P>(array)
    { construct(array.Len,array.Data,DataOwned); }

  /// Overrides of the ArrayBase virtuals:
  /// This determines how many bitwords to allocate for l elements
  size_type alloc_size(size_type l) const
    {
#ifdef UTILIB_AIX_CC
      return (l + index_mask) >> (LOG2BITWORDSIZE - k); 
#else
      return (l + index_mask) >> index_shift; 
#endif
    }

  ///Returns the value of the \a ndx-th element
  inline T get(const size_type ndx) const;

  ///Returns the value of the \a ndx-th element
  inline T operator[](const size_type ndx) const
    { return get(ndx); }

  ///Returns the value of the \a ndx-th element
  inline BitArrayBaseHandle<k,T,P> operator[](const size_type ndx)
    { return BitArrayBaseHandle<k,T,P>(ndx, this); }

  /// The () operator is a synonym for [] here
  inline T operator()(const size_type ndx) const
    { return get(ndx); }

  /// Returns the number of bits in an element in the array.
  inline int element_size() const { return bits; }

  /// Put bit value \a val in the \a ndx-th bit-pair.
  inline void put(const size_type ndx, const T val);

  /// Disconnect from any prior data and copy the data from object \b array
  BitArrayBase<k,T,P>& operator=(const BitArrayBase<k,T,P>& array)
	{
	this->ArrayBase<bitword,P>::operator=(array);
	return *this;
	}

  /// Checks to see if the current array equals \b array.
  int operator==(const BitArrayBase<k,T,P>& array) const;

  /// Checks to see if the current array does not equal \b array.
  inline int operator!=(const BitArrayBase<k,T,P>& array) const
    { return !operator==(array); }

  /**
   * Compares the current array with \b array.
   * If the current array is longer, then returns 1. If shorter, then
   * returns -1.  Otherwise, does a pairwise comparison on elements of
   * both arrays.  At the first unequal pair of elements, it
   * returns a positive value if the current array is greater 
   * a negative value otherwise. If the arrays are equal, it returns 0.
   */
  int compare(const BitArrayBase<k,T,P>& array) const;

  /// Perform an element-wise comparison for greater-than
  int operator>(const BitArrayBase<k,T,P>& vec) const
    { return (compare(vec) > 0); }

  /// Perform an element-wise comparison for greater-than-or-equal-to
  int operator>=(const BitArrayBase<k,T,P>& vec) const
    { return (compare(vec) >= 0); }

  /// Perform an element-wise comparison for less-than
  int operator<(const BitArrayBase<k,T,P>& vec) const
    { return (compare(vec) < 0); }

  /// Perform an element-wise comparison for less-than-or-equal-to
  int operator<=(const BitArrayBase<k,T,P>& vec) const
    { return (compare(vec) <= 0); }

  /// I/O methods:

  /**
   * Write the array to an output stream.
   * The default format for this output is
   * \verbatim <len> : <bit-1><bit-2>...<bit-n> \endverbatim
   */
  virtual int write(std::ostream& output) const;

  /// Read the array from an input stream.
  virtual int read(std::istream& input);

  /// Pack the array into a PackBuffer class.
  int write(PackBuffer& output) const;

  /// Unpack the array from an UnPackBuffer class.
  int read(UnPackBuffer& input);	

  /// Little method to write out characters.  Can be overridden.
  virtual char translate_to_char(const int digit) const;

  /// Translate a character into an element of the array.  
  /// Can be overridden.
  virtual int translate_from_char(const char c) const;
  
  // These define the basic bit-oriented constants.

  enum { bits          = 1 << k                 };
  enum { index_shift   = LOG2BITWORDSIZE - k    };
  enum { bits_per_word = 1 << LOG2BITWORDSIZE   };
  enum { index_mask    = (1 << index_shift) - 1 };
  enum { data_mask     = (1 << bits) - 1        };
  enum { elts_per_word = bits_per_word/bits     };

protected:

  ///
  void construct(const size_type             mylen,
                         bitword*            d=0,
                         const EnumDataOwned o=DataNotOwned)
	{ArrayBase<bitword,P>::construct(mylen,d,o); }

  /// Initializes an array, or the end of one, to zero
  void initialize(bitword* data,
		  const size_type old_len,
		  const size_type new_len);

  /// To dump data for debugging
  void dump_data(std::ostream& str, unsigned int max_elements=0);

  /// The BitArrayBase serializer / deserializer 
  static int serializer( SerialObject::elementList_t& serial, 
                         Any& data, bool serialize );
};


///
/// INLINED METHODS
///

template <int k, class T, class P>
inline T BitArrayBase<k,T,P>::get(const size_type ndx) const
{
#if (ArraySanityChecking==1)
  if (ndx >= Len)
     EXCEPTION_MNGR(std::runtime_error, "BitArrayBase::operator[] : iterator out of range. ndx=" << ndx << " len=" << Len);
#endif
  size_type shift = (ndx & index_mask) << k;
#ifdef UTILIB_AIX_CC
  return (T) ((Data[ndx >> (LOG2BITWORDSIZE - k)] >> shift) & data_mask);
#else
  return (T) ((Data[ndx >> index_shift] >> shift) & data_mask);
#endif
}


template <int k, class T, class P>
inline void BitArrayBase<k,T,P>::put(const size_type ndx, const T val)
{
#if (ArraySanityChecking==1)
  if ((size_type)ndx >= Len)
     EXCEPTION_MNGR(std::runtime_error, "BitArrayBase::put : iterator out of range. ndx=" << ndx << " len=" << Len);
  if ((val < 0) || (val > static_cast<int>(data_mask)))
     EXCEPTION_MNGR(std::runtime_error, "BitArrayBase::put : value out of range. val=" << val << " ndx=" << ndx );
#endif
#ifdef UTILIB_AIX_CC
  size_type i = ndx >> (LOG2BITWORDSIZE - k);
#else
  size_type i = ndx >> index_shift;
#endif
  size_type shift = (ndx & index_mask) << k;
  Data[i] = static_cast<utilib::bitword>
     ( (Data[i] & ~(data_mask << shift)) | (val << shift) );
}


///
/// METHOD definitions
///

template<int k, class T, class P> 
void BitArrayBase<k,T,P>::initialize(bitword* data,
				     const size_type old_len,
				     const size_type new_len)
{
  // Zap any bits that were hanging on the end of the old version.

  size_type cursor   = alloc_size(old_len);
  size_type leftover = (old_len & index_mask);
  if (leftover > 0)
    {
      workingBitData mask = (1 << (leftover << k)) - 1;
      data[cursor - 1] &= mask;
    }

  // Now zap all remaining words.

  size_type nwords = alloc_size(new_len);
  for(; cursor < nwords; cursor++)
    data[cursor] = 0;
}


template<int k, class T, class P>
int BitArrayBase<k,T,P>::operator==(const BitArrayBase<k,T,P>& vec) const
{
  // Check for trivial differences

  if (vec.Data == Data)
    return 1;
  if (vec.Len != Len)
    return 0;

  // Check everything except the last bitword

  size_type numFullWords = Len >> index_shift;
  size_type i = 0;
  for (; i<numFullWords; i++)
    if (Data[i] != vec.Data[i])
      return 0;

  // Check the last bitword, if any

  size_type leftover = (Len & index_mask);
  if (leftover == 0)
    return 1;
  workingBitData mask = (1 << (leftover << k)) - 1;
  return (Data[i] & mask) == (vec.Data[i] & mask);
}


template<int k, class T, class P>
int BitArrayBase<k,T,P>::compare(const BitArrayBase<k,T,P>& vec) const
{
  if (vec.Data != Data)     // If there's a chance the data differs...
    {
      // Check whether leading data elements are the same

      bitword* mypointer  = Data;
      bitword* vecpointer = vec.Data;
  
      workingBitData mydata  = 0;
      workingBitData vecdata = 0;

      for (size_type i=0; (i < Len) && (i < vec.Len); i++)
	{
	  if ((i & index_mask) == 0)
	    {
	      mydata  = *(mypointer++);
	      vecdata = *(vecpointer++);
	    }
	  else
	    {
	      mydata  >>= bits;
	      vecdata >>= bits;
	    }
	  int diff = (int)(mydata & data_mask) - (int)(vecdata & data_mask);

	  if (diff > 0)         // Return immediately if there's a
	    return 1;           // a difference.
	  else if (diff < 0)
	    return -1;
	}
    }

  // If leading data match, then the longer string is greater.
  // If lengths match, they are "equal"

  return sgn((int) Len - (int) vec.Len);
}

// CAP: changed this to be compatible with the format of write.  Originally
// got size informtion from the BitArray receiving the data

template<int k, class T, class P>
int BitArrayBase<k,T,P>::read(std::istream& s)
{
  size_type i, reportedLen;
  char c;

  s >> reportedLen;
  if (reportedLen != Len)
    EXCEPTION_MNGR(std::runtime_error, "BitArray::read: size mismatch: " << Len << " (current) vs " << reportedLen << "(new)");

  s >> c;
  if (c != ':')
    EXCEPTION_MNGR(std::runtime_error, "BitArray::read: format error.  Expected : after length ");

  for (i=0; i<Len; i++) 
    {
      s >> c;
      while (s && ((c == ' ') || (c == '\t') || (c == '\n')))
	s >> c;
      put(i,(T) translate_from_char(c));
    }

  return OK ;
}


// Default mapping is by hex digits

template<int k, class T, class P>
int BitArrayBase<k,T,P>::translate_from_char(const char c) const
{
  if ((c >= '0') && (c <= '9'))
    {
      int val = c - '0';
      if (val <= data_mask)
	return val;
    }
  else if (data_mask > 9)
    {
      if ((c >= 'A') && (c <= 'A' + data_mask - 10))
	return c - 'A' + 10;
      else if ((c >= 'a') && (c <= 'a' + data_mask - 10))
	return c - 'a' + 10;
    }
  EXCEPTION_MNGR(std::runtime_error, "BitArray::read : Bad input value: \'" << c << "\'");
  return 0;  // Never happens
}


template<int k, class T, class P>
int BitArrayBase<k,T,P>::write(std::ostream& output) const
{
  output << Len << ": ";
  if (Data) 
    {
      bitword* pointer = Data;
      workingBitData temp = 0;
      for (size_type i=0; i<Len; i++)
	{
	  if ((i & index_mask) == 0)
	    temp = *(pointer++);
	  else
	    temp >>= bits;
	  output << translate_to_char(temp & data_mask);
	}
    }
  return OK ;
}


template<int k, class T, class P>
char BitArrayBase<k,T,P>::translate_to_char(const int digit) const
{
  if (digit < 10)
    return '0' + digit;
  else
    return 'a' - 10 + digit;
}


template<int k, class T, class P>
int BitArrayBase<k,T,P>::write(PackBuffer& output) const
{
  if (Data) 
    {
      output << Len;
      output.pack(Data,alloc_size(Len));
    }
  else
    output << (size_t) 0;
  return OK ;
}


template<int k, class T, class P>
int BitArrayBase<k,T,P>::read(UnPackBuffer& input)
{
  size_t read_len;
  input >> read_len;
  base_t::resize(read_len, 0);    /* The zero means don't set new contents */
  input.unpack(Data, alloc_size(read_len));
  return OK ;
}


template<int k, class T, class P>
void BitArrayBase<k,T,P>::dump_data(std::ostream& str, unsigned int max_elements)
{
  if (Data) 
    {
      bitword* pointer = Data;
      workingBitData temp = 0;
      for (size_type i=0; i<Len && i<max_elements; i++)
	{
	  if ((i & index_mask) == 0)
	    temp = *(pointer++);
	  else
	    temp >>= bits;
	  str << translate_to_char(temp & data_mask);
	}
      str << std::endl;
    }
}


/** Serialize a BitArrayBase object.  Note: as no one will ever create a
 *  BitArrayBase object, we will not bother to register this with the
 *  Serializer().
 */
template<int k, class T, class P>
int BitArrayBase<k,T,P>::serializer( SerialObject::elementList_t& serial, 
                                     Any& data, bool serialize )
{
   // NB: We intentionally do NOT serialize the handle.  I think it is
   // roughly equivalent to an iterator - so it can be (safely)
   // invalidated by deserializing data into the BitArrayBase.

   // The following is a trick to cast the incoming Any into the base
   // class (needed until Anys support polymorphism). 
   Any tmp;
   tmp.set<ArrayBase<bitword,P> >
      ( data.template expose<BitArrayBase<k,T,P> >(), true );
   // NB: directly calling the base class serializer ONLY works
   // because this derived class does not add any additional data.
   // That said, doing this saves a level of indirection in the
   // actual serial stream.
   return ArrayBase<bitword,P>::serializer(serial, tmp, serialize); 
}



} // namespace utilib

/*
** TODO: find/write a test for this
*/
#ifndef NO_NONTYPE_TEMPLATE_ARGUMENTS_FUNCTIONS
/// utilib::BitArrayBaseHandle out-stream operator
template<int k, class T, class P>
inline
std::ostream& operator<<(std::ostream& output, const utilib::BitArrayBaseHandle<k,T,P>& array)
{ output << array.value(); return(output); }

 
/// utilib::BitArrayBaseHandle in-stream operator
template<int k, class T, class P>
inline
std::istream& operator>>(std::istream& input, utilib::BitArrayBaseHandle<k,T,P>& array)
{
T tmp;
input >> tmp;
array = tmp;
return(input);
}


/// utilib::BitArrayBaseHandle PackBuffer operator
template<int k, class T, class P>
inline
utilib::PackBuffer& operator<<(utilib::PackBuffer& output, const utilib::BitArrayBaseHandle<k,T,P>& array)
{ output << array.value(); return(output); }
 

/// utilib::BitArrayBaseHandle UnPackBuffer operator
template<int k, class T, class P>
inline
utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& input, utilib::BitArrayBaseHandle<k,T,P>& array)
{
T tmp;
input >> tmp;
array = tmp;
return(input);
}


/// utilib::BitArrayBase out-stream operator
template<int k, class T, class P>
inline
std::ostream& operator<<(std::ostream& output, const utilib::BitArrayBase<k,T,P>& array)
{ array.write(output); return(output); }

 
/// utilib::BitArrayBase in-stream operator
template<int k, class T, class P>
inline
std::istream& operator>>(std::istream& input, utilib::BitArrayBase<k,T,P>& array)
{ array.read(input); return(input); }


/// utilib::BitArrayBase PackBuffer operator
template<int k, class T, class P>
inline
utilib::PackBuffer& operator<<(utilib::PackBuffer& output, const utilib::BitArrayBase<k,T,P>& array)
{ array.write(output); return(output); }
 

/// utilib::BitArrayBase UnPackBuffer operator
template<int k, class T, class P>
inline
utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& input, utilib::BitArrayBase<k,T,P>& array)
{ array.read(input); return(input); }


/// utilib::BitArrayBase copy stream operator (utilib::BitArrayBase)
template<int k, class T, class P>
utilib::BitArrayBase<k,T,P>& 
operator<<(utilib::BitArrayBase<k,T,P>& x, const utilib::BitArrayBase<k,T,P>& vec)
{
  if (x.size() != vec.size())
     EXCEPTION_MNGR(std::runtime_error, "BitArray operator<< : Unequal array lengths " << x.size() << " and " << vec.size());
  if (x.data() == vec.data())
    return x;
  size_type nwords = x.alloc_size(x.size());
  for(unsigned int i=0; i<nwords; i++)
    x.data()[i] = vec.data()[i];
  return x;
}


/// utilib::BitArrayBase copy stream operator (value)
template<int k, class T, class P>
utilib::BitArrayBase<k,T,P>& 
operator<<(utilib::BitArrayBase<k,T,P>& x, const T& value)
{
#if (ArraySanityChecking==1)
if ((value < 0) || ((static_cast<int>(value)) > (static_cast<int>(x.data_mask))))
   EXCEPTION_MNGR(std::runtime_error, "operator<<(const T value) : value " << value << " out of range for " << static_cast<int>(x.bits) << "-bit array");
#endif
utilib::workingBitData pattern = 0;
size_type i = 0;
for(; i<x.elts_per_word; i++) {
  pattern <<= x.bits;
  pattern |=  value;
  }
size_type nwords = x.alloc_size(x.size());
for (size_type i=0; i<nwords; i++)
  x.data()[i] = pattern;
return x;
}

#endif

#endif
