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
 * \file EnumBitArray.h
 *
 * Defines the utilib::EnumBitArray class
 */

#ifndef utilib_EnumBitArray
#define utilib_EnumBitArray

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/BitArrayBase.h>

namespace utilib {

/**
 * An array of 2^k-bit values that uses an enumerated datatype. 
 * The main elements of this array have the same look and feel as a 
 * SimpleArray object.  However, this class uses a special, compact 
 * representation of the data elements, which forces it to be a separate class.
 * This class does not impose a restriction on the maximum length of the array.
 *
 * Note: this class assumes that
 *	1. the user initializes the data structures necessary for interpreting 
 *         the enum.  The class takes ownership of the arrays passed in during
 *         this initialization.
 *	2. the enum type is set to integral values, starting from 0
 */
template <int k, typename T>
class EnumBitArray : public BitArrayBase< k, T, EnumBitArray<k,T> >
{
public:

  /// Empty constructor.
  EnumBitArray<k,T>() : 
	BitArrayBase< k, T, EnumBitArray<k,T> >()
	{ }
  
  /// Explicit constructor: construct with data provided by the \a d array.
  explicit
  EnumBitArray<k,T>(const size_type len, 
		    bitword* d=0, 
		    const EnumDataOwned o=DataNotOwned) :
	BitArrayBase< k, T, EnumBitArray<k,T> >(len,d,o)
	{ }
  
  /// Copy constructor.
  EnumBitArray<k,T>(const EnumBitArray<k,T>& array) :
	BitArrayBase< k, T, EnumBitArray<k,T> >(array.Len,array.Data,DataOwned)
	{ }

  /// Disconnect from any prior data and copy the data from object \b array
  EnumBitArray<k,T>& operator=(const EnumBitArray<k,T>& array)
        {
        this->BitArrayBase<k,T,EnumBitArray<k,T> >::operator=(array);
        return *this;
        }

  /// Translate values to characters for output
  char translate_to_char(const int digit) const;

  /// Translate characters to values for input
  int translate_from_char(const char c) const;

  /// Set the static variables.  This has to be done before using these arrays.
  /// This allows us to avoid static initialization, which has a lot of
  /// portability problems.  These arrays must not be deleted till the end
  /// of all computation.  This class does not delete them.
  void setEnumInformation(int new_count, char *new_labels, T* new_values);

protected:

  /// How many values...
  static int enum_count;

  /// The (single) character labels of the enumeration types.
   static const char* enum_labels;

  /// A list of the enumeration values.
   static T *enum_vals;

};

 template <int k, typename T>
   int EnumBitArray<k,T>::enum_count = 0;

 template <int k, typename T>
   const char* EnumBitArray<k,T>::enum_labels = "";

 template <int k, typename T>
   T *EnumBitArray<k,T>::enum_vals = NULL;

///
/// METHOD definitions
///

template<int k, typename T>
char EnumBitArray<k,T>::translate_to_char(const int digit) const
{
#ifdef UTILIB_VALIDATING
if (enum_count ==0)
   EXCEPTION_MNGR(runtime_error, "EnumBitArray::Static data for the class uninitialized.");
#endif
return enum_labels[digit];
}


template<int k, typename T>
int EnumBitArray<k,T>::translate_from_char(const char c) const
{
#ifdef UTILIB_VALIDATING
if (enum_count ==0)
   EXCEPTION_MNGR(runtime_error, "EnumBitArray::Static data for the class uninitialized.");
#endif
int j = 0;
for (; j<enum_count; j++)
  if (c == enum_labels[j])
     return enum_vals[j];

   EXCEPTION_MNGR(runtime_error, "EnumBitArray::translate_from_char : Bad input value: \'" << c << "\'");
return 0; // Never happens
}

template<int k, typename T>
void EnumBitArray<k,T>::setEnumInformation(int new_count, char *new_labels, T* new_values)
{
  enum_count = new_count;
  enum_labels = new_labels;
  enum_vals = new_values;
}

} // namespace utilib

#endif
