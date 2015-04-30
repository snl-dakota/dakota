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
 * \file AbstractHashTable.h
 *
 * Defines the utilib::AbstractHashTable class
 */

#ifndef utilib_AbstractHashTable_h
#define utilib_AbstractHashTable_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/LinkedList.h>
#include <utilib/BasicArray.h>
#include <utilib/hash_fn.h>
#include <utilib/compare.h>
#include <utilib/_math.h>

namespace utilib {

template <class T, class KEY, class InfoT>
class AbstractHashTable;

}

template <class T, class KEY, class InfoT>
std::ostream& operator<<(std::ostream& os,
			const utilib::AbstractHashTable<T,KEY,InfoT>& table);

template <class T, class KEY, class InfoT>
utilib::PackBuffer& operator<<(utilib::PackBuffer& os,
			const utilib::AbstractHashTable<T,KEY,InfoT>& table);

template <class T, class KEY, class InfoT>
std::istream& operator>>(std::istream& is, 
			utilib::AbstractHashTable<T,KEY,InfoT>& table);

template <class T, class KEY, class InfoT>
utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is,
			utilib::AbstractHashTable<T,KEY,InfoT>& table);

namespace utilib {

/**
 * Externs of a list of primes that are good for hash tables.
 */
extern unsigned long 	utilib_prime_list[];
/**
 * Number of primes in the external list.
 */
extern int 		utilib_num_primes;


/**
 * Implements an abstract class for defining the core operations of a
 * hash table with chaining. 
 */
template <class T, class KEY, class InfoT>
class AbstractHashTable
{ 
  #if !defined(DOXYGEN)
  #if !defined(UTILIB_NO_MEMBER_TEMPLATE_FRIENDS)
  friend std::ostream& ::operator<< <T,KEY,InfoT>(std::ostream& os,
			const utilib::AbstractHashTable<T,KEY,InfoT>& table);
  friend std::istream& ::operator>> <T,KEY,InfoT>(std::istream& is, 
			utilib::AbstractHashTable<T,KEY,InfoT>& table);
  friend utilib::PackBuffer& ::operator<< <T,KEY,InfoT>(utilib::PackBuffer& os,
			const utilib::AbstractHashTable<T,KEY,InfoT>& table);
  friend utilib::UnPackBuffer& ::operator>> <T,KEY,InfoT>(utilib::UnPackBuffer& is,
			utilib::AbstractHashTable<T,KEY,InfoT>& table);
  #endif
  #endif

public:

  /// The type of the hash indices.
  typedef size_t size_type;

  /// The type of a list of hash elements.
  typedef ListItem<T*>* hash_element;
  
  /// The type of a hash element list item.
  typedef ListItem<T*> _Node;
  
  /// The type of a hash element list iterator.
  typedef __LinkedList_iterator<T,T&,T*,_Node,__LinkedList_Pointer_OpClass<T&,_Node> > iterator;

  /// The type of a hash element list const_iterator.
  typedef __LinkedList_iterator<T,const T&,const T*,_Node,
		__LinkedList_Pointer_OpClass<const T&,_Node> > const_iterator;

  /// Constructor.
  explicit AbstractHashTable(const char* nameBuff = "Unnamed");

  /// Constructor
  explicit AbstractHashTable(size_type initial_size, 
					const char* nameBuff = "Unnamed");

  /// Copy constructor
  AbstractHashTable(const AbstractHashTable<T,KEY,InfoT>& table)
        { *this = table; }

  /// Copy operator=
  AbstractHashTable<T,KEY,InfoT>& operator=(const AbstractHashTable<T,KEY,InfoT>& t)
        {
	clear();
	typename utilib::LinkedList<T*>::const_iterator curr = t.data.begin();
	typename utilib::LinkedList<T*>::const_iterator end  = t.data.end();
	while (curr != end) {
	  add( (*curr)->key(), (*curr)->info());
  	  curr++;
  	  }
        return *this;
        }

  /// Destructor.
  virtual ~AbstractHashTable()
	{
	clear();
	CachedAllocator<ListItem<T*> > :: delete_unused();
	}

  /// Returns \c true if the list is empty and \c false otherwise.
  bool empty() const
	{ return (data.size() == 0); }

