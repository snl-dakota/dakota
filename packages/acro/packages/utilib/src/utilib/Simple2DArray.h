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
 * \file Simple2DArray.h
 *
 * \deprecated definition of utilib::Simple2DArray class
 */

#ifndef utilib_Simple2DArray_h
#define utilib_Simple2DArray_h

#include <utilib/std_headers.h>
#include <utilib/_generic.h>
#include <utilib/Basic2DArray.h>
#include <utilib/PackBuf.h>

namespace utilib {

/**
 * A 2D array data type that manages the allocation and deallocation of memory. 
 * The Simple2DArray class extends the Basic2DArray class to include
 * I/O operations and comparison methods.
 */
template <class T>
class Simple2DArray : public Basic2DArray<T>
{
public:

  #if !defined(DOXYGEN)
  typedef Basic2DArray<T> base_t;
  #if !defined(UTILIB_SOLARIS_CC)
  using base_t::a;
  #endif
  #endif

  /// Null constructor.
  Simple2DArray()  
		: Basic2DArray<T>() {}

  /**
   * Constructor that initializes the 2D array using a given array.
   * The 2D array only constructs the array of pointers, which point
   * into the data owned by the \c array object.
   */
  Simple2DArray(const BasicArray<T>& array, const size_type nrows=1,
                const EnumDataOwned own=DataNotOwned)
                : Basic2DArray<T>(array,nrows,own) {}

  /**
   * Constructor that initializes the 2D array using a given array.
   * The 2D array only constructs the array of pointers, which point
   * into the data owned by the \c d object.
   */
  Simple2DArray(const size_type nrows, const size_type ncols, T *d=((T*)0),
                const EnumDataOwned own=DataNotOwned)
                : Basic2DArray<T>(nrows,ncols,d,own) {}

  /**
   * Constructor that initializes the 2D array using a given array.
   * The 2D array only constructs the array of pointers, which point
   * into the data owned by the \c array object.
   */
  Simple2DArray(const size_type nrows, const size_type ncols, const BasicArray<T>& array,
                const EnumDataOwned own=DataNotOwned)
                : Basic2DArray<T>(nrows,ncols,array,own) {}

  /// Copy constructor.
  Simple2DArray(const Simple2DArray<T>& array)
                : Basic2DArray<T>(array) {}

  ///
  virtual ~Simple2DArray() {}
 
  /// Copies the \b array object by constructing a new \b Simple2DArray
  Simple2DArray<T>& operator=(const Simple2DArray<T>& array)
		{Basic2DArray<T>::operator=(array); return *this;}

  /// Checks to see if the current array equals \b array.
  bool  operator== (const Simple2DArray<T>& array) const;

  /// Checks to see if the current array does not equal \b array.
  bool  operator!= (const Simple2DArray<T>& array) const;

  /**
   * Compares the current array with \b array.
   * If the current array is longer, then returns 1. If shorter, then
   * returns -1.  Otherwise, does a pairwise comparison on elements of
   * both arrays.  If there exists a nonequal pair of elements, then
   * returns 1 if the current array is greater and -1 otherwise.
   */
  int  compare(const Simple2DArray<T>& array) const;

/* BUG!  Let's see where this is used!
  int  operator<(const Simple2DArray<T>& array) const
                {return compare(array);}
*/

  /**
   * Write the array to the output stream.
   * The format for this output is
   * \verbatim <nrows> <ncols> <elt-1,1> <elt-1,2> ... <elt-1,m> ... <elt-n,m> \endverbatim
   */
  virtual int write(std::ostream& output) const;       

  /// Read the array from the input stream
  virtual int read(std::istream& input);              

  /// Pack the array into a \b PackBuffer class
  virtual int write(PackBuffer& output) const;  

  /// Unpack the array from an \b UnPackBuffer class
  virtual int read(UnPackBuffer& input);       

};

 

template <class T>
int Simple2DArray<T>::compare(const Simple2DArray<T>& array) const
{
if (array.a == a)
   return 0;
 
for (size_type i=0; i<min(a->Nrows,array.a->Nrows); i++)
  for (size_type j=0; j<min(a->Ncols,array.a->Nrows); j++)
    if (a->Data[i][j] < array.a->Data[i][j])
       return -1;
    else if (a->Data[i][j] > array.a->Data[i][j])
       return 1;

if ((array.a->Nrows == a->Nrows) && (array.a->Ncols == a->Ncols))
   return 0;

if ((array.a->Nrows < a->Nrows) || (array.a->Nrows < a->Nrows))
   return -1;
return 1;
}


template <class T>
bool Simple2DArray<T>::operator==(const Simple2DArray<T>& array) const
{
if (array.a == a)
   return 1;
if ((array.a->Nrows != a->Nrows) || (array.a->Ncols != a->Ncols))
   return 0;
 
for (size_type i=0; i<a->Nrows; i++)
  for (size_type j=0; j<a->Ncols; j++)
    if (a->Data[i][j] != array.a->Data[i][j])
       return 0;
 
return 1;
}


template <class T>
bool Simple2DArray<T>::operator!=(const Simple2DArray<T>& array) const
{
if ((array.a->Nrows != a->Nrows) || (array.a->Ncols != a->Ncols))
   return 1;
 
for (size_type i=0; i<a->Nrows; i++)
  for (size_type j=0; j<a->Ncols; j++)
    if (a->Data[i][j] != array.a->Data[i][j])
       return 1;
 
return 0;
}


template <class T>
int Simple2DArray<T>::write(PackBuffer& os) const
{
os << a->Nrows << a->Ncols;
if (a->Data) {
   for (size_type i=0; i<a->Nrows; i++)
     for (size_type j=0; j<a->Ncols; j++)
       os << a->Data[i][j];
   }
return OK;
}


template <class T>
int Simple2DArray<T>::write(std::ostream& os) const
{
os << a->Nrows << " " << a->Ncols << ": ";
if (a->Data) {
   for (size_type i=0; i<a->Nrows; i++) {
     for (size_type j=0; j<a->Ncols; j++)
       os << a->Data[i][j] << " ";
     os << std::endl;
     }
   }
return OK;
}


template <class T>
int Simple2DArray<T>::read(UnPackBuffer& is)
{
size_type nrows=0;
size_type ncols=0;
is >> nrows >> ncols;
base_t::resize(nrows,ncols);
for (size_type i=0; i<a->Nrows; i++)
  for (size_type j=0; j<a->Ncols; j++)
    is >> a->Data[i][j];

return OK;
}



template <class T>
int Simple2DArray<T>::read(std::istream& /*is*/)
{
//Basic2DArray<T>::free();

return OK;
}

} // namespace utilib

/// Stream operator for writing
template <class T>
inline std::ostream& operator<<(std::ostream& output, const utilib::Simple2DArray<T>& array)
{ array.write(output); return(output); }
 
/// Stream operator for reading
template <class T>
inline std::istream& operator>>(std::istream& input, utilib::Simple2DArray<T>& array)
{ array.read(input); return(input); }
 
/// Stream operator for packing
template <class T>
inline utilib::PackBuffer& operator<<(utilib::PackBuffer& output, const utilib::Simple2DArray<T>& array)
{ array.write(output); return(output); }
 
/// Stream operator for unpacking
template <class T>
inline utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& input, utilib::Simple2DArray<T>& array)
{ array.read(input); return(input); }



#endif
