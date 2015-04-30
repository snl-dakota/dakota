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
 * \file LinkedList.h
 *
 * Defines the utilib::ListItem and utilib::LinkedList classes
 */

#ifndef utilib_LinkedList_h
#define utilib_LinkedList_h

#include <utilib/std_headers.h>
#include <utilib/exception_mngr.h>
#include <utilib/CachedAllocator.h>
#include <utilib/_generic.h>
#include <utilib/PackBuf.h>


namespace utilib {

using std::bidirectional_iterator_tag;

template <class T, class _Alloc>
class LinkedItem;

template <class _Tp, class _Alloc>
class LinkedList;

template<typename _Tp, typename _Ref, typename _Ptr, typename _Node,
			typename _NodeOpClass>
class __LinkedList_iterator;


/**
 * A simple container class that is used by LinkedList.
 *
 * This code includes the management of a list of unused ListItem's,
 * which are used first before new ListItem's are constructed.  The
 * user can clear the unused items if that is desirable.
 */
template <class T>
class ListItem : public CachedAllocatorObject<ListItem<T> >
{

  #if !defined(DOXYGEN)
  template <class wasT, class wasAlloc> friend class LinkedList;
  #endif

public:

  /// Construct an item with the given data value.
  ListItem( const T& Data_)
		: Data(Data_) {next = prev = 0;}

  /// Construct an item with the given data value.
  ListItem( )
		{next = prev = 0;}

  /// Destructor
  virtual ~ListItem() {}

  /// Return the item's data value.
  T& data() 
		{return Data;}

  /// Return true if \a item equals the data value in the current item.
  bool operator==(const ListItem<T>& item) 
		{return (Data == item.Data);}

  /// The item's data value.
  T Data;

  /// The next item in the linked list
  ListItem<T>* next;

  /// The previous item in the linked list
  ListItem<T>* prev;

  ///
  void deallocate_derived()
	{ CachedAllocator<ListItem<T> > :: deallocate_derived(this); }

};



/**
 * Base class for __LinkedList_iterator that defines operations that
 * are specific to the _Node object used to represent the elements of
 * the linked list.
 */
template<typename _Tp, typename _Ref, typename _Ptr, typename _Node,
					typename _NodeOpClass>
class __LinkedList_iterator_base : public _NodeOpClass
{
public:

  /// Pointer to the data that is being iteratored through
  _Node* _M_node;

  /// Constructor
  __LinkedList_iterator_base()
        : _M_node(0)
        { }

  /// Constructor
  __LinkedList_iterator_base(_Node* __x)
        : _M_node(__x)
        { }

  /// Increment the iterator
  void _M_incr()
	{ _M_node = _M_node->next; }

  /// Decrement the iterator
  void _M_decr()
	{ _M_node = _M_node->prev; }

  /// Return a reference to the data
  _Ref operator*() const  
	{ return this->value(_M_node); }

  /// Return a pointer to the data
  _Ptr operator->() const
	{ _Ref tmp = this->operator*(); return &tmp; }

};



/**
 * Base class for __LinkedList_iterator that a LinkedList can use
 * as an iterator for the array
 */
template<typename _Ref, typename _Node>
class __LinkedList_Standard_OpClass
{
public:

  /// Return a reference to the data being passed in
  _Ref value(_Node* node) const
	{
	if (!node)
	   EXCEPTION_MNGR(runtime_error,"Accessing an invalid iterator.");
	return node->Data;
	}

};



/**
 * Base class for __LinkedList_iterator that a LinkedList can use
 * as an iterator for the array
 */
template<typename _Ref, typename _Node>
class __LinkedList_Pointer_OpClass
{
public:

  /// Return a reference to the data being passed in
  _Ref value(_Node* node) const
	{
	if (!node)
	   EXCEPTION_MNGR(runtime_error,"Accessing an invalid iterator.");
	return *(node->Data);
	}

};


/**
 * class for __LinkedList_iterator that a LinkedList can use
 * as an iterator for the array
 */
template<typename _Tp, typename _Ref, typename _Ptr, typename _Node,
					typename _NodeOpClass>
class __LinkedList_iterator : 
	public __LinkedList_iterator_base<_Tp,_Ref,_Ptr,_Node,_NodeOpClass>
{
public:

  #if !defined(DOXYGEN)
  ///
  typedef size_t size_type;

  ///
  typedef ptrdiff_t difference_type;

  ///
  typedef bidirectional_iterator_tag iterator_category;

  ///
  typedef _Tp value_type;

  ///
  typedef _Ptr pointer;

  ///
  typedef _Ref reference;

  ///
  typedef __LinkedList_iterator<_Tp,_Ref,_Ptr,_Node,_NodeOpClass> iterator;
  #endif

  /// Constructor
  __LinkedList_iterator()
	: __LinkedList_iterator_base<_Tp,_Ref,_Ptr,_Node,_NodeOpClass>() {}

  /// Constructor
  __LinkedList_iterator(_Node* __x)
	: __LinkedList_iterator_base<_Tp,_Ref,_Ptr,_Node,_NodeOpClass>(__x) {}

  /// Copy operator
  iterator& operator=(const iterator& __x)
	{
	this->_M_node = __x._M_node;
	return *this;
	}
  
  /// Equality operator
  bool operator==(const iterator& __x) const
	{ return this->_M_node == __x._M_node; }

  /// Inequality operator
  bool operator!=(const iterator& __x) const
	{ return this->_M_node != __x._M_node; }

  /// Increment the iterator
  iterator& operator++()
	{
        this->_M_incr();
        return *this;
        }

  /// Increment the iterator
  iterator operator++(int)
	{
        iterator __tmp = *this;
        this->_M_incr();
        return __tmp;
	}

  /// Decrement the iterator
  iterator& operator--()
	{
        this->_M_decr();
        return *this;
	}

  /// Decrement the iterator
  iterator operator--(int)
	{
        iterator __tmp = *this;
        this->_M_decr();
        return __tmp;
	}

};


/**
 * class for __LinkedList_ptr_iterator that a LinkedList can use
 * as an iterator for the array
 */
template<typename _Tp, typename _Ref, typename _Ptr, typename _Node,
					typename _NodeOpClass>
class __LinkedList_ptr_iterator : 
	public __LinkedList_iterator_base<_Tp,_Ref,_Ptr,_Node,_NodeOpClass>
{
public:

#if !defined(DOXYGEN)
  ///
  typedef size_t size_type;

  ///
  typedef ptrdiff_t difference_type;

  ///
  typedef bidirectional_iterator_tag iterator_category;

  ///
  typedef _Tp value_type;

  ///
  typedef _Ptr pointer;

  ///
  typedef _Ref reference;

  ///
  typedef __LinkedList_ptr_iterator<_Tp,_Ref,_Ptr,_Node,_NodeOpClass> iterator;
#endif

  /// Constructor
  __LinkedList_ptr_iterator()
	: __LinkedList_iterator_base<_Tp,_Ref,_Ptr,_Node,_NodeOpClass>() {}

  /// Constructor
  __LinkedList_ptr_iterator(_Node* __x)
	: __LinkedList_iterator_base<_Tp,_Ref,_Ptr,_Node,_NodeOpClass>(__x) {}

  /// Copy constructor
  __LinkedList_ptr_iterator(const iterator& __x)
	: __LinkedList_iterator_base<_Tp,_Ref,_Ptr,_Node,_NodeOpClass>(__x._M_node) {}

  /// Constructor
  __LinkedList_ptr_iterator(const typename __LinkedList_iterator<_Tp*,_Ref,_Ptr,_Node,_NodeOpClass>::iterator& __x)
	: __LinkedList_iterator_base<_Tp,_Ref,_Ptr,_Node,_NodeOpClass>(__x._M_node) {}

  /// Copy operator
  iterator& operator=(const iterator& __x)
	{
	this->_M_node = __x._M_node;
	return *this;
	}
  
  /// Copy operator
  iterator& operator=(const typename __LinkedList_iterator<_Tp*,_Tp*&,_Tp**,_Node,_NodeOpClass>::iterator& __x)
	{ this->_M_node = __x._M_node; return *this; }

  /// Copy operator
  iterator& operator=(const typename __LinkedList_iterator<_Tp*,_Tp* const&,_Tp*const *,_Node,_NodeOpClass>::iterator& __x)
	{ this->_M_node = __x._M_node; return *this; }

  /// Returns true if the current iterator points to a value that differs
  /// from the value returned by this iterator.
  bool operator!=(const iterator& __x) const
	{ return this->_M_node != __x._M_node; }

  /// Increment the iterator
  iterator& operator++()
	{
        this->_M_incr();
        return *this;
        }

  /// Increment the iterator
  iterator operator++(int)
	{
        iterator __tmp = *this;
        this->_M_incr();
        return __tmp;
	}

  /// Decrement the iterator
  iterator& operator--()
	{
        this->_M_decr();
        return *this;
	}

  /// Decrement the iterator
  iterator operator--(int)
	{
        iterator __tmp = *this;
        this->_M_decr();
        return __tmp;
	}

};


/**
 * A class that defines a doubly-linked list.
 *
 * The design of this class was strongly influenced by
 * Stanley B. Lippman, "C++ Primer, 2nd Edition."   Addison Wesley, 1991.
 * This linked list code can be switched between use as a queue, stack or
 * simple linked list by setting different `mode' values.  By default, a
 * LinkedList object behaves like a queue.
 */
template <class _Tp, class _Alloc = std::allocator<_Tp> >
class LinkedList
{
public:

#if !defined(DOXYGEN)
  ///
  typedef _Tp value_type;