  /// Returns \c true if the list is empty and \c false otherwise.
  operator bool() const
	{ return (data.size() != 0); }

  /// Returns the number of elements in the hash table.
  size_type size() const
	{ return data.size(); }

  /// Returns an iterator to the beginning of the list of elements in the
  /// hash table.
  iterator begin()
	{ return iterator(data.begin()._M_node); }

  /// Returns a const_iterator to the beginning of the list of elements in the
  /// hash table.
  const_iterator begin() const
	{ return const_iterator(data.begin()._M_node); }

  /// Returns an iterator to the end of the list of elements in the
  /// hash table.
  iterator end()
	{ return data.end()._M_node; }

  /// Returns a const_iterator to the end of the list of elements in the
  /// hash table.
  const_iterator end() const
	{ return data.end()._M_node; }

  /**
   * Return an iterator of the item in the hash table with the given key.
   * The iterator equals end() if the table is empty or if the item is not in
   * the tree.
   */
  virtual iterator find(const KEY& key);

  /**
   * Return a const_iterator of the item in the hash table with the given key.
   * The iterator equals end() if the table is empty or if the item is not in
   * the tree.
   */
  virtual const_iterator find(const KEY& key) const;

  /**
   * Return a beginning and ending iterator to the bucket that contains the 
   * given key.
   */
  virtual void find(const KEY& key, const_iterator& begin, const_iterator& end);

  
  /// Returns true if key exists in the hash table.
  virtual bool exists(const KEY& key)
	{return exists_key(key);}

  #if 0
  DEPRICATED
  /// Return the ``first'' element in the hash table.
  virtual T* first() const
	{
	if (data.size() == 0) return ((T*)0);
	return data.head()->data(); 
	}
  #endif

  /**
   * Add a key to the hash table and return the hash table item 
   * that contains it.
   */
  virtual iterator add(const KEY& key, InfoT& info, bool use_cached_hash=false)
	{return insert(0,&key,&info,use_cached_hash);}

  /**
   * Remove a hash table item with the given key.
   * The status flag is \c true if the key was found and \c false
   * otherwise.
   */
  virtual iterator remove(const KEY& key, bool& status)
	{
	iterator tmp = find(key);
	if (tmp != end())
	   return extract(tmp,status);
	status = false;
	return end();
	}

  /**
   * Remove a hash table item.
   * The status flag is \c true if the item was found and \c false
   * otherwise.
   */
  virtual iterator remove(iterator& item, bool& status)
	{
        if (item == end()) {
           status = false;
	   return end();
	   }
        return extract(item,status);
        }

  /**
   * Remove a hash table item and return the item's key.
   * The status flag is \c true if the item was found and \c false
   * otherwise.
   */
  virtual iterator remove(iterator& item, KEY& key, bool& status)
        {
        if (item == end()) {
           status = false;
	   return end();
	   }
        key = item->key();
        return extract(item,status);
        }

  /// Empty the hash table
  void clear();	

  /// Print statistics about the hash table.
  void statistics(std::ostream& os);

  /// A flag that can be set to ignore duplicates in the hash table.
  bool& ignore_duplicates()
	{ return ignore_duplicates_flag; }

  /// If true, then the hash table size is prime.  Can be set by the user.
  bool using_prime_ht;

  /// Set the hash function used by this hash table.
  void set_hash_fn(size_type (*hashfn)(const KEY&,size_type))
	{curr_hashfn = hashfn;}

  /// A pointer to the current hash function.
  size_type (*curr_hashfn)(const KEY&,size_type);

#if !defined(UTILIB_NO_MEMBER_TEMPLATE_FRIENDS)
protected:
#else
public:
#endif

  /// Create a new item.
  virtual T* create_item(const KEY* key, const InfoT* info)
	{ return new T(key,info); }

  /// Hashes a key with the given table size.
  virtual size_type hash(const KEY& key, size_type table_size) const
	{
	if (curr_hashfn)
	   return (*curr_hashfn)(key,table_size);
	else
	   return hash_fn(key,table_size);
	}

  /// Add a hash table item to the tree.
  iterator insert(T* item, const KEY* key, InfoT* info, 
			bool use_cached_hash=false, const bool resize_OK=true);

