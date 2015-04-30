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
 * \file ArrayBase.h
 *
 * Defines the utilib::ArrayBase class
 */

#ifndef utilib_ArrayBase_h
#define utilib_ArrayBase_h

#include <utilib/std_headers.h>
#include <utilib/_generic.h>
#include <utilib/exception_mngr.h>

#include <utilib/Serialize.h>

namespace utilib {

/**
 * \def ArraySanityChecking
 *
 * If the macro ArraySanityChecking=1 then some routine sanity checks will 
 * be performed when accessing vectors.  Sanity checking generally degrades 
 * performance.  The default is is to perform sanity checking.
 *
 * Note: Because inlines are typically put "in place" when the optimization flag
 * -O is used, you can define some files with ArraySanityChecking=0 to 
 * acheive better performance, while using the default debugging mode in the
 * rest of the code.
 */
#if !defined(ArraySanityChecking)
#define ArraySanityChecking    1
#endif

template <class A, class P>
class ArrayBase;


/**
 * This class implements sharable array objects.  This class implements
 * features common to both regular arrays, such as BasicArray<T>,
 * SimpleArray<T>, NumArray<T>, {Int,Real,Double}Vector, and CharString,
 * and also bit-oriented arrays implemented via the BitArrayBase class.
 * 
 * It is essentially a replacement for the old ClassRef class, but 
 * (for the time being, at least), only works for array-like objects
 * that have a "length".
 *
 * The template argument "A" refers to what kind of memory objects 
 * are actually allocated.  The template argument "P" refers to what
 * kinds of pointers are stored to other objects sharing the same memory.
 *
 * This class differs from ClassRef in that sharing is implemented
 * via an embedded linked list instead of a separate 
 * ArrayRef/ClassRef object. That means there are two "extra" pointers 
 * in the array object, but sharing is fairly simple, without any 
 * overhead being imposed on the indexing [ ] operator.  The tradeoff
 * is that resizing a shared array carries O(s) overhead, where s
 * is the number of sharers.  Deletion is O(1), however.
 *
 */
template <class A, class P>
class ArrayBase
{
public:

  /// Destructor
  virtual ~ArrayBase()
	{ free(); }

  /// Returns true if this array shares data with another array.
  bool shared_mem() const
	{ return true_pointer(prev_share) || next_share; }

  /// Resizes the array to the given, nonnegative, value.
  /// If the second argument is non-zero (the default), the old contents
  /// are copied, and if the new array is longer, the new elements
  /// are set with the \b initialize method.
  virtual void resize(const size_type newl,int set_new_contents=1);

  /// Returns true if size == 0 (for STL compatibility)
  bool empty() const
        { return Len == 0; }

  /// Returns the length of the array.
  size_type size() const
	{ return Len; }

  /// This method controls the mapping of the size of the array
  /// to the actual number of type "A" elements allocated.  The
  /// default implementation is the identity mapping, which should
  /// work for everything except BitArrays and CharStrings.
  virtual size_type alloc_size(size_type l) const
	{ return l; }

  /// Same, but takes the size of the array as an effective default argument
  size_type allocation() const
	{ return alloc_size(Len); }

  /// Returns the number of bytes used by the array
  size_type bytes_used() const
	{ return allocation()*sizeof(A); }

  /// Returns the internal pointer to the array.
  A* data() const
	{ return Data; }

#if 0
#ifndef SWIG

  /// A coercion operator returning a pointer to the internal array
  operator A* () const
	{ return Data; }
  
#endif
#endif

  /// Returns the number of references to the internal array.
  /// This method has to scan the entire share group and is 
  /// now deprecated.
  int nrefs() const;

  /// Disconnect from any prior data and copy the data from object \b array
  ArrayBase<A,P>& operator=(const ArrayBase<A,P>& array);

  /// Set to share storage with \b array
  ArrayBase<A,P>& operator&=(ArrayBase<A,P>& array);

  /// Method to explicitly set the Data pointer and length.
  /// Applies to all members of the share group.
  void set_data(const size_type len, 
		A* data,
		const EnumDataOwned o=DataNotOwned);

  /// Print debugging information to a stream
  void debug_print(std::ostream& str,int max_elements=0);

protected:

  /// The pointer to this array's data.
  A* Data;

