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
 * \file SimpleHeap.h
 *
 * Defines the utilib::SimpleHeapItem, utilib::SimpleHeapBase and 
 * utilib::SimpleHeap classes
 */

#ifndef utilib_SimpleHeap_h
#define utilib_SimpleHeap_h

#include <utilib_config.h>
#include <utilib/_generic.h>
#include <utilib/AbstractHeap.h>
#include <utilib/compare.h>

namespace utilib {

template <class ItemType>
class SimpleHeapBase;

/**
 * Defines the container class used for the SimpleHeap class.
 */
template <class T>
class SimpleHeapItem 
{

  #if !defined(DOXYGEN)
  friend class SimpleHeapBase<SimpleHeapItem<T> >;
  #endif

public:

  /// Constructor, which requires a key.
  explicit SimpleHeapItem(const T* Key_) : 
    Key(*Key_), 
    element(0), 
    ctr(1) 
    { };

  /// Copy constructor (for heap copies)
  explicit SimpleHeapItem(const SimpleHeapItem* toCopy) :
    Key(toCopy->Key),
    element(toCopy->element),
    ctr(toCopy->ctr)
    { };

  /// Write the key to an output stream.
  void write(std::ostream& os) {os << Key;};

  /// Return the key.
  T& key() {return Key;};

private:

  /// The key object.
  T Key;

  /// The index into the heap's array that represents this key.
  int element;

  /// The number of heap elements in the heap with this key.
  int ctr;
};



/**
 *  This class includes methods that access private data within
 *  a SimpleHeapItem.  This is a base class of SimpleHeap, and using
 *  this class gets around having to make SimpleHeapItem templated on
 *  the comparison class.
 */
template <class ItemType>
class SimpleHeapBase
{
protected:

  /// Return the index of a given item
  int& element(ItemType* item)
		{return item->element;}

  /// Return the counter of a given item
  int& ctr(ItemType* item)
		{return item->ctr;}
};


/**
 * A heap class that uses standard C/C++ data types for keys.
 * The SimpleHeap class is derived from AbstractHeap, which
 * defines the basic operations of the heap.   This particular
 * instantiation of heaps uses a simple data type for the key, and
 * no auxiliary data is associated with the key.  This greatly simplifies
 * the definition of the heap, but it assumes that copying the key
 * is relatively inexpensive (e.g. no memory allocation/deallocation is
 * required).
 *
 * \par
 * This template class can be instantiated with a data type for which the 
 * following methods are defined:
 *  - operator<
 *  - operator>
 *  - operator==
 *  - operator<<
 *  - operator<<(ostream& os)
 * 
 * \sa GenericHeap
 */
template <class T, class Compare=SimpleCompare<T> >
class SimpleHeap : public AbstractHeap<SimpleHeapItem<T>,T,Compare>,
			public SimpleHeapBase<SimpleHeapItem<T> >
{
public:

  #if !defined(DOXYGEN)
  typedef typename AbstractHeap<SimpleHeapItem<T>,T,Compare>::item_t item_t;
  #endif

/// Constructor, which may specify a name
SimpleHeap(const char* nameBuff = "Unnamed") :
AbstractHeap<SimpleHeapItem<T>,T,Compare>(nameBuff), 
duplicate_flag(true), 
check_duplicates_flag(false) 
{ };

/// Copy onstructor
SimpleHeap(SimpleHeap<T,Compare>& toCopy,
	   const char* nameBuff = "Copied") :
AbstractHeap<SimpleHeapItem<T>,T,Compare>(toCopy,nameBuff), 
duplicate_flag(toCopy.duplicate_falg), 
check_duplicates_flag(toCopy.check_duplicates_flag) 
{ };

  /// Returns the value of \c duplicate_flag.
  bool& duplicate() {return duplicate_flag;}

  /// Returns the value of \c check_duplicates_flag.
  bool& check_duplicates() {return check_duplicates_flag;}

protected:
 
  ///
  int& element(SimpleHeapItem<T>* item)
		{return SimpleHeapBase<SimpleHeapItem<T> >::element(item);}

  /// Records whether the last item inserted was a duplicate
  bool duplicate_flag;

  /// If \c true then track items in the tree that are duplicates
  bool check_duplicates_flag;

  ///
  SimpleHeapItem<T>* insert(T* key);

  ///
  void extract(SimpleHeapItem<T>* item, bool& status);
};


template <class T, class Compare>
SimpleHeapItem<T>* SimpleHeap<T,Compare>::insert(T* key)
{
duplicate_flag = false;
if (check_duplicates_flag) {
   SimpleHeapItem<T>* item = AbstractHeap<SimpleHeapItem<T>,T,Compare>::find(*key);  
   if (item) {
      ctr(item)++;
      this->heap_size++;
      duplicate_flag = true;
      return item;
      }
   }
return AbstractHeap<SimpleHeapItem<T>,T,Compare>::insert(key);
}


template <class T, class Compare>
void SimpleHeap<T,Compare>::extract(SimpleHeapItem<T>* item, bool& status)
{
if (check_duplicates_flag) {
   SimpleHeapItem<T>* tmp = AbstractHeap<SimpleHeapItem<T>,T,Compare>::find(item->key());
   if (tmp) {
      ctr(tmp)--;
      this->heap_size--;
      if (ctr(tmp) > 0) {
         status=true;
         return;
         }
      }
   }
AbstractHeap<SimpleHeapItem<T>,T,Compare>::extract(item,status);
}

} // namespace utilib

#endif