  /**
   * Remove a hash table item.
   * The status flag is \c true if the item was found and \c false
   * otherwise.
   */
  virtual iterator extract(iterator& item, bool& status);

  /// Resize the hash table.
  void resize(const size_type newsize, bool auto_resize=false);

  /// Resize the hash table.
  void resize()
	{resize(0,true);}

  /// A list of pointers to actual data values.
  LinkedList<T*> data;

  /// The hash table, which points at list item elements
  BasicArray<ListItem<T*>*> table;

  /// The size of the bucket in the hash table;
  BasicArray<int> bucket_size;

  /// Resize the table if (# vals)/(table.size()) > max_load
  double max_load;

  /// Ignore duplicates that are hashed if true.
  bool ignore_duplicates_flag;

  /// A customizable way of checking if an item exists
  virtual bool exists_item(const T* item)
	{return exists_core(item->key());}

  /// A customizable way of checking if a key exists
  virtual bool exists_key(const KEY& key)
	{return exists_core(key);}
	
  /**
   * Find an item defined by a HashTableItem.
   * This method is only used internally, so it assumes that
   * the hash table item is propery 'configured'.
   */
  virtual bool exists_core(const KEY& key);

  /// The index of the last hashed item.
  size_type curr_index;

};





template <class T, class KEY, class InfoT>
AbstractHashTable<T,KEY,InfoT>::AbstractHashTable(const char* /*nameBuff*/):
  using_prime_ht(true),
  curr_hashfn(0),
  table(utilib_prime_list[0]),
  bucket_size(utilib_prime_list[0]),
  max_load(3.0),
  ignore_duplicates_flag(true)
{
bucket_size << 0;
table << (ListItem<T*>*)NULL;
}



template <class T, class KEY, class InfoT>
AbstractHashTable<T,KEY,InfoT>::AbstractHashTable(size_type initial_size, 
						const char* /*nameBuff*/) :
  using_prime_ht(true),
  curr_hashfn(0),
  table(initial_size),
  bucket_size(initial_size),
  max_load(3.0),
  ignore_duplicates_flag(true)
{
bucket_size << 0;
table << (ListItem<T*>*)NULL;
}



template <class T, class KEY, class InfoT>
bool AbstractHashTable<T,KEY,InfoT>::exists_core(const KEY& key)
{
curr_index = hash(key, table.size());
if (bucket_size[curr_index] == 0)
   return false;

hash_element curr = table[curr_index];

int i=0;
for (; curr && (i<bucket_size[curr_index]); i++) {
  if (curr->data()->compare(key) == 0)
     break;
  curr = data.next(curr);
  }

if ((curr == NULL) || (i == bucket_size[curr_index]))
   return false;
return true;
}


template <class T, class KEY, class InfoT>
typename AbstractHashTable<T,KEY,InfoT>::iterator
AbstractHashTable<T,KEY,InfoT>::find(const KEY& key)
{
curr_index = hash(key, table.size());
if (bucket_size[curr_index] == 0)
   return end();

hash_element curr = table[curr_index];

int i=0;
for (; curr && (i<bucket_size[curr_index]); i++) {
  if (curr->data()->compare(key) == 0)
     break;
  curr = data.next(curr);
  }

if ((curr == NULL) || (i == bucket_size[curr_index]))
   return end();
return iterator(curr);
}


template <class T, class KEY, class InfoT>
typename AbstractHashTable<T,KEY,InfoT>::const_iterator
AbstractHashTable<T,KEY,InfoT>::find(const KEY& key) const
{
size_type ndx = hash(key, table.size());
if (bucket_size[ndx] == 0)
   return end();

hash_element curr = table[ndx];

int i=0;
for (; curr && (i<bucket_size[ndx]); i++) {
  if (curr->data()->compare(key) == 0)
     break;
  curr = data.next(curr);
  }

if ((curr == NULL) || (i == bucket_size[ndx]))
   return end();
return const_iterator(curr);
}


template <class T, class KEY, class InfoT>
void AbstractHashTable<T,KEY,InfoT>::find(const KEY& key, 
			const_iterator& begin,
			const_iterator& end)
{
curr_index = hash(key, table.size());
if (bucket_size[curr_index] == 0) {
   begin._M_node = data.end()._M_node;
   end._M_node = data.end()._M_node;
   return;
   }

begin = const_iterator(table[curr_index]);
if (curr_index == (table.size() - 1))
   end._M_node = data.end()._M_node;
else
   end = const_iterator(table[curr_index+1]);
}



template <class T, class KEY, class InfoT>
typename AbstractHashTable<T,KEY,InfoT>::iterator
AbstractHashTable<T,KEY,InfoT>::insert(T* item, const KEY* key, 
				InfoT* info, bool use_cached_hash,
				const bool resize_OK)
{
bool resize_flag=false;
size_type index;
iterator curr;

if ( item && !(ignore_duplicates_flag && exists_item(item)) ) {
   index = (use_cached_hash ? curr_index : hash(item->key(),table.size()));
   table[index] = data.insert_value(item, table[index]);
   curr = table[index];
   bucket_size[index]++;
   }
else if ( key && !(ignore_duplicates_flag && exists_key(*key)) ) {
   item = create_item(key,info);
   if (!item)
      EXCEPTION_MNGR(runtime_error, 
                     "AbstractHashTable<T,KEY,InfoT>::insert - memory error.");
   index = (use_cached_hash ? curr_index : hash(item->key(),table.size()));
   table[index] = data.insert_value(item, table[index]);
   curr = table[index];
   bucket_size[index]++;
   }
else {
   curr = data.end()._M_node;
   }

if ( (resize_OK == true) &&
     ((resize_flag == true) || 
      (max_load < (((double)data.size())/((double)table.size())))))
   resize();

return curr;
}



template <class T, class KEY, class InfoT>
typename AbstractHashTable<T,KEY,InfoT>::iterator AbstractHashTable<T,KEY,InfoT>::extract(iterator& item, bool& status)
{
status = false;

typename LinkedList<T*>::iterator curr = data.find(&*item);
if (curr != data.end()) {
   size_type index = hash(item->key(),table.size());
   if (curr == table[index]) {
      if (bucket_size[index] > 1)
         table[index] = data.next(table[index]);
      else
         table[index] = NULL;
      }
   bucket_size[index]--;
   status = true;
   return data.erase(curr)._M_node;
   }
return item;
}



template <class T, class KEY, class InfoT>
void AbstractHashTable<T,KEY,InfoT>::statistics(std::ostream& os)
{
os << "Hash table size is " << table.size() << std::endl;
os << "Total number of objects is " << data.size() << std::endl;
os << "Table load is " << (((double)data.size())/table.size()) << std::endl;
os << "Largest bucket contains " << max(bucket_size) << " objects" << std::endl;
os << "Largest bucket is #" << argmax(bucket_size) << std::endl;
os << "Hash table histogram:" << std::endl;
char str[128];
#ifdef _MSC_VER
sprintf_s(str,128,"%10s %10s %10s %10s", "size", "buckets", "objects", "sum-pct");
#else
sprintf(str,"%10s %10s %10s %10s", "size", "buckets", "objects", "sum-pct");
#endif
os << str << std::endl;

size_type buckets = table.size();
int tnbuckets=0;
for (int j=0; j<16; j++) {
  int nbuckets=0;
  int nobjs=0;
  for (size_type i=0; i < buckets; i++) {
    if (bucket_size[i] == j) {
       nbuckets++;
       nobjs += j;
       }
  }
  tnbuckets+=nobjs;
#ifdef _MSC_VER
  sprintf_s(str,128,"%10d %10d %10d %10d", j, nbuckets, nobjs, (int)((100.0*tnbuckets)/data.size()) );
#else
  sprintf(str,"%10d %10d %10d %10d", j, nbuckets, nobjs, (int)((100.0*tnbuckets)/data.size()) );
#endif
  os << str << std::endl;
  }
{
int nbuckets=0;
int nobjs=0;
for (size_type i=0; i < buckets; i++) {
  if (bucket_size[i] >= 16) {
     nbuckets++;
     nobjs += bucket_size[i];
     }
  }
tnbuckets+=nobjs;
#ifdef _MSC_VER
sprintf_s(str,128,"%10s %10d %10d %10d", ">15", nbuckets, nobjs, (int)((100.0*tnbuckets)/data.size()) );
#else
sprintf(str,"%10s %10d %10d %10d", ">15", nbuckets, nobjs, (int)((100.0*tnbuckets)/data.size()) );
#endif
os << str << std::endl;
}
}



template <class T, class KEY, class InfoT>
void AbstractHashTable<T,KEY,InfoT>::clear()
{
bucket_size << 0;
while (data) {
  T* tmp = data.head()->data();
  delete tmp;
  data.remove(data.head());
  }
table << (ListItem<T*>*)NULL;
}


template <class T, class KEY, class InfoT>
void AbstractHashTable<T,KEY,InfoT>::resize(const size_type newsize, bool auto_resize)
{
size_type tmp = newsize;
bool flag=true;

while (flag) {
   if (auto_resize) {
      if (using_prime_ht) {
         int i=0;
         while (i < utilib_num_primes) {
           if (utilib_prime_list[i] > table.size())
              break;
           i++;
           }
         if (utilib_prime_list[i] <= table.size())
            EXCEPTION_MNGR(runtime_error,"AbstractHashTable<T,KEY,InfoT>::resize -- Cannot automatically resize the hashtable any larger.");
         tmp = utilib_prime_list[i];
         }
      else {
         tmp = table.size()*2;
         }
      }
      
   table.resize(tmp);
   table << (ListItem<T*>*)NULL;
   bucket_size.resize(tmp);
   bucket_size << 0;
   
   size_type ndata = data.size();
   for (size_type i=0; i<ndata; i++) {
     T* tmp_item=NULL;
     data.remove(tmp_item);
     if (data.size() != (ndata-1))
        EXCEPTION_MNGR(runtime_error,"Bug here - First.");
     insert(tmp_item,0,false);
     if (data.size() != ndata)
        EXCEPTION_MNGR(runtime_error,"Bug here - Second.");
     }

   //
   // We might need to loop until the table is big enough, so
   // we set the flag variable for now...
   //
   flag = false;
   auto_resize=true;
   }
}

} // namespace utilib