  ///
  typedef value_type* pointer;

  ///
  typedef const value_type* const_pointer;

  ///
  typedef value_type& reference;

  ///
  typedef const value_type& const_reference;

  ///
  typedef ListItem<_Tp> _Node;

  ///
  typedef size_t size_type;

  ///
  typedef ptrdiff_t difference_type;

  ///
  typedef __LinkedList_iterator<_Tp,_Tp&,_Tp*,_Node,__LinkedList_Standard_OpClass<_Tp&,_Node> > iterator;

  ///
  typedef __LinkedList_iterator<_Tp,const _Tp&,const _Tp*,_Node,__LinkedList_Standard_OpClass<_Tp&,_Node> > const_iterator;

#if  defined(COUGAR) || defined(TFLOPS_SERVICE)

#if 0	/// disabled because the coompiler has problems...
   ///
   typedef reverse_bidirectional_iterator<const_iterator, value_type,
	         const_reference, difference_type>
	const_reverse_iterator;

   ///
   typedef reverse_bidirectional_iterator<iterator, value_type, reference,
		difference_type>
	reverse_iterator;
#endif

#elif 0 // defined(UTILIB_SOLARIS_CC)
  // disabled as the Solaris 10 compiler appears to no longer need this
  // specialization

  ///
  typedef std::__reverse_bi_iterator<const_iterator,
      bidirectional_iterator_tag, value_type,
      const_reference, const_pointer, difference_type>
      const_reverse_iterator;

  ///
  typedef std::__reverse_bi_iterator<iterator,
      bidirectional_iterator_tag, value_type,
      reference, pointer, difference_type>
      reverse_iterator;

#else
  ///
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  ///
  typedef std::reverse_iterator<iterator> reverse_iterator;
#endif
#endif

