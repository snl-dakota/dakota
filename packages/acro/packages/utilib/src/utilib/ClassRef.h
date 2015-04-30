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
 * \file ClassRef.h
 *
 * Defines the utilib::ClassRef and utilib::ArrayRef classes
 */

#ifndef utilib_ClassRef_h
#define utilib_ClassRef_h

#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/_generic.h>

namespace utilib {

/**
 * A data type that manages the reference counting for unspecified data
 * elements.  This is a rather non-standard form of memory referencing,
 * in which the reference object knows about all of the objects for which
 * it is sharing memory.  This is not a scalable form of reference
 * management.  However, it facilitates the use of reference sharing on
 * an as-needed basis.  Further, it facilitates the fast access of data
 * in the main objects.  See the BasicArray class for an example of
 * how this reference sharing is used.
 *
 * The variable ClassRef<V>::num_refs_limit can be used to set the maximum 
 * number of references allowed in a code.  When this is exceeded, this code 
 * aborts.
 *
 * If the flag ClassRef<V>::const_ref is true, then these references are
 * assumed to share a 'constant' array, for which resize operations are not
 * permited.  This value is set by the 'freeze()' method.  Note that there
 * seems to be no reasonable way to 'unfreeze' references, since the 
 * back references are not updated for frozen ClassRef objects.
 */
template <class V> 
class ClassRef {
 
public:

  /// The default constructor
  ClassRef() {own=DataNotOwned; ref=0; ref_ptrs=0; ptrlen=0; const_ref=false;}

  /// Destructor
  virtual ~ClassRef() {if (ref_ptrs) delete [] ref_ptrs;}

  /// Update the data in the arrays sharing this object.
  void update_refs()
	{
	if (const_ref == true)
   	   EXCEPTION_MNGR(runtime_error, "Setting data for an ClassRef object with constant data references.");
	for (int i=0; i<ref; i++)
  	  copy_data(ref_ptrs[i]);
	}

  /// Add a new array to the list of arrays sharing this object
  void acquire(V* array);

  /// Remove an array from the list of arrays sharing this object
  bool release(V* array);

  /// Resize the list of reference pointers
  void resize_ref_ptrs(const int);

  /// Copy the class' data to \c array.
  virtual void copy_data(V* array) = 0;

  /// Reset the data in \c array.
  virtual void reset_data(V* array) = 0;

  /// Delete the data in the ArrayRef object
  virtual void delete_data() = 0;

  /// The ownership category.
  EnumDataOwned own;

  /// Number of arrays that are referencing this \b ClassRef instance.
  int ref;

  /// Set a flag to share a fixed data array (e.g. that is not resized).
  void freeze()
	{ const_ref=true; }

  /// If true, references share a 'constant' array
  bool const_ref;

  /// Max number of shared references
  static int num_refs_limit;

protected:

  /// Length of the \a ref_ptrs array.
  int ptrlen;

  /// Array of pointers to array objects that share this object.
  V** ref_ptrs;

};


template <class V>
int ClassRef<V>::num_refs_limit=32;



template <class V>
void ClassRef<V>::acquire(V* array)
{
if (const_ref == false) {
   //
   // Ignore num_refs_limit if this is a constant reference
   //
   if (ref == num_refs_limit)
      EXCEPTION_MNGR(runtime_error, "Exceeded maximum number of reference counts allowed in ClassRef<V>::num_refs_limit = " << num_refs_limit);

   if (ref == ptrlen)
      resize_ref_ptrs(ptrlen*2+2);
   ref_ptrs[ref++] = array;
   }
copy_data(array);
}



template <class V>
bool ClassRef<V>::release(V* array)
{
if (const_ref == false) {
   V** tmp = ref_ptrs;
   int i=0;
   for (i=0; i<(ref-1); i++, tmp++)
     if (*tmp == array)  break;
   if (i < (ref-1))
      ref_ptrs[i] = ref_ptrs[--ref];
   else
      ref--;
   if ((ptrlen > 2) && (ref < ptrlen/2))
      resize_ref_ptrs(ref);
   }
else
   ref--;

if (ref == 0) {
   delete_data();
   own = DataNotOwned;
   const_ref=false;
   delete [] ref_ptrs;
   ref_ptrs = 0;
   }

reset_data(array);
return (ref == 0);
}



template <class V>
void ClassRef<V>::resize_ref_ptrs(const int newlen)
{
V** tmp = new V* [newlen];
if (ref) {
   for (int i=0; i<min(ref,newlen); i++)
     tmp[i] = ref_ptrs[i];
   delete [] ref_ptrs;
   }
ptrlen = newlen;
ref_ptrs=tmp;
}



/// A subclass of ClassRef that is tailored for Arrays of data
template <class T, class V> 
class ArrayRef : public ClassRef<V>
{
public:

  /// The default constructor
  ArrayRef() {Len=0; Data=0;}

  /// Destructor
  virtual ~ArrayRef()
	{if (this->own) delete [] Data;}

  /// Copy the class' data to \c array.
  void copy_data(V* array);

  /// Reset the data in \c array.
  void reset_data(V* array);

  /// Delete the data in the ArrayRef object
  void delete_data()
	{ if (this->own) delete [] Data; Data=0; }

  /// Set the array data.
  void set_data(T* Data_, size_type Len_, EnumDataOwned own_);

  /// Pointer to an array of T objects.
  T* Data;

  /// Length of the \c Data array.
  size_type Len;

};


template <class T, class V>
void ArrayRef<T,V>::copy_data(V* array)
{
array->Data = Data;
array->Len  = Len;
}


template <class T, class V>
void ArrayRef<T,V>::reset_data(V* array)
{
array->Data = 0;
array->Len  = 0;
}


template <class T, class V>
void ArrayRef<T,V>::set_data(T* Data_, size_type Len_, EnumDataOwned own_)
{
if (this->const_ref == true)
   EXCEPTION_MNGR(runtime_error, "Setting data for an ArrayRef object with constant data references");

if (Data && this->own && (Len > 0))
   delete [] Data;
if (Len_ > 0) {
   Data = Data_;
   this->own = own_;
   Len = Len_;
   }
else {
   Data=0;
   Len=0;
   this->own=DataNotOwned;
   }
this->update_refs();
}

} // namespace utilib

#endif