  /// Pointer to the previous element of this share group.
  /// If the first in a group, and the data is not owned, contains
  /// the special pointer value special_not_owned_pointer().
  P* prev_share;

  /// Pointer to the next element of this share group.
  P* next_share;

  /// The length of the array in \a Data.  This is set up so that 
  /// it is always equal to the user's perceived length of the array.
  /// To get the actual number of type "A" elements allocated in \b Data,
  /// apply the \b alloc_size method below.
  size_type Len;

  /// This method copies from one data block to another.  The
  /// default should suffice for regular arrays, but for BitArrays
  /// and CharStrings it will need some tweaking.
  virtual void copy_data(A*        target,
			 size_type target_size,
			 A*        source,
			 size_type source_size);
  
  /// This method should be used by constructors of derived classes.
  virtual void construct(const size_type     mylen, 
			 A*                  d=0, 
			 const EnumDataOwned o=DataNotOwned);

  /// Supply initial values to part of an array.  Default is 
  /// a no-op, but for NumArrays this is overridden to set 0's.
  virtual void initialize(A*              /*data*/, 
			  const size_type /*start*/, 
			  const size_type /*stop*/)
	{}

  /// Unlink from storage.  Delete the associated memory if
  /// nobody else is linked to it.
  void free();

  /// Special bogus pointer to use to terminate the beginning of 
  /// the list when the memory block is not owned by the array.
  /// We assume that just as no data can live at address 0, it
  /// can't live at address 1 either.
  static P* special_not_owned_pointer()
	{ return (P*) 1; }

  /// Internal function for testing whether a pointer is valid.
  /// Checks for both NULL and the special_not_owned_pointer().
  static int true_pointer(P* p)
	{ return p && (p != special_not_owned_pointer()); }

  /// Generic method for dumping contents to a stream (debugging only)
  /// Default is nothing.
  virtual void dump_data(std::ostream& /*str*/, unsigned int /*max_elements*/=0)
	{}

