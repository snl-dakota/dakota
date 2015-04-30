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
 * \file Basic2DArray.h
 *
 * \deprecated definition of the utilib::Basic2DArray class
 */

#ifndef utilib_Basic2DArray_h
#define utilib_Basic2DArray_h

#include <utilib/std_headers.h>
#include <utilib/BasicArray.h>

namespace utilib {


/**
 * \def TwoDArraySanityChecking
 *
 * If the macro TwoDArraySanityChecking=1 then some routine sanity checks will 
 * be performed when accessing vectors.  Sanity checking generally degrades 
 * performance.  The default is is to perform sanity checking.
 *
 * Note: Because inlines are typically put "in place" when the optimization flag
 * -O is used, you can define some files with TwoDArraySanityChecking=0 to 
 * acheive better performance, while using the default debugging mode in the
 * rest of the code.
 */
#if !defined(TwoDArraySanityChecking)
#define TwoDArraySanityChecking    1
#endif

template <class T>
class Basic2DArray;

template <class T>
class Simple2DArray;

template <class T>
class Num2DArray;


/**
 * The internal class that is used to manage the data for
 * Basic2DArray's and their derived classes.
 */
template <class T>
class Basic2DArrayRep {
 
  friend class Basic2DArray<T>;
  friend class Simple2DArray<T>;
  friend class Num2DArray<T>;

protected:
 
	/// Empty constructor.
  Basic2DArrayRep() {ref=1; Nrows=0; Ncols=0; Data=0;}
	/// The pointer to the data.
  T** Data; 
	/// Number of 2D arrays that are referencing this \b Basic3DArrayRep instance
  int ref;
	/// Number of rows.
  size_type Nrows;
	/// Number of columns.
  size_type Ncols;
	/// The ownership category for the main data array.
  EnumDataOwned own_data;
	/// The ownership category for the data that the main array points to.
  EnumDataOwned own_ptrs;

};


/**
 * A 2D array data type that manages the allocation and
 * deallocation of memory.  Note that 2DArrays are different from Arrays
 * of Arrays, though they are quite similar.  This class is similar to a
 * dense matrix class, and it should probably be replaced with a real dense
 * matrix facility.
 */
template <class T>
class Basic2DArray : public BasicArrayBounds<T>
{
public:

	/// Null constructor.
  Basic2DArray()
		{construct(0,0,(T*)0);}
	/**
	 * Constructor that initializes the 2D array using a given array.
	 * The 2D array only constructs the array of pointers, which point
	 * into the data owned by the \c array object.
	 */
  Basic2DArray(const BasicArray<T>& array, const size_type nrows=1, 
		const EnumDataOwned own=DataNotOwned)
		{construct(nrows,array.size()/nrows,array,own);}
	/**
	 * Constructor that initializes the 2D array using a given array.
	 * The 2D array only constructs the array of pointers, which point
	 * into the data owned by the \c d object.
	 */
  Basic2DArray(const size_type nrows, const size_type ncols, T *d=((T*)0), 
		const EnumDataOwned own=DataNotOwned)
		{construct(nrows,ncols,d,own);}
	/**
	 * Constructor that initializes the 2D array using a given array.
	 * The 2D array only constructs the array of pointers, which point
	 * into the data owned by the \c array object.
	 */
  Basic2DArray(const size_type nrows, const size_type ncols, 
		const BasicArray<T>& array, 
		const EnumDataOwned own=DataNotOwned)
		{construct(nrows,ncols,array,own);}
	/// Copy constructor.
  Basic2DArray(const Basic2DArray& array)
		{construct(array.a->Nrows,array.a->Ncols,array.a->Data,
				DataOwned);}

	/// Destructor.
  virtual ~Basic2DArray()
	{free();}

	/// Resize the 2D array.
  int resize(const size_type nrows, const size_type ncols);
	/// Returns the number of rows in the 2D array
  size_type nrows() const
	{return a->Nrows;}
	/// Returns the number of columns in the 2D array
  size_type ncols() const
	{return a->Ncols;}
#if 0
	/// Coersion operator that returns the internal array of pointers.
  operator T** () const
	{return a->Data;}
#endif
	/// Returns the internal array of pointers.
  T** data() const
	{return a->Data;}
	/// Returns the number of references to the internal array.
  int nrefs() const
	{return a->ref;}
	/// Returns the i-th row of the 2D array
  T* operator[](size_type i) const;
	/// Returns the i-th row of the 2D array
  //const T* operator[](size_type i) const;
	/// Returns the element from the row-th row and col-th column
  T& operator()(const size_type row, const size_type col);
	/// Returns the element from the row-th row and col-th column
  const T& operator()(const size_type row, const size_type col) const;

