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
 * \file HashedSet.h
 *
 * Defines the utilib::HashedSet class
 */

#ifndef utilib_HashedSet_h
#define utilib_HashedSet_h

#include <utilib/SimpleHashTable.h>
#include <utilib/GenericHashTable.h>

namespace utilib {

template <class _Key, bool simple_flag>
class HashedSet;

}

#if !defined(DOXYGEN)
template <class _Key, bool simple_flag>
std::ostream& operator<<(std::ostream& os, const typename utilib::HashedSet<_Key,simple_flag>& obj);

template <class _Key, bool simple_flag>
utilib::PackBuffer& operator<<(utilib::PackBuffer& os, const utilib::HashedSet<_Key,simple_flag>& obj);

template <class _Key, bool simple_flag>
std::istream& operator>>(std::istream& is, utilib::HashedSet<_Key,simple_flag>& obj);

template <class _Key, bool simple_flag>
utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is, utilib::HashedSet<_Key,simple_flag>& obj);
#endif


namespace utilib {

/**
 * Base class for __LinkedList_iterator that a HasehdSet can use
 * as an iterator
 */
template<typename _Ref, typename _Node>
class __LinkedList_HashedSet_OpClass
{
public:

  /// Returns the value of a node in the set
  _Ref value(_Node* node) const
        { 
        if (!node)
	   EXCEPTION_MNGR(runtime_error, "Accessing an invalid iterator.");
	return node->Data->key();
	}

};


/// A base class for hashed set data types.  The simple_flag template parameter
/// is true if a simple data type is used for the hash key.
template <class _Key, bool simple_flag>
class HashedSetBase
{
public:

  #if !defined(DOXYGEN)
  ///
  typedef SimpleHashTable<_Key,char> _HT_type;

  ///
  typedef ListItem<SimpleHashTableItem<_Key,char>*> _Node;

  ///
  typedef __LinkedList_iterator<_Key,_Key&,_Key*,_Node,__LinkedList_HashedSet_OpClass<_Key&,_Node> > iterator;

  ///
  typedef __LinkedList_iterator<_Key,const _Key&,const _Key*,_Node,__LinkedList_HashedSet_OpClass<_Key&,_Node> > const_iterator;
  #endif

#if !defined(UTILIB_NO_MEMBER_TEMPLATE_FRIENDS)
protected:
#else
public:
#endif

  ///
  void write(std::ostream& os) const
	{
	os << ht.size() << std::endl;
	typename utilib::SimpleHashTable<_Key,char>::const_iterator curr = ht.begin();
	typename utilib::SimpleHashTable<_Key,char>::const_iterator end  = ht.end();
	while (curr != end) {
  	  os << curr->key();
  	  os << std::endl;
  	  curr++;
  	  }
	}

  /// The hash table used in this set.
  SimpleHashTable<_Key,char> ht;

};


/// A specialization of the base class for hashed set data types.  This
/// specialization applies to cases where the hash key is a class object.
template <class _Key>
class HashedSetBase<_Key,false>
{
public:

  #if !defined(DOXYGEN)
  ///
  typedef GenericHashTable<_Key,char> _HT_type;

  ///
  typedef ListItem<GenericHashTableItem<_Key,char>*> _Node;

  ///
  typedef __LinkedList_iterator<_Key,_Key&,_Key*,_Node,__LinkedList_HashedSet_OpClass<_Key&,_Node> > iterator;

  ///
  typedef __LinkedList_iterator<_Key,const _Key&,const _Key*,_Node,__LinkedList_HashedSet_OpClass<_Key&,_Node> > const_iterator;
  #endif

#if !defined(UTILIB_NO_MEMBER_TEMPLATE_FRIENDS)
protected:
#else
public:
#endif

  ///
  void write(std::ostream& os) const
	{
	os << ht.size() << std::endl;
	typename utilib::GenericHashTable<_Key,char>::const_iterator curr = ht.begin();
	typename utilib::GenericHashTable<_Key,char>::const_iterator end  = ht.end();
	while (curr != end) {
  	  os << curr->key();
  	  os << std::endl;
  	  curr++;
  	  }
	}

  /// The hash table used in this set
  GenericHashTable<_Key,char> ht;

};


/// A set class that uses a hash table to manage the set operations.
template <class _Key, bool simple_flag=true>
class HashedSet : public HashedSetBase<_Key,simple_flag>
{
  #if !defined(DOXYGEN)
  #if !defined(UTILIB_NO_MEMBER_TEMPLATE_FRIENDS)
  friend std::ostream& ::operator<< <_Key,simple_flag>(std::ostream& os, const utilib::HashedSet<_Key,simple_flag>& );

  friend utilib::PackBuffer& ::operator<< <_Key,simple_flag>(utilib::PackBuffer& os, const utilib::HashedSet<_Key,simple_flag>& );

  friend std::istream& ::operator>> <_Key,simple_flag>(std::istream& is, utilib::HashedSet<_Key,simple_flag>& );

  friend utilib::UnPackBuffer& ::operator>> <_Key,simple_flag>(utilib::UnPackBuffer& is, utilib::HashedSet<_Key,simple_flag>& );
  #endif
  #endif

public:

