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
 * \file CachedAllocator.h
 *
 * Defines the utilib::CachedAllocator and utilib::CacheList classes
 */

#ifndef utilib_CachedAllocator_h
#define utilib_CachedAllocator_h

#include <utilib/std_headers.h>
#include <utilib/CommonIO.h>

namespace utilib {

///
template <class TYPE>
class CachedAllocator;

///
template <class TYPE>
class CacheList;

/// Trait that determines whether or not an object is a cache_allocated type
template <class DomainT>
struct is_cache_allocated
{
public:

  /// A boolean that is true if this object is cache allocated
  static const bool value = false;

};


/**
 * A class that provides methods for objects that will be cache allocated.
 */
template <class TYPE>
class CachedAllocatorObject
{
public:

  ///
  virtual ~CachedAllocatorObject() {}

  /// Called when an object is allocated
  virtual void allocate_exec() {}
  
  /// Called when an object is deallocated
  virtual void deallocate_exec() {}

  /// Put the derived class into the appropriate cache
  virtual void deallocate_derived() = 0;
};


/**
 * A class that redefines the \c new and \c delete methods for a class
 * to cache the allocation and deallocation of objects.
 */
template <class TYPE>
class CachedAllocator
{
public:

  /// Constructor
  CachedAllocator() {}

  /// Delete unused objects
  static void delete_unused();

  /// A cached allocator
  static TYPE* allocate();

  /// A cached deallocator
  static void deallocate(TYPE* );

  /// The deallocator that does the real work, but shouldn't be called
  static void deallocate_derived(TYPE* );

  /// Flag that turns off caching.  For example, this is used
  /// to turn off caching permanently.
  static bool cache_enabled;

protected:

  /// The list of unused objects
  static CacheList<TYPE>* unused_list;

  /// The length of the unused_list.
  //static int unused_len;

  /// A list of CacheList objects that have been allocated but are not in use.
  /// These objects have an empty 'data' field.
  static CacheList<TYPE>* tmp_list;

  /// The length of the tmp_list.
  //static int tmp_len;

};


/// A class used by CachedAllocator for a simple singly-linked list
template <class ListType>
class CacheList
{
  friend class CachedAllocator<ListType>;

  /// Constructor
  CacheList() 
	: next(0), data(0) {}

  /// Destructor
  ~CacheList()
	{}

public:

  /// The next element in the list
  CacheList<ListType>* next;

  /// The list item data
  ListType* data;
};


template <class TYPE>
bool CachedAllocator<TYPE>::cache_enabled = true;

template <class TYPE>
CacheList<TYPE>* CachedAllocator<TYPE>::unused_list = 0;

template <class TYPE>
CacheList<TYPE>* CachedAllocator<TYPE>::tmp_list = 0;

//template <class TYPE>
//int CachedAllocator<TYPE>::unused_len = 0;

//template <class TYPE>
//int CachedAllocator<TYPE>::tmp_len = 0;


template <class TYPE>
void CachedAllocator<TYPE>::delete_unused()
{
//ucout << "CachedAllocator::delete_unused - unused= " << unused_len << " tmp_len= " << tmp_len << endl;

while (unused_list) {
  CacheList<TYPE>* tmp = unused_list->next;
  delete unused_list->data;
  delete unused_list;
  unused_list = tmp;
  }

while (tmp_list) {
  CacheList<TYPE>* tmp = tmp_list->next;
  delete tmp_list;
  tmp_list = tmp;
  }
}


template <class TYPE>
TYPE* CachedAllocator<TYPE>::allocate()
{
TYPE* ans;
if (!unused_list) {
   ans = new TYPE;
   return ans;
   }

ans = unused_list->data;
ans->allocate_exec();
unused_list->data = 0;
//
// Move the unused_list data object to the tmp_list
//
CacheList<TYPE>* tmp = unused_list->next;
unused_list->next = tmp_list;
tmp_list = unused_list;
unused_list = tmp;

return ans;
}


template <class TYPE>
void CachedAllocator<TYPE>::deallocate(TYPE* obj)
{ obj->deallocate_derived(); }


template <class TYPE>
void CachedAllocator<TYPE>::deallocate_derived(TYPE* obj)
{
obj->deallocate_exec();
if (!cache_enabled) {
   delete obj;
   return;
   }
CacheList<TYPE>* tmp;
if (tmp_list) {
   tmp = tmp_list->next;
   tmp_list->next = unused_list;
   unused_list = tmp_list;
   tmp_list = tmp;
   }
else {
   tmp = new CacheList<TYPE>;
   tmp->next = unused_list;
   unused_list = tmp;
   }
unused_list->data = obj;
}

} // namespace utilib

#endif