/// Out-stream operator for writing the contents of a hash table
template <class T, class KEY, class InfoT>
inline std::ostream& operator<<(std::ostream& os,
			const utilib::AbstractHashTable<T,KEY,InfoT>& table)
{
os << table.data.size() << std::endl;
typename utilib::LinkedList<T*>::const_iterator curr = table.data.begin();
typename utilib::LinkedList<T*>::const_iterator end  = table.data.end();
while (curr != end) {
  os << (*curr)->key();
  os << (*curr)->info();
  os << std::endl;
  curr++;
  }
return(os);
}

/// Out-stream operator for writing the contents of a hash table
template <class T, class KEY, class InfoT>
inline utilib::PackBuffer& operator<<(utilib::PackBuffer& os,
			const utilib::AbstractHashTable<T,KEY,InfoT>& table)
{
os << table.data.size();
typename utilib::LinkedList<T*>::const_iterator curr = table.data.begin();
typename utilib::LinkedList<T*>::const_iterator end  = table.data.end();
while (curr != end) {
  os << (*curr)->key();
  os << (*curr)->info();
  curr++;
  }
return(os);
}

/// In-stream operator for reading the contents of a hash table
template <class T, class KEY, class InfoT>
inline std::istream& operator>>(std::istream& is, 
			utilib::AbstractHashTable<T,KEY,InfoT>& table)
{
//
// NOTE: I'm not sure how to dynamically allocate a 'generic' item
//
#if 0
   EXCEPTION_MNGR(runtime_error, "Undefined method.");
table.clear();
size_type Size;
is >> Size;
for (size_type i=0; i<Size; i++) {
  T item;
  item.read(is);
  table.add(item.key(), item.data());
  }
#endif
return(is);
}

/// In-stream operator for reading the contents of a hash table
template <class T, class KEY, class InfoT>
inline utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is,
			utilib::AbstractHashTable<T,KEY,InfoT>& table)
{
//
// NOTE: I'm not sure how to dynamically allocate a 'generic' item
//
   EXCEPTION_MNGR(std::runtime_error, "Undefined method.");
#if 0
table.clear();
size_type Size;
is >> Size;
for (size_type i=0; i<Size; i++) {
  T item;
  item.read(is);
  table.add(item.key(),item.data());
  }
#endif
return(is);
}

#endif