  #if !defined(DOXYGEN)
  typedef          _Key     key_type;
  typedef          _Key     value_type;
  typedef 	   HashedSetBase<_Key,simple_flag> base;
  typedef typename base::iterator iterator;
  typedef typename base::const_iterator const_iterator;
  typedef typename base::_HT_type _Rep_type;
  typedef typename _Rep_type::size_type size_type;
  #endif

  /// Constructor
  HashedSet() 
	{}

  /// Constructor
  HashedSet(const HashedSet<_Key,simple_flag>& __x) 
	{ this->ht = __x.ht; }

  /// Copy operator
  HashedSet<_Key,simple_flag>& operator=(const HashedSet<_Key,simple_flag>& __x)
  	{
    	this->ht = __x.ht;
    	return *this;
  	}

  /// Find a value and return an iterator to it
  const_iterator find(const value_type& __x) const
	{
	typename _Rep_type::const_iterator item = this->ht.find(__x);
	return item._M_node;
	}

  /// Find a value and return an iterator to it
  iterator find(const value_type& __x)
	{
	typename _Rep_type::iterator item = this->ht.find(__x);
	return item._M_node;
	}

  /// Returns true if a set contains an object
  bool contains(const value_type& __x) const
	{ return find(__x) != end(); }

  /// insert/erase
  std::pair<iterator,bool> insert(const value_type& __x)
	{
	typename _Rep_type::iterator item = this->ht.find(__x);
	if (item != this->ht.end()) {
	   iterator tmp(item._M_node);
	   return std::pair<iterator,bool>(tmp,false);
           }
	char dummy='a';
	typename _Rep_type::iterator tmpitem = this->ht.add(__x,dummy,true);
	iterator tmp(tmpitem._M_node);
        return std::pair<iterator, bool>(tmp,true);
  	}

  /// Erase the value from the set
  void erase(const key_type& __x)
	{
	typename _Rep_type::iterator item = this->ht.find(__x);
	if (item == this->ht.end())
	   EXCEPTION_MNGR(std::runtime_error, "HashedSet::erase - item is not in set");
	bool status;
	this->ht.remove(item,status);
  	}

  /// Erase the value from the set
  iterator erase(const iterator& curr)
	{
	if (curr == end())
	   EXCEPTION_MNGR(std::runtime_error, "HashedSet::erase - attempting to remove an item that is the 'end'");
	bool status;
  	typename _Rep_type::iterator tmp(curr._M_node);
	typename _Rep_type::iterator tmp_next = this->ht.remove(tmp,status);
	return tmp_next._M_node;
  	}

  /// The first value of the set
  iterator begin() { return this->ht.begin()._M_node; }

  /// The first value of the set
  const_iterator begin() const { return this->ht.begin()._M_node; }

  /// The last value of the set
  iterator end() { return this->ht.end()._M_node; }

  /// The last value of the set
  const_iterator end() const { return this->ht.end()._M_node; }

  /// The number of elements in the set
  size_type size() const { return this->ht.size(); }

  /// Returns true of the set is non-empty
  bool empty() const { return (this->ht.size() == 0); }

  /// Returns true of the set is non-empty
  operator bool() const { return (this->ht.size() != 0); }

  /// Remove all elements from the set
  void clear() { this->ht.clear(); }

};


} // namespace utilib


/// Returns true if the two hashed sets are disjoint
template <class _Key, bool simple_flag>
bool is_disjoint(const utilib::HashedSet<_Key,simple_flag>& set1, 
		 const utilib::HashedSet<_Key,simple_flag>& set2)
{
if (set2.size() < set1.size()) {
   typename utilib::HashedSet<_Key,simple_flag>::const_iterator curr = set2.begin();
   typename utilib::HashedSet<_Key,simple_flag>::const_iterator end  = set2.end();
   while (curr != end) {
     if (set1.contains(*curr)) return false;
     curr++;
     }
   }
else {
   typename utilib::HashedSet<_Key,simple_flag>::const_iterator curr = set1.begin();
   typename utilib::HashedSet<_Key,simple_flag>::const_iterator end  = set1.end();
   while (curr != end) {
     if (set2.contains(*curr)) return false;
     curr++;
     }
   }
return true;
}

/// Out-stream operator for writing the contents of a HashedSet
template <class _Key, bool simple_flag>
inline std::ostream& operator<<(std::ostream& os, const utilib::HashedSet<_Key,simple_flag>& obj)
{
obj.write(os);
return os;
}

/// Out-stream operator for writing the contents of a HashedSet
template <class _Key, bool simple_flag>
inline utilib::PackBuffer& operator<<(utilib::PackBuffer& os,
                                        const utilib::HashedSet<_Key,simple_flag>& obj)
{ os << obj.ht; return os; }

/// In-stream operator for reading the contents of a HashedSet
template <class _Key, bool simple_flag>
inline std::istream& operator>>(std::istream& is, utilib::HashedSet<_Key,simple_flag>& obj)
{
is >> obj.ht;
return is;
}

/// In-stream operator for reading the contents of a HashedSet
template <class _Key, bool simple_flag>
inline utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is,
                                        utilib::HashedSet<_Key,simple_flag>& obj)
{ is >> obj.ht; return is; }

#endif