	/// Copies the \b array object by constructing a new \b Basic2DArray
  Basic2DArray<T>& operator=(const Basic2DArray<T>& array);
	/// Copies the pointer from the \b array object
  Basic2DArray<T>& operator&=(const Basic2DArray<T>& array);

	/**
	 * Method to explicitly set the internal data array.
	 * The current array is resized to length \a len, and pointers in the
	 * array are setup to point into the \a data array.
	 */
  Basic2DArray<T>& set_data(const size_type len, T* data,
                                const EnumDataOwned o=DataNotOwned);
	/// Method to explicitly set the internal data array using a \a BasicArray object.
  Basic2DArray<T>& set_data(const BasicArray<T>& array,
                                const EnumDataOwned o=DataNotOwned)
		{return set_data(array.size(),array,o);}

/* BUG??  Where are these defined?
  friend Basic2DArray<T> T(const BasicArray<T>& array);
	/// HUH???
  friend Basic2DArray<T> T(const Basic2DArray<T>& array);
*/

protected:

	/// Pointer to the 2D array representation.
  Basic2DArrayRep<T>* a;
	/// Constructor method that provides the array of data elements
  void construct(const size_type nrows, const size_type ncols, T *d, 
		const EnumDataOwned o=DataNotOwned);
	/// Constructor method that provides the complete 2D array
  void construct(const size_type nrows, const size_type ncols, T** d, 
		const EnumDataOwned o=DataNotOwned);
	/// Method used to free up the 2D array
  void free();

};


template <class T>
T* Basic2DArray<T>::operator[](size_type idx) const
{
#if (TwoDArraySanityChecking==1)
if (this->is_negative(idx) || ((size_type)idx >= a->Nrows))
   EXCEPTION_MNGR(runtime_error, "Basic2DArray<T>::operator[] : iterator out of range. idx=" << idx << " len=" << a->Nrows);
#endif
 
return a->Data[idx];
}
 
#if 0
template <class T>
inline const T* Basic2DArray<T>::operator[](size_type idx) const
{
#if (TwoDArraySanityChecking==1)
if ((idx < 0) || ((size_type)idx >= a->Nrows))
   EXCEPTION_MNGR(runtime_error, "Basic2DArray<T>::operator[] : iterator out of range. idx=" << idx << " len=" << a->Nrows);
#endif
 
return a->Data[idx];
}
#endif


template <class T>
inline T& Basic2DArray<T>::operator()(const size_type row, const size_type col)
{
#if (TwoDArraySanityChecking==1)
if ((row >= a->Nrows) || (col >= a->Ncols))
   EXCEPTION_MNGR(runtime_error, "Basic2DArray<T>::operator() : iterator out of range. row=" << row << " nrows=" << a->Nrows << " col=" << col << " ncols=" << a->Ncols);
#endif
 
return a->Data[row][col];
}
 
 
template <class T>
inline const T& Basic2DArray<T>::operator()(const size_type row, const size_type col) const
{
#if (TwoDArraySanityChecking==1)
if ((row >= a->Nrows) || (col >= a->Ncols))
   EXCEPTION_MNGR(runtime_error, "Basic2DArray<T>::operator() : iterator out of range. row=" << row << " nrows=" << a->Nrows << " col=" << col << " ncols=" << a->Ncols);
#endif
 
return a->Data[row][col];
}


template <class T>
void Basic2DArray<T>::construct(const size_type nrows, const size_type ncols, T * d, 
					const EnumDataOwned o)
{
a = new Basic2DArrayRep<T>;
assert(a != 0);

a->Nrows = nrows;
a->Ncols = ncols;
if (d == NULL) {
   if (nrows > 0) {
      a->Data = new T* [nrows] ;
      assert(a != 0);
      for (size_type i=0; i<nrows; i++) {
         a->Data[i] = new T [ncols] ;
         assert(a->Data[i] != 0);
         }
      }
   else
      a->Data = NULL;
   a->own_ptrs = DataOwned;
   a->own_data = DataOwned;
   }

else {
   if (o == DataOwned) {
      if (nrows > 0)  {
         a->Data = new T* [nrows] ;
	 assert(a->Data != 0);
         for (size_type i=0; i<nrows; i++) {
            a->Data[i] = new T [ncols] ;
            assert(a->Data[i] != 0);
            }
	 size_type ndx=0;
         for (size_type ii=0; ii<nrows; ii++)
           for (size_type jj=0; jj<ncols; jj++)
             a->Data[ii][jj] = d[ndx++];
         }
      else
         a->Data = NULL;
      a->own_ptrs = DataOwned;
      a->own_data = DataOwned;
      }
   else {
      if (nrows > 0)  {
         a->Data = new T* [nrows] ;
	 assert(a->Data != 0);
         T* tmp = d;
         for (size_type i=0; i<ncols; i++) {
            a->Data[i] = tmp;
            tmp+= ncols;
            }
         }
      else
         a->Data = NULL;
      a->own_ptrs = DataOwned;
      a->own_data = DataNotOwned;
      }
   }
}


template <class T>
void Basic2DArray<T>::construct(const size_type nrows, const size_type ncols, T **d, 
					const EnumDataOwned o)
{
a = new Basic2DArrayRep<T>;
assert(a != 0);

a->Nrows = nrows;
a->Ncols = ncols;
if (d == NULL) {
   if (nrows > 0) {
      a->Data = new T* [nrows] ;
      assert(a != 0);
      for (size_type i=0; i<nrows; i++) {
         a->Data[i] = new T [ncols] ;
         assert(a->Data[i] != 0);
         }
      }
   else
      a->Data = NULL;
   a->own_ptrs = DataOwned;
   a->own_data = DataOwned;
   }

else {
   if (o == DataOwned) {
      if (nrows > 0)  {
         a->Data = new T* [nrows] ;
	 assert(a->Data != 0);
         for (size_type i=0; i<nrows; i++) {
            a->Data[i] = new T [ncols] ;
            assert(a->Data[i] != 0);
            }
         for (size_type ii=0; ii<nrows; ii++)
           for (size_type jj=0; jj<ncols; jj++)
             a->Data[ii][jj] = d[ii][jj];
         }
      else
         a->Data = NULL;
      a->own_ptrs = DataOwned;
      a->own_data = DataOwned;
      }
   else {
      a->Data=d;
      a->own_ptrs = DataNotOwned;
      a->own_data = DataNotOwned;
      }
   }
}


template <class T>
void Basic2DArray<T>::free()
{
if (--a->ref == 0) {
   if ((a->Data) && (a->Nrows>0)) {
      if (a->own_data && (a->Ncols > 0)) {
         for (size_type i=0; i<a->Nrows; i++)
           delete [] a->Data[i];
         }
      if (a->own_ptrs)
         delete [] a->Data;
      }
   delete a;
   }
}


template <class T>
int Basic2DArray<T>::resize(const size_type nrows, const size_type ncols)
{
//
// Maybe we get lucky.
//
if ((ncols == a->Ncols) && (nrows == a->Nrows))
   return OK;

//
// Need to completely replace old data.
//
if ((ncols != a->Ncols) && (nrows != a->Nrows)) {
   //
   // Delete old data (if owned)
   //
   if ((a->Data) && (a->Nrows>0)) {
      if (a->own_data && (a->Ncols > 0)) {
         for (size_type i=0; i<a->Nrows; i++)
           delete [] a->Data[i];
         }
      if (a->own_ptrs)
         delete [] a->Data;
      }
   //
   // Make new data
   //
   if (nrows > 0) {
      a->Data = new T* [nrows] ;
      assert(a != 0);
      for (size_type i=0; i<nrows; i++) {
         a->Data[i] = new T [ncols] ;
         assert(a->Data[i] != 0);
         }
      }
   else
      a->Data = NULL;
   a->own_ptrs = DataOwned;
   a->own_data = DataOwned;
   a->Nrows = nrows;
   a->Ncols = ncols;
   return OK;
   }

//
// We need to simply resize the number of rows.
//
if (nrows != a->Nrows) {
   T **d=NULL;
   if (nrows > 0) {
      d = new T* [nrows];
      assert(d != 0);
      // 
      // Note:  the new memory is not initialized beyond what the constructors
      // do.
      //
      if (a->Nrows > 0) {
         for (size_type i=0; i<std::min(nrows,a->Nrows); i++)
           d[i] = a->Data[i];
         }
      }
 
   if (a->Data && a->own_ptrs && (a->Nrows > 0))
      delete [] a->Data;
 
   a->Data = d;
   a->Nrows = nrows;
   }

//
// We need to simply resize the number of columns
//
if (ncols != a->Ncols) {
   if (ncols > 0) {
      for (size_type j=0; j<a->Nrows; j++) {
        T* d=NULL;
        d = new T [ncols];
        assert(d != 0);
        // 
        // Note:  the new memory is not initialized beyond what the constructors
        // do.
        //
        if (a->Ncols > 0) {
           for (size_type i=0; i<std::min(ncols,a->Ncols); i++)
             d[i] = a->Data[j][i];
           }
        if (a->Data && a->own_data && (a->Ncols > 0))
           delete [] a->Data[j];
        a->Data[j] = d;
        }
      }
   else 
      if (a->Data && a->own_data && (a->Ncols > 0))
         for (size_type i=0; i<a->Nrows; i++) {
           delete [] a->Data[i];
           a->Data[i] = NULL;
           }
 
   a->Ncols = ncols;
   }

return OK;
}


template <class T>
Basic2DArray<T>& Basic2DArray<T>::operator=(const Basic2DArray<T>& array)
{
if (this != &array) {
   free();
   construct(array.nrows(),array.ncols(),array.data(),DataOwned);
   }
return *this;
}



template <class T>
Basic2DArray<T>& Basic2DArray<T>::operator&=(const Basic2DArray<T>& array)
{
if (array.a == a) return *this;

free();
a = array.a;
a->ref++;
return *this;
}



template <class T>
Basic2DArray<T>& Basic2DArray<T>::set_data(const size_type len, T * data, const EnumDataOwned o)
{
#if TwoDArraySanityChecking==1
if ( (nrows()*ncols()) != len)
   EXCEPTION_MNGR(runtime_error, "Basic2DArray<T>::set_data : bad vector size "
                  << (nrows()*ncols()) << " != " << len);
#endif

if (a->own_data && (a->Ncols > 0)) {
   for (size_type i=0; i<a->Nrows; i++)
     delete [] a->Data[i];
   }
size_type ndx=0;
for (size_type i=0; i<a->Nrows; i++) {
  a->Data[i] = &(data[ndx]);
  ndx += a->Ncols;
  }

//
// BUG: I need to fix the model of ownership here.
//
a->own_data = DataNotOwned;
return *this;
}

} // namespace utilib


