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
 * \file Basic3DArray.h
 *
 * \deprecated definition of the utilib::Basic3DArray class
 */

#ifndef utilib_Basic3DArray_h
#define utilib_Basic3DArray_h

#include <utilib/std_headers.h>
#include <utilib/_generic.h>
#include <utilib/BasicArray.h>

namespace utilib {


//
// If the macro ThreeDArraySanityChecking=1 then some routine sanity checks will 
// be performed when accessing vectors.  Sanity checking generally degrades 
// performance.  The default is is to perform sanity checking.
//
// Note: Because inlines are typically put "in place" when the optimization flag
// -O is used, you can define some files with ThreeDArraySanityChecking=0 to 
// acheive better performance, while using the default debugging mode in the
// rest of the code.
// 
#if !defined(DOXYGEN)
#if !defined(ThreeDArraySanityChecking)
#define ThreeDArraySanityChecking    1
#endif
#endif

template <class T>
class Basic3DArray;

template <class T>
class Simple3DArray;

template <class T>
class Num3DArray;

///
/// The internal class that is used to manage the data for
/// Basic3DArray's and their derived classes.
///
template <class T>
class Basic3DArrayRep {
 
  #if !defined(DOXYGEN)
  friend class Basic3DArray<T>;
  friend class Simple3DArray<T>;
  friend class Num3DArray<T>;
  #endif

protected:
 
  /// TODO
  Basic3DArrayRep() {ref=1; Nrows=0; Ncols=0; Data=0;}
  /// TODO
  T** Data; 
  /// TODO
  int ref;
  /// TODO
  size_type Nrows;
  /// TODO
  size_type Ncols;
  /// TODO
  EnumDataOwned own_data;
  /// TODO
  EnumDataOwned own_ptrs;

};

/**
 * Definitions for a basic 3D array data type that manages the allocation and
 * deallocation of memory.  Note that 3DArrays are different from Arrays
 * of 2D Arrays, though they are quite similar.
 *
 * \note This code is still under development!
 */
template <class T>
class Basic3DArray
{
public:

  /// TODO
  Basic3DArray()
		{construct(0,0,0,(T*)0);}
  /// TODO
  Basic3DArray(const BasicArray<T>& array, const size_type nrows=1, 
		const EnumDataOwned own=DataNotOwned)
		{construct(nrows,array.size()/nrows,array,own);}
  /// TODO
  Basic3DArray(const size_type nrows, const size_type ncols, const size_type ndeep, T *d=((T*)0), 
		const EnumDataOwned own=DataNotOwned)
		{construct(nrows,ncols,ndeep,d,own);}
  /// TODO
  Basic3DArray(const size_type nrows, const size_type ncols, const BasicArray<T>& array, 
		const EnumDataOwned own=DataNotOwned)
		{construct(nrows,ncols,array.size(),array.data(),own);}
  /// TODO
  Basic3DArray(const Basic3DArray& array)
		{construct(array.nrows(),array.ncols(),array.array.data(),
				DataOwned);}

  /// TODO
  virtual ~Basic3DArray()
	{free();}

  /// TODO
  int resize(const size_type nrows, const size_type ncols, const size_type ndeep);
  /// TODO
  size_type nrows() const
	{return a->Nrows;}
  /// TODO
  size_type ncols() const
	{return a->Ncols;}
  /// TODO
  operator T** () const
	{return a->Data;}
  /// TODO
  T** data() const
	{return a->Data;}
  /// TODO
  int nrefs() const
	{return a->ref;}
  /// TODO
  T* operator[](const size_type );
  /// TODO
  const T* operator[](const size_type ) const;
  /// TODO
  T& operator()(const size_type row, const size_type col);
  /// TODO
  const T& operator()(const size_type row, const size_type col) const;

  /// TODO
  Basic3DArray<T>& operator=(const Basic3DArray<T>& array); // construct & copy
  /// TODO
  Basic3DArray<T>& operator&=(const Basic3DArray<T>& array);// copy ptr
  /// TODO
  Basic3DArray<T>& operator<<(const Basic3DArray<T>& array);// copy 3Darray
  /// TODO
  Basic3DArray<T>& operator=(const T& val);		// 3Darray set to val
  /// TODO
  Basic3DArray<T>& operator=(const BasicArray<T>& val);	// rows set to val

  /// TODO
  Basic3DArray<T>& set_data(const size_type len, T* data,
                                const EnumDataOwned o=DataNotOwned);
  /// TODO
  Basic3DArray<T>& set_data(const BasicArray<T>& array,
                                const EnumDataOwned o=DataNotOwned)
		{return set_data(array.size(),array,o);}

/* BUG? Where are these defined?
  friend Basic3DArray<T> T(const BasicArray<T>& array);
  friend Basic3DArray<T> T(const Basic3DArray<T>& array);
*/

protected:

  /// TODO
  Basic3DArrayRep<T>* a;

  /// TODO
  void construct(const size_type nrows, const size_type ncols, const size_type ndeep, T *d, 
		const EnumDataOwned o=DataNotOwned);
  /// TODO
  void construct(const size_type nrows, const size_type ncols, const size_type ndeep, T** d, 
		const EnumDataOwned o=DataNotOwned);
  /// TODO
  void free();

};


template <class T>
inline T* Basic3DArray<T>::operator[](const size_type idx)
{
#if (ThreeDArraySanityChecking==1)
if ((idx < 0) || (idx >= a->Nrows))
   EXCEPTION_MNGR(runtime_error, "Basic3DArray<T>::operator[] : iterator out of range. idx=" << idx << " len=" << a->Nrows);
#endif
 
return a->Data[idx];
}
 
 
template <class T>
inline const T* Basic3DArray<T>::operator[](const size_type idx) const
{
#if (ThreeDArraySanityChecking==1)
if ((idx < 0) || (idx >= a->Nrows))
   EXCEPTION_MNGR(runtime_error, "Basic3DArray<T>::operator[] : iterator out of range. idx=" << idx << " len=" << a->Nrows);
#endif
 
return a->Data[idx];
}


template <class T>
inline T& Basic3DArray<T>::operator()(const size_type row, const size_type col)
{
#if (ThreeDArraySanityChecking==1)
if ((row < 0) || (row >= a->Nrows) || (col < 0) || (col >= a->Ncols))
   EXCEPTION_MNGR(runtime_error, "Basic3DArray<T>::operator() : iterator out of range. " << row << "x" << col << " not in " << a->Nrows << "x" << a->Ncols);
#endif
 
return a->Data[row][col];
}
 
 
template <class T>
inline const T& Basic3DArray<T>::operator()(const size_type row, const size_type col) const
{
#if (ThreeDArraySanityChecking==1)
if ((row < 0) || (row >= a->Nrows) || (col < 0) || (col >= a->Ncols))
   EXCEPTION_MNGR(runtime_error, "Basic3DArray<T>::operator() : iterator out of range. " << row << "x" << col << " not in " << a->Nrows << "x" << a->Ncols);
#endif
 
return a->Data[row][col];
}




//
// Basic3DArray
//


template <class T>
void Basic3DArray<T>::construct(const size_type nrows, const size_type ncols, 
			const size_type ndeep, T * d, const EnumDataOwned o)
{
a = new Basic3DArrayRep<T>;
assert(a != 0);

a->Nrows = nrows;
a->Ncols = ncols;
a->Ndeep = ndeep;
if (d == NULL) {
   if (nrows > 0) {
      a->Data = new T** [nrows] ;
      assert(a != 0);
      for (size_type i=0; i<nrows; i++) {
         a->Data[i] = new T* [ncols] ;
         for (size_type j=0; j<ncols; j++) {
           a->Data[i][j] = new T [ndeep] ;
           assert(a->Data[i][j] != 0);
           }
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
         a->Data = new T** [nrows] ;
         assert(a != 0);
         for (size_type i=0; i<nrows; i++) {
            a->Data[i] = new T* [ncols] ;
            for (size_type j=0; j<ncols; j++) {
              a->Data[i][j] = new T [ndeep] ;
              assert(a->Data[i][j] != 0);
              }
            assert(a->Data[i] != 0);
            }

	 size_type ndx=0;
         for (size_type ii=0; ii<nrows; ii++)
           for (size_type jj=0; jj<ncols; jj++)
             for (size_type kk=0; kk<ndeep; kk++)
               a->Data[ii][jj][kk] = d[ndx++];
         }
      else
         a->Data = NULL;
      a->own_ptrs = DataOwned;
      a->own_data = DataOwned;
      }
   else {
      if (nrows > 0)  {
         a->Data = new T** [nrows] ;
	 assert(a->Data != 0);
         for (size_type i=0; i<nrows; i++) {
            a->Data[i] = new T* [ncols] ;
            assert(a->Data[i] != 0);
            }
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
void Basic3DArray<T>::construct(const size_type nrows, const size_type ncols, const size_type ndeep, T **d, 
					const EnumDataOwned o)
{
a = new Basic3DArrayRep<T>;
assert(a != 0);

a->Nrows = nrows;
a->Ncols = ncols;
if (d == NULL) {
   if (nrows > 0) {
      a->Data = new T** [nrows] ;
      assert(a != 0);
      for (size_type i=0; i<nrows; i++) {
         a->Data[i] = new T* [ncols] ;
         for (size_type j=0; j<ncols; j++) {
           a->Data[i][j] = new T [ndeep] ;
           assert(a->Data[i][j] != 0);
           }
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
         a->Data = new T** [nrows] ;
	 assert(a->Data != 0);
         for (size_type i=0; i<nrows; i++) {
            a->Data[i] = new T* [ncols] ;
            for (size_type j=0; j<ncols; j++) {
              a->Data[i][j] = new T [ndeep] ;
              assert(a->Data[i][j] != 0);
              }
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
void Basic3DArray<T>::free()
{
if (--a->ref == 0) {
   if ((a->Data) && (a->Nrows>0)) {
      if (a->own_data && (a->Ncols > 0)) {
         for (size_type i=0; i<a->Nrows; i++) {
           for (size_type j=0; j<a->Ncols; j++)
             if (a->Ndeep > 0) delete [] a->Data[i][j];
           delete [] a->Data[i];
           }
         }
      if (a->own_ptrs)
         delete [] a->Data;
      }
   delete a;
   }
}


template <class T>
int Basic3DArray<T>::resize(const size_type nrows, const size_type ncols, const size_type ndeep)
{
//
// Maybe we get lucky.
//
if ((ncols == a->Ncols) && (nrows == a->Nrows) && (ndeep == a->Ndeep))
   return OK;

//
// Need to completely replace old data.
//
if ((ncols != a->Ncols) && (nrows != a->Nrows) && (ndeep != a->Ndeep)) {
   //
   // Delete old data (if owned)
   //
   if ((a->Data) && (a->Nrows>0)) {
      if (a->own_data && (a->Ncols > 0)) {
         for (size_type i=0; i<a->Nrows; i++) {
           for (size_type j=0; j<a->Ncols; j++)
             if (a->Ndeep > 0) delete [] a->Data[i][j];
           delete [] a->Data[i];
           }
         }
      if (a->own_ptrs)
         delete [] a->Data;
      }
   //
   // Make new data
   //
   if (nrows > 0) {
      a->Data = new T** [nrows] ;
      assert(a != 0);
      for (size_type i=0; i<nrows; i++) {
         a->Data[i] = new T* [ncols] ;
         for (size_type j=0; j<ncols; j++) {
           a->Data[i][j] = new T [ndeep] ;
           assert(a->Data[i][j] != 0);
           }
         assert(a->Data[i] != 0);
         }
      }
   else
      a->Data = NULL;
   a->own_ptrs = DataOwned;
   a->own_data = DataOwned;
   a->Nrows = nrows;
   a->Ncols = ncols;
   a->Ndeep = ndeep;
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
         for (size_type i=0; i<MIN(nrows,a->Nrows); i++)
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
           for (size_type i=0; i<MIN(ncols,a->Ncols); i++)
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
Basic3DArray<T>& Basic3DArray<T>::operator=(const Basic3DArray<T>& array)
{
if (this != &array) {
   free();
   construct(array.nrows(),array.ncols(),array.data(),DataOwned);
   }
return *this;
}


template <class T>
Basic3DArray<T>& Basic3DArray<T>::operator<<(const Basic3DArray<T>& array)
{
if (array.a == a)
   return *this;
if ((array.a->Nrows != a->Nrows) || (array.a->Ncols != a->Ncols))
   EXCEPTION_MNGR(runtime_error, "Basic3DArray<T>::operator<< : Unequal vector matrices " << a->Nrows << "x" << a->Ncols << " and " << array.a->Nrows << "x" << array.a->Ncols);
 
for (size_type i=0; i<a->Nrows; i++)
  for (size_type j=0; j<a->Ncols; j++)
    a->Data[i][j] = array.a->Data[i][j];
 
return *this;
}



template <class T>
Basic3DArray<T>& Basic3DArray<T>::operator&=(const Basic3DArray<T>& array)
{
if (array.a == a) return *this;

free();
a = array.a;
a->ref++;
return *this;
}



template <class T>
Basic3DArray<T>& Basic3DArray<T>::operator=(const T& val)
{
for (size_type i=0; i<a->Nrows; i++)
  for (size_type j=0; j<a->Ncols; j++)
    a->Data[i][j] = ( T ) val;
return *this;
}



template <class T>
Basic3DArray<T>& Basic3DArray<T>::operator=(const BasicArray<T>& val)
{
if (a->Ncols != val.size())
   EXCEPTION_MNGR(runtime_error, "Basic3DArray<T>::operator= : Array length does not equal the 3DArray columns " << val.size() << " and " << a->Ncols);

for (size_type i=0; i<a->Nrows; i++)
  for (size_type j=0; j<a->Ncols; j++)
    a->Data[i][j] = val[j];
return *this;
}

template <class T>
Basic3DArray<T>& Basic3DArray<T>::set_data(const size_type len, T * data, const EnumDataOwned o)
{
#if TwoDArraySanityChecking==1
if ( (nrows()*ncols()) != len)
   EXCEPTION_MNGR(runtime_error, "Basic3DArray<T>::set_data : bad vector size " << (nrows()*ncols()) << " != " << len);
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

#endif