  /// Empty constructor, which sets up an empty linked list.
  LinkedList()
	{
	mode=queueLL;
	counter++;
	first = last = CachedAllocator<ListItem<_Tp> > :: allocate();
	last->next = last->prev = 0;
	Len=0;
	validate_flag=false;
	}

  /** 
   * Destructor.
   * After deleting the list, the destructor deletes the list of
   * additional unused ListItem objects.
   */
  virtual ~LinkedList();

  /**
   * Copy equal operator.
   */
   LinkedList<_Tp,_Alloc>& operator=(const LinkedList<_Tp,_Alloc>& list_)
	{
	validate_flag = list_.validate_flag;
	mode = list_.mode;
	clear();
	const_iterator curr = list_.begin();
	const_iterator end  = list_.end();
	while (curr != end) {
	  push_back(*curr);
	  curr++;
	  }
	return *this;
	}

  /**
   * Copy constructor.
   */
   LinkedList(const LinkedList<_Tp,_Alloc>& list_)
	{
	mode=queueLL;
	counter++;
	first = last = CachedAllocator<ListItem<_Tp> > :: allocate();
	last->next = last->prev = 0;
	Len=0;
	validate_flag=false;
 	*this = list_;
	}

  /**
   * Find the ListItem with value \a data in the list.
   * If this data value is not found in the list, this method
   * returns \c NULL.
   */
  ListItem<_Tp>* find(const _Tp& data);

  /// Returns the iterator that defines the beginning of the list
  iterator begin()
	{ return first; }

  /// Returns the iterator that defines the ending of the list
  iterator end()
	{ return last; }

  /// Returns the const iterator that defines the beginning of the list
  const_iterator begin() const
	{ return first; }

  /// Returns the const iterator that defines the ending of the list
  const_iterator end() const
	{ return last; }
	//{ return const_cast<ListItem<_Tp>*>(last); }

  ///
  /// Returns a reverse iterator to the last element of the list
  reverse_iterator rbegin()
	{ return reverse_iterator(end()); }

  /// Returns a reverse iterator to the last element of the list
  const_reverse_iterator rbegin() const
	{ return const_reverse_iterator(end()); }

  /// Returns a reverse iterator to the first element of the list
  reverse_iterator rend()
	{ return reverse_iterator(begin()); }

  /// Returns a reverse iterator to the first element of the list
  const_reverse_iterator rend() const
	{ return const_reverse_iterator(begin()); }

  /// Returns an iterator to the first element of the list
  reference front()
	{
	if (Len == 0)
	   EXCEPTION_MNGR(runtime_error, "LinkedList::front() - empty list");
	return *begin();
	}

  /// Returns an iterator to the first element of the list
  const_reference front() const
	{
	if (Len == 0)
	   EXCEPTION_MNGR(runtime_error, "LinkedList::front() - empty list");
	return *begin();
	}

  /// Returns an iterator to the last element of the list
  reference back()
	{
	if (Len == 0)
	   EXCEPTION_MNGR(runtime_error, "LinkedList::back() - empty list");
	return *(--end());
	}

  /// Returns an iterator to the last element of the list
  const_reference back() const
	{
	if (Len == 0)
	   EXCEPTION_MNGR(runtime_error, "LinkedList::back() - empty list");
	return *(--end());
	}

  /// Insert __x before the specified position
  iterator insert(iterator __position, const _Tp& __x)
	{
	return insert_value(__x, __position._M_node);
	}

  /// Insert an empty value before the specified position
  iterator insert(iterator __position)
	{ return insert_value(__position._M_node); }

  /// Insert a value to the begining of the list
  void push_front(const _Tp& __x)
	{ insert(begin(), __x); }

  /// Insert an empty element to the begining of the list
  void push_front()
	{ insert(begin()); }

  /// Insert a value to the end of the list
  void push_back(const _Tp& __x)
	{ insert(end(), __x); }

  /// Insert an empty element to the end of the list
  void push_back()
	{ insert(end()); }