  /// The general serializer for all ArrayBase derivatives.
  static int serializer( SerialObject::elementList_t& serial, 
                         Any& data, bool serialize );

};


///
/// METHOD DEFINITIONS
///

template <class A, class P>
void ArrayBase<A,P>::construct(const size_type mylen, 
			       A* d, 
			       const EnumDataOwned o)
{
  Len = mylen;

  if (d == NULL) 
    {
      if (mylen > 0) 
	{
	  size_type newlen = alloc_size(mylen);
	  Data = new A[newlen];
	  if (Data == 0)
	     EXCEPTION_MNGR(runtime_error, "ArrayBase::construct - new A[" << alloc_size(mylen) << "] failed.");
	  initialize(Data,0,Len);
	}
      else
	Data = NULL;
    }
  else  // if d was supplied...
    {
      if (o == DataOwned) {
	if (mylen > 0)  
	  {
	    size_type newlen = alloc_size(mylen);
	    Data = new A[newlen];
	    if (Data == 0)
	       EXCEPTION_MNGR(runtime_error, "ArrayBase::construct - new A["
                              << alloc_size(mylen) << "] failed.");
	    copy_data(Data,Len,d,Len);
	  }
      }
      else
	Data = d;
    }

  // Initialize the pointers to the rest of the share group, which is now
  // empty.  Note that if the data is not owned, we use a special bogus value
  // instead of NULL for the first pointer.

  if ((o == DataNotOwned) && d)
    prev_share = special_not_owned_pointer();
  else
    prev_share = NULL;

  next_share = NULL;

}


template <class A, class P>
void ArrayBase<A,P>::free()
{
  // Unlink from previous member of share group, if any

  if (true_pointer(prev_share))
    prev_share->next_share = next_share;

  // Unlink from next member of share group, if any

  if (next_share)
    next_share->prev_share = prev_share;

  // If there were no other members and the data is owned, free memory.

  if ((Data != NULL) && (prev_share == NULL) && (next_share == NULL))
    delete [] Data;
  
}


template <class A, class P>
void ArrayBase<A,P>::copy_data(A*        target,
			       size_type target_size,
			       A*        source,
			       size_type source_size)
{
  size_type target_alloc = alloc_size(target_size);
  size_type source_alloc = alloc_size(source_size);
  for(size_type i=0; i < target_alloc && i < source_alloc; i++)
    *(target++) = *(source++);
}



template <class A, class P>
ArrayBase<A,P>& ArrayBase<A,P>::operator=(const ArrayBase<A,P>& array)
{
  if (this != &array) 
    {
      free();
      construct(array.size(),array.data(),DataOwned);
    }
  return *this;
}



template <class A, class P>
ArrayBase<A,P>& ArrayBase<A,P>::operator&=(ArrayBase<A,P>& array)
{
  if (this == &array)
     return *this;
    
  if ((array.Data != Data) || (Data == NULL))
    { 
      free();

      Data = array.Data;
      Len  = array.Len;

      prev_share = array.prev_share;
      if (true_pointer(array.prev_share))
	array.prev_share->next_share = (P*) this;
      array.prev_share = (P*) this;
      next_share       = (P*) &array;
    }
  return *this;
}



template <class A, class P>
void ArrayBase<A,P>::resize(const size_type newl,int set_new_contents)
{
  if (newl == Len)
    return;
  
  A* d = 0;
  size_type new_alloc = alloc_size(newl);
  
  if (new_alloc == alloc_size(Len))
    d = Data;
  else if (newl > 0) 
    {
      d = new A[new_alloc];
      if (d == 0)
	 EXCEPTION_MNGR(runtime_error, "ArrayBase::resize - new T ["
			<< new_alloc << "] failed.");
      if (set_new_contents)
	 copy_data(d,newl,Data,Len);
    }

  if (set_new_contents && (newl > Len))
    initialize(d,Len,newl);
  
  set_data(newl,d,DataOwned);
}


template <class A, class P>
void ArrayBase<A,P>::set_data(const size_type len, 
					 A* data, 
					 const EnumDataOwned o)
{
  if (len == 0) 
    data = NULL;

  P* share = next_share;

  // Set data and size of all subsequent elements.

  for (; share; share = share->next_share)
    {
      share->Data = data;
      share->Len  = len;
    }

  // Set data and size for all elements except first in the chain.
  // Leave share pointing to first in the chain.

  for (share = (P*) this; 
       true_pointer(share->prev_share); 
       share = share->prev_share)
    {
      share->Data = data;
      share->Len  = len;
    }

  // Free the old memory if necessary.
  
  if ((share->Data != NULL) && 
      (share->Data != data) && 
      (share->prev_share == NULL))
    delete [] share->Data;

  // Set the length and data for the first chain member.

  share->Data = data;
  share->Len  = len;
  
  // Set the ownership flag

  if (o == DataNotOwned)
    share->prev_share = special_not_owned_pointer();
  else
    share->prev_share = NULL;
}


template <class A, class P>
int ArrayBase<A,P>::nrefs() const
{
  int n = 1;
  P*  share;
  for (share = prev_share; true_pointer(share); share = share->prev_share)
    n++;
  for (share = next_share; share; share = share->next_share)
    n++;
  return n;
}
 

template <class A, class P>
void ArrayBase<A,P>::debug_print(std::ostream& str, int /*max_elements*/ )
{
  str << "this=" << this;
  str << " size=" << size() << " Data=" << (void*) Data;
  str << " prev_share=" << prev_share;
  if (prev_share == special_not_owned_pointer())
    str << "(not owned flag)";
  str << " next_share=" << next_share << std::endl;
  //dump_data(str,max_elements);
}


/** Serialize an ArrayBase object.  Note: as no one will ever create a
 *  ArrayBase object, we will not bother to register this with the
 *  Serializer().
 */
template <class A, class P>
int ArrayBase<A,P>::serializer( SerialObject::elementList_t& serial, 
                                Any& data, bool serialize )
{
   int ans = 0;
   ArrayBase<A,P>& tmp 
      = const_cast<ArrayBase<A,P>&>(data.template expose<ArrayBase<A,P> >());
   
   size_t tmpLen = tmp.Len;
   if ( 0 != (ans = serial_transform(serial, tmpLen, serialize)) )
      return ans;
   if ( ! serialize )
      tmp.resize(tmpLen, 0);
   
   A* it = tmp.Data;
   for (size_t index = tmp.allocation(); index > 0; ++it, --index )
      if ( 0 != (ans = serial_transform(serial, *it, serialize)) )
         return ans;
      
   return 0;
}

} // namespace utilib

#endif