/// Stream copy operator
template <class T>
utilib::Basic2DArray<T>& operator<<(utilib::Basic2DArray<T>& x, const utilib::Basic2DArray<T>& array)
{
if (array.data() == x.data())
   return x;

if ((array.nrows() != x.nrows()) || (array.ncols() != x.ncols()))
   EXCEPTION_MNGR(std::runtime_error, "Basic2DArray<T>::operator<< : Unequal vector matrices " << x.nrows() << "x" << x.ncols() << " and " << array.nrows() << "x" << array.ncols());
 
for (size_type i=0; i<x.nrows(); i++)
  for (size_type j=0; j<x.ncols(); j++)
    x[i][j] = array[i][j];
 
return x;
}


/// Stream operator to fill a 2D array with a value
template <class T>
inline utilib::Basic2DArray<T>& operator<<(utilib::Basic2DArray<T>& x, const T& val)
{
for (size_type i=0; i<x.nrows(); i++)
  for (size_type j=0; j<x.ncols(); j++)
    x[i][j] = val;
return x;
}


/// to fill a 2D array with an array
template <class T>
inline utilib::Basic2DArray<T>& operator<<(utilib::Basic2DArray<T>& x, const utilib::BasicArray<T>& val)
{
if (x.ncols() != val.size())
   EXCEPTION_MNGR(std::runtime_error, "operator<< : Array length does not equal the 2DArray columns " << val.size() << " and " << x.ncols());

for (size_type i=0; i<x.nrows(); i++)
  for (size_type j=0; j<x.ncols(); j++)
    x[i][j] = val[j];
return x;
}


#endif