  /// Remove an element at the specified position
  iterator erase(iterator __position)
	{
	ListItem<_Tp>* next = __position._M_node->next;
	remove(__position._M_node);
	return iterator(next);
	}

  /// Clear the list
  void clear()
	{
	while (!empty())
	  pop_front();
	}


  /// Remove an item from the beginning
  void pop_front()
	{ erase(begin()); }

  /// Remove an item from the end
  void pop_back()
	{
        iterator __tmp = end();
        erase(--__tmp);
	}

  /// Returns the head of the linked list.
  ListItem<_Tp>* head() const
	{return (first == last ? 0 : first);}

  /// Returns the tail of the linked list.
  ListItem<_Tp>* tail() const
	{return last->prev;}

  /// Returns the next item in the list after \a item.
  ListItem<_Tp>* next(ListItem<_Tp>* item) const
		{return (item->next == last ? 0 : item->next);}

  /// Returns the value of the next item that will be removed.
  _Tp& top()
	{
	if (Len <= 0) {
	   EXCEPTION_MNGR(runtime_error, "LinkedList::top -- Empty list");
	   exit(0);
	   }
	if (mode == queueLL) return first->data();
	return last->prev->data();
	}

  /**
   * Adds a list item with data value \a data, and returns the ListItem
   * object that is generated.
   */
  ListItem<_Tp>* add(const _Tp& data)
	{ return insert_value(data,0); }

  /**
   * Adds a list item with data value \a data, which is inserted
   * just before the given position.  Returns the ListItem 
   * object in the \a item argument.
   */
  ListItem<_Tp>* add(const _Tp& data, ListItem<_Tp>* position)
	{ return insert_value(data,position); }

  /**
   * Insert an item in the list with data value \a data.
   * If \a item is not null, then the new item is inserted before
   * \a next.  Otherwise, it is inserted at the end of the list.
   */
  ListItem<_Tp>* insert_value( const _Tp& data, ListItem<_Tp>* item)
	{
	ListItem<_Tp>* tmp = insert_value(item);
	tmp->Data=data;
	return tmp;
	}

  /**
   * Insert an empty item in the list.
   * If \a item is not null, then the new item is inserted before
   * \a next.  Otherwise, it is inserted at the end of the list.
   */
  ListItem<_Tp>* insert_value(ListItem<_Tp>* item);

  /// Removes the next list item and returns the data value in \a data.
  void remove( _Tp& data) 
	{
	if (mode==queueLL) 
	   extract(first,data); 
	else
	   extract(last->prev,data);
	}

  /// Removes \a item from the list.
  void remove( ListItem<_Tp>* item) 
	{extract(item);}

  /// Removes \a item from the list and returns the data value in \a data.
  void remove(ListItem<_Tp>* item, _Tp& data) 
	{extract(item,data);}

  /// Returns true if the list is empty
  bool empty() const;

  /// Returns true if the list is not empty
  operator bool() const
	{return (first == last ? false : true);}

  /// Returns the length of the list.
  size_type size() const 
	{return Len;}

  /// Sets the add/remove mode to operate a stack.
  void stack_mode()
	{mode = stackLL;}

  /// Sets the add/remove mode to operate a queue (the default).
  void queue_mode()
	{mode = queueLL;}

  /// If true, then validate extraction operations
  bool validate_flag;

  /// Write this object to a stream
  void write(std::ostream& os) const;

  /// Read this object from a stream
  void read(std::istream& is);

  /// Write this object to a buffer
  void write(PackBuffer& os) const;

  /// Read this object from a buffer
  void read(UnPackBuffer& is);

protected:

  /// Defines the different modes that the LinkedList can operate
  enum {
	/// Adds items to the beginning; Removes items from the beginning
	stackLL = 0,
	/// Adds items to the end; Removes items from the beginning
	queueLL = 1	
	};

  /// The add/remove mode.
  int mode;

