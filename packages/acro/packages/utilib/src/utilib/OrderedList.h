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
 * \file OrderedList.h
 *
 * Defines the utilib::OrderedListItem and utilib::OrderedList classes
 */

#ifndef utilib_OrderedList_h
#define utilib_OrderedList_h

#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/_generic.h>
#include <utilib/CachedAllocator.h>

namespace utilib {

template <class T,class V>
class OrderedListItem;

template <class T,class V>
class OrderedList;


/**
 * A simple container class that is used by OrderedList.
 *
 * This code includes the management of a list of unused OrderedListItems,
 * which are used first before new OrderedListItems are constructed.  The
 * user can clear the unused list if that is desirable.
 */
template <class T, class V>
class OrderedListItem : public CachedAllocatorObject<OrderedListItem<T,V> >
{
  #if !defined(DOXYGEN)
  friend class OrderedList<T,V>;
  #endif

public:

  /// Construct an item
  OrderedListItem() {next = prev = 0;}

  /// Construct an item with a given data value and sort/order key.
  OrderedListItem( T Data_, V Key_) 
	: Data(Data_), Key(Key_) {next = prev = 0;}

  /// Destructor
  virtual ~OrderedListItem() {}

  /// Return the item's data value.
  T& data()  {return Data;}

  /// Return the item's key.
  V key() {return Key;}

  /// Return true if the key in \a item equals the key in the current item.
  bool operator==(const OrderedListItem<T,V>& item) 
	{return (Key == item.Key);}

  /// The item's data value.
  T Data;

  /// The item's key.
  V Key;

  /// The next item in the linked list
  OrderedListItem<T,V>* next;

  /// The previous item in the linked list
  OrderedListItem<T,V>* prev;

  ///
  void deallocate_derived()
        { CachedAllocator<OrderedListItem<T,V> > :: deallocate_derived(this); }

};



/**
 * A data type that defines an ordered doubly linked list.
 * List items are inserted after all other items with equal or lower 
 * values for their order parameter.  Note that OrderedList is not simply a 
 * subclass of LinkedList because we need to maintain a seperate pool of 
 * unused OrderedListItems 
 */
template <class T,class V>
class OrderedList
{
public:

  /// Empty constructor, which sets up an empty list.
  OrderedList() 
		{ counter++; first = last = 0; Len=0; }

  /// Copy constructor
  OrderedList(const OrderedList<T,V>& list)
		{ counter++; first = last = 0; Len=0; *this = list;}

  /** 
   * Destructor.
   * After deleting the list, the destructor deletes the list of
   * additional unused OrderedListItem objects.
   */
  virtual ~OrderedList();

  /// Copy equals
  OrderedList<T,V>& operator=(const OrderedList<T,V>& list);

  /**
   * Find the OrderedListItem with value \a data in the list.
   * If this data value is not found in the list, this method
   * returns \c NULL.
   */
  OrderedListItem<T,V>* find( V& key);

  /// Returns the head of the ordered list.
  OrderedListItem<T,V>* head() const
                {return first;}

  /// Returns the tail of the ordered list.
  OrderedListItem<T,V>* tail() const
                {return last;}

  /// Returns the next item in the list after \a item.
  OrderedListItem<T,V>* next(OrderedListItem<T,V>* item) const
                {return item->next;}         

  /// Returns the value of the next item that will be removed.
  T& top()
		{
		if (Len == 0)
		   EXCEPTION_MNGR(runtime_error, "OrderedList::top -- Empty list");
		return first->data();
		}

  /**
   * Adds a list item and returns the OrderedListItem object in the 
   * \a item argument.
   */
  OrderedListItem<T,V>* add( T& data, V& key) 
		{return insert(data,key);}
  /**
   * Removes the first item in the ordered list, returning the values \a data 
   * and \a key.
   */
  void remove( T& data, V& key)
		{extract(first,data,key);}

  /// Removes \a item from the list
  void remove(OrderedListItem<T,V>*& item)
		{extract(item);}

