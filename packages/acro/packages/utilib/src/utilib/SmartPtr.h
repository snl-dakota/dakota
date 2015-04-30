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
 * \file SmartPtr.h
 *
 * Defines the utilib::SmartPtr class
 */

#ifndef pico_SmartPtr_h
#define pico_SmartPtr_h

#include <utilib/CachedAllocator.h>

namespace utilib {

template <class InfoType>
class SmartPtrInfo;

#ifndef _ENUMDATAOWNED_
#define _ENUMDATAOWNED_
/**
 * Ownership categories for objects with reference counts.
 */
enum EnumDataOwned
{
  DataNotOwned=0,       /**< Data owned by some other object */
  DataOwned=1,          /**< Memory allocated by object itself */
  AcquireOwnership=1,   /**< Synonym for DataOwned */
  AssumeOwnership=2     /**< We own it now, but it comes from elsewhere */
                        /**< Once the object has been made this is      */
                        /**< identical to DataOwned                     */
};
#endif


/// Base SmartPtr class that provides the default mechanism for deleting 
/// the Type objects.
template <class Type, bool cache_flag>
class SmartPtrInfoBase
{
protected:

  /// Perform deallocation by deleting the data directly
  void deallocate_data(Type* data)
	{ delete data; }
};


/// A specialization of the base SmartPtr class that specifies how
/// to deallocated data when using a cache.
template <class Type>
class SmartPtrInfoBase<Type,true>
{
protected:

  /// Perform deallocation with the CachedAllocator
  void deallocate_data(Type* data)
	{ CachedAllocator<Type> :: deallocate(data); }
};


/// Class that is used to cache information allocated by a
/// utilib::SmartPtr object
template <class Type>
class SmartPtrInfo : public SmartPtrInfoBase<Type,is_cache_allocated<Type>::value >,
  		       public CachedAllocatorObject<SmartPtrInfo<Type> >
{
public:

  /// Constructor
  SmartPtrInfo() 
		: Data(0), own(DataNotOwned), nref(0) {}

  /// Destructor
  virtual ~SmartPtrInfo() { decrement(); }

  /// Initialization routine
  void initialize(Type* Data_, EnumDataOwned own_) 
	{
	Data = Data_;
	if (Data) {
	   own=own_;
	   nref=1;
	   }
	else {
	   own=DataNotOwned;
	   nref=0;
	   }
	}

  ///
  void allocate_exec()
	{ Data = 0; own = DataNotOwned; nref = 0; }
 
  /// The data pointer
  Type* Data;

  /// The ownership flag
  EnumDataOwned own;

  /// The number of references
  int nref;

  /// Set the data of this class
  void set_data(Type* Data_, EnumDataOwned own_)
	{
	Data=Data_;
	own=own_;
	nref=1;
	}

  /// Decrement the reference counter
  void decrement()
	{
	if (nref == 0) return;
	nref--;
	if (!nref && own) { this->deallocate_data(Data); Data=0; own=DataNotOwned; }
	}

  ///
  void deallocate_derived()
        { CachedAllocator< SmartPtrInfo<Type> > :: deallocate_derived(this); }

};


/// Class that manages pointer deletion and allows for sharing of pointers
/// with reference counters
template <class Type>
class SmartPtr 
{
public:

  /// Constructor
  SmartPtr()
	{ info = CachedAllocator<SmartPtrInfo<Type> > :: allocate(); }

  /// Constructor
  SmartPtr(Type* data_, EnumDataOwned own_=AssumeOwnership)
        {
	info = CachedAllocator<SmartPtrInfo<Type> > :: allocate() ;
	if (data_)
	   info->initialize(data_,own_);
	}

  /// Copy constructor
  SmartPtr(const SmartPtr& ptr)
	{
	info = CachedAllocator<SmartPtrInfo<Type> > :: allocate();
	*this = ptr;
	}

  /// Destructor
  virtual ~SmartPtr()
        {
	info->decrement();
	if (info->nref == 0)
	   CachedAllocator<SmartPtrInfo<Type> > :: deallocate(info);
	}

  /// Resets the SmartPointer to own its own copy of SmartPtrInfo
  void reset()
	{
	info->decrement();
	if (info->nref == 0)
           CachedAllocator<SmartPtrInfo<Type> > :: deallocate(info);
	info = CachedAllocator<SmartPtrInfo<Type> > :: allocate();
	}

  /// Copy equal operator
  SmartPtr<Type>& operator=(const SmartPtr<Type>& ptr)
	{
	info->decrement();
	if (info->nref == 0)
           CachedAllocator<SmartPtrInfo<Type> > :: deallocate(info);
	info = ptr.info;
	info->nref++;
	return *this;
	}

  /// Equals operator with a pointer
  SmartPtr<Type>& operator=(Type* data)
	{
	set_data(data,AssumeOwnership);
	return *this;
	}

  /// The number of shared references of the pointer
  int nref() {return info->nref;}

  /// Not-Equal operator
  bool operator!=(const SmartPtr<Type>& ptr)
	{ return info->Data != ptr.info->Data; }

  /// Equal operator
  bool operator==(const SmartPtr<Type>& ptr)
	{ return info->Data == ptr.info->Data; }

  /// Set the data of the shared pointer
  void set_data(Type* data_, EnumDataOwned own_=DataNotOwned)
        {
	if (info->Data == data_)
           return;
	info->decrement();
	if (!data_) return;

	if (info->nref) {
	   info = CachedAllocator<SmartPtrInfo<Type> > :: allocate();
	   info->initialize(data_,own_);
	   }
	else 
	   info->set_data(data_,own_);
        }

  /// Return the pointer
  Type& operator*()
	{return *(info->Data);}

  /// Return a const pointer
  const Type& operator*() const
	{return *(info->Data);}

  /// Return the pointer
  Type* operator->()
	{return info->Data;}

  /// Return a const pointer
  const Type* operator->() const
	{return info->Data;}

  /// Coerce the class to a pointer type
  operator Type* ()
	{return info->Data;}

  /// Coerce the class to a const pointer type
  operator const Type* () const
	{return info->Data;}

  /// Return the pointer
  Type* data()
        {return info->Data;}

  /// Return the const pointer
  const Type* data() const
        {return info->Data;}

protected:

  /// The shared information
  SmartPtrInfo<Type>* info;

};

} // namespace utilib

#endif