  /// Removes an item from the list.
  void extract(ListItem<_Tp>* item);

  /// Removes an item from the list and returns its value in \a data.
  void extract(ListItem<_Tp>* item, _Tp& data)
		{data = item->Data; extract(item);}

  /// The first item in the list.
  ListItem<_Tp> *first;

  /// The last item in the list.
  ListItem<_Tp>* last;

  /// The length of the list.
  size_type Len;

  /// The number of unused ListItem objects.
  static int counter;

  /// A routine used to debug/validate the LinkedList class
  void validate(ListItem<_Tp>* item=0);
};





//
// LinkedList methods
//

template <class _Tp, class _Alloc>
int LinkedList<_Tp,_Alloc>::counter=0;


template <class _Tp, class _Alloc>
LinkedList<_Tp,_Alloc>::~LinkedList()
{
while (!empty())
  extract(first);
counter--;
CachedAllocator<ListItem<_Tp> > :: deallocate(last);
last = 0;

if (!counter)
   CachedAllocator<ListItem<_Tp> > :: delete_unused();
}


template <class _Tp, class _Alloc>
bool LinkedList<_Tp,_Alloc>::empty() const
{
return (first == last ? true : false) ;
}


template <class _Tp, class _Alloc>
void LinkedList<_Tp,_Alloc>::extract(ListItem<_Tp>* item)
{
if (item == last)
   EXCEPTION_MNGR(runtime_error, "LinkedList::extract - trying to erase 'last'");
if (empty())
   EXCEPTION_MNGR(runtime_error, "LinkedList<_Tp,_Alloc>::extract : empty list");

if (validate_flag)
   validate(item);

if (item->prev) 
   item->prev->next = item->next;
else 
   first = item->next;

item->next->prev = item->prev;
Len--;

if (validate_flag)
   validate();

CachedAllocator<ListItem<_Tp> > :: deallocate(item);
item = 0;
}


template <class _Tp, class _Alloc>
ListItem<_Tp>* LinkedList<_Tp,_Alloc>::insert_value(ListItem<_Tp>* next)
{
ListItem<_Tp>* item = CachedAllocator<ListItem<_Tp> > :: allocate();
item->next = item->prev = 0;

if (next) {
   if (next->prev)
      next->prev->next = item;
   else
      first = item;
   item->next = next;
   item->prev = next->prev;
   next->prev = item;
   }
else { 			// appending to end of list 
   if (last->prev) {
      last->prev->next = item;
      item->prev = last->prev;
      item->next = last;
      last->prev = item;
      }
   else {
      first = last->prev = item;
      first->next = last;
      }
   }

Len++;
if (validate_flag)
   validate();
return item;
}


template <class _Tp, class _Alloc>
ListItem<_Tp>* LinkedList<_Tp,_Alloc>::find(const _Tp& data)
{
ListItem<_Tp>* curr = first; //(first == last ? 0 : first);

while (curr != last) {
  if (data == curr->data()) break;
  curr = curr->next;
  }

return curr;
}

template <class _Tp, class _Alloc>
void LinkedList<_Tp,_Alloc>::validate(ListItem<_Tp>*item)
{
if (first == last) {
   if (Len != 0)
      EXCEPTION_MNGR(runtime_error, "Nonzero length but first==last");
   if (last->next || last->prev)
      EXCEPTION_MNGR(runtime_error, "Bad link pointers in last");
   return;
   }

if (last->next)
   EXCEPTION_MNGR(runtime_error, "Bad next pointers in last");
if (first->prev)
   EXCEPTION_MNGR(runtime_error, "Bad prev pointers in first");

ListItem<_Tp>* curr = first;
unsigned int ctr=0;
while (curr != last) {
  ctr++;
  if (ctr > Len)
     EXCEPTION_MNGR(runtime_error, "More than Len items in the list");
  if ((curr != first) && (curr->prev == 0))
     EXCEPTION_MNGR(runtime_error, "Null prev ptr for non-first");
  if ((curr != last) && (curr->next == 0))
     EXCEPTION_MNGR(runtime_error, "Null next ptr for non-last");
  if ((curr->prev) && (curr->prev->next != curr))
     EXCEPTION_MNGR(runtime_error, "curr->prev->next != curr");
  if ((curr->next) && (curr->next->prev != curr))
     EXCEPTION_MNGR(runtime_error, "curr->next->prev != curr");
  curr = curr->next;
  }

if (!item) return;

curr = first;
bool flag=false;
while (curr != last) {
  if (curr == item) {
     if (curr->next != item->next)
        EXCEPTION_MNGR(runtime_error, "curr->next != item->next");
     if (curr->prev != item->prev)
        EXCEPTION_MNGR(runtime_error, "curr->prev != item->prev");
     flag=true;
     break;
     }
  curr = curr->next;
  }
if (!flag)
   EXCEPTION_MNGR(runtime_error, "The given item is not in the list!");
}


template <class _Tp, class _Alloc>
void LinkedList<_Tp,_Alloc>::write(std::ostream& os) const
{
utilib::ListItem<_Tp>* curr = (first == last ? 0 : first);

os << size() << " : ";
if (size() == 0) return;
while (curr != last) {
  os << curr->data() << " ";
  curr = curr->next;
  }
}


template <class _Tp, class _Alloc>
void LinkedList<_Tp,_Alloc>::write(utilib::PackBuffer& os) const
{
utilib::ListItem<_Tp>* curr = (first == last ? 0 : first);

os << size();
if (size() == 0) return;
while (curr != last) {
  os << curr->data();
  curr = curr->next;
  }
}


template <class _Tp, class _Alloc>
void LinkedList<_Tp,_Alloc>::read(std::istream& is)
{
int tmp;
is >> tmp;
char c;
is >> c;
for (int i=0; i<tmp; i++) {
  _Tp item;
  is >> item;
  add(item);
  }
}


template <class _Tp, class _Alloc>
void LinkedList<_Tp,_Alloc>::read(utilib::UnPackBuffer& is)
{
size_type tmp;
is >> tmp;
for (size_type i=0; i<tmp; i++) {
  _Tp item;
  is >> item;
  add(item);
  }
}


} // namespace utilib