  /// Removes \a item from the list, returning the values \a data and \a key.
  void remove(OrderedListItem<T,V>*& item, T& data, V& key)
		{extract(item,data,key);}

  /// Changes the key value for \a item, and repositions it in the list.
  void update(OrderedListItem<T,V>* item, V& key);

  /// Returns true if the list is empty
  bool empty() const;

  /// Returns true if the list is not empty
  operator int() const 
		{return (first == 0 ? false : true);}

  /// Returns the length of the list.
  size_type size() const 
		{return Len;}

protected:

  /// Removes \a item from the linked list after extracting its data and key.
  void extract(OrderedListItem<T,V>* item, T& data, V& key)
		{ data = item->Data; key = item->Key; extract(item); }

  /// Removes \a item from the linked list.
  void extract(OrderedListItem<T,V>* item);

  /**
   * Creates an item in the list from \a data and \a key, and returns 
   * the pointer to the OrderedListItem.
   */
  OrderedListItem<T,V>* insert( T& data, V& key);

  /// Removes all items from the list
  void extract_all();

  /// The first item in the list.
  OrderedListItem<T,V> *first;

  /// The last item in the list.
  OrderedListItem<T,V> *last;

  /// The length of the list.
  int Len;

  /// The number of unused OrderedListItem objects.
  static int counter;

};




//
// OrderedList methods
//

template <class T,class V>
int OrderedList<T,V>::counter=0;


template <class T,class V>
OrderedList<T,V>::~OrderedList()
{
extract_all();
counter--;

if (!counter)
   CachedAllocator<OrderedListItem<T,V> > :: delete_unused();
}


template <class T,class V>
void OrderedList<T,V>::extract_all()
{
while (!empty())
  extract(first);
}


template <class T, class V>
OrderedList<T,V>& OrderedList<T,V>::operator=(const OrderedList<T,V>& list)
{
extract_all();
OrderedListItem<T,V>* curr = list.head();

while (curr) {
  T tdata = curr->data();
  V tkey  = curr->key();
  add(tdata,tkey);
  curr = list.next(curr);
  }
return *this;
}



//
// This is a stupid way of doing this.  I should simply update the key
// value and move the 'item'.
//
template <class T,class V>
void OrderedList<T,V>::update(OrderedListItem<T,V>* item, V& key)
{
insert(item->Data,key);
extract(item);
}


template <class T,class V>
bool OrderedList<T,V>::empty() const
{
return (first == NULL ? true : false) ;
}      


template <class T,class V>
void OrderedList<T,V>::extract(OrderedListItem<T,V>* item)
{
if (empty())
   EXCEPTION_MNGR(runtime_error, "OrderedList::extract - empty list");

if (item->prev) 
   item->prev->next = item->next;
else
   first = item->next;

if (item->next)
   item->next->prev = item->prev;
else
   last = item->prev;

CachedAllocator<OrderedListItem<T,V> > :: deallocate(item);
Len--;
}


template <class T,class V>
OrderedListItem<T,V>* OrderedList<T,V>::insert( T& data, V& key)
{
OrderedListItem<T,V>* item = CachedAllocator<OrderedListItem<T,V> > :: allocate();
item->Data = data;
item->Key = key;
item->next = item->prev = 0;

OrderedListItem<T,V>* curr = last;

while (curr && (item->Key < curr->Key))
  curr = curr->prev;

if (curr) {
   item->prev = curr;
   if (curr->next)
      curr->next->prev = item;
   else
      last = item;
   item->next = curr->next;
   curr->next = item;
   }

else {			// insert at beginning of list
   item->prev = NULL;
   item->next = first;
   if (first)
      first->prev = item;
   first = item;
   if (!last) 
      last = item;
   }

Len++;
return item;
}


template <class T,class V>
OrderedListItem<T,V>* OrderedList<T,V>::find( V& key)
{
OrderedListItem<T,V>* curr = first;

while (curr) {
  if (key == curr->key()) break;
  curr = curr->next;
  }

return curr;
}          

} // namespace utilib

#endif