/// Out-stream operator for removing data from a list
template <class _Tp, class _Alloc>
inline utilib::LinkedList<_Tp,_Alloc>& operator>>(utilib::LinkedList<_Tp,_Alloc>& list, _Tp& data)
{list.remove(data); return list;}

/// Out-stream operator for adding data to a list
template <class _Tp, class _Alloc>
inline utilib::LinkedList<_Tp,_Alloc>& operator<<(utilib::LinkedList<_Tp,_Alloc>& list, const _Tp& data)
{list.add(data); return list;}

/// Out-stream operator for writing the contents of a LinkedList
template <class _Tp, class _Alloc>
inline std::ostream& operator<<(std::ostream& os, 
				const utilib::LinkedList<_Tp,_Alloc>& obj)
{ obj.write(os); return(os); }

/// Out-stream operator for writing the contents of a LinkedList
template <class _Tp, class _Alloc>
inline utilib::PackBuffer& operator<<(utilib::PackBuffer& os,
				const utilib::LinkedList<_Tp,_Alloc>& obj)
{ obj.write(os); return(os); }

/// In-stream operator for reading the contents of a LinkedList
template <class _Tp, class _Alloc>
inline std::istream& operator>>(std::istream& is, 
				utilib::LinkedList<_Tp,_Alloc>& obj)
{ obj.read(is); return(is); }

/// In-stream operator for reading the contents of a LinkedList
template <class _Tp, class _Alloc>
inline utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is,
				utilib::LinkedList<_Tp,_Alloc>& obj)
{ obj.read(is); return(is); }

#endif
