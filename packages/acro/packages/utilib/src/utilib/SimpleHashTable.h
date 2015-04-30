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
 * \file SimpleHashTable.h
 *
 * Defines the utilib::SimpleHashTableItem and utilib::SimpleHashTable
 * classes
 */

#ifndef utilib_SimpleHashTable_h
#define utilib_SimpleHashTable_h

#include <utilib_config.h>
#include <utilib/AbstractHashTable.h>
 
namespace utilib {

template <class T, class InfoT>
class SimpleHashTable;



/**
 * Defines the container class used for the SimpleHashTree class.
 */
template <class T, class InfoT>
class SimpleHashTableItem
{
  #if !defined(DOXYGEN)
  friend class AbstractHashTable<SimpleHashTableItem<T,InfoT>,T,InfoT>;
  #endif

public:

  /// The type of the hash indices.
  typedef size_t size_type;

  /// The type of the hash table item.
  typedef SimpleHashTableItem<T,InfoT> hash_item;

  /// The type of a list of hash elements.
  typedef ListItem<hash_item*>* hash_element;

  /// Write the key to an output stream..
  void write(std::ostream& os) const
	{ os << Key << " " << Info; }

  /// Write the key to an output stream.
  void write(PackBuffer& os) const
	{ os << Key << Info; }

  /// Write the key to an output stream.
  void read(std::istream& is)
	{ is >> Key >> Info; }

  /// Write the key to an output stream.
  void read(UnPackBuffer& is)
	{ is >> Key >> Info; }

  /// Return the key.
  T& key()
	{ return Key; }

  /// Return the key.
  const T& key() const
	{ return Key; }

  /// Return the info.
  InfoT& info()
	{ return Info; }

  /// Return the info.
  const InfoT& info() const
	{ return Info; }

  /// Compare the current key with \a key.
  int compare(const T& key) const
	{ return utilib::compare(Key,key); }

  /// The information object.
  InfoT Info;

  /// The key object.
  T Key;

  /// Constructor, which requires a key.
  SimpleHashTableItem(const T* Key_, const InfoT* info_) : Info(*info_), Key(*Key_) {}

  /// Constructor.
  SimpleHashTableItem() {}

};



/**
 * A hash table class that uses standard C/C++ data types for keys.
 * The SimpleHashTable class is derived from AbstractHashtable, which 
 * defines the basic operations of the hash table.  This particular 
 * instantiation of hash tables uses a simple data type for the key, and
 * no auxillary data is associated with the key.  This greatly simplifies the
 * definition of the hash table, but it assumes that copying the key
 * is relatively inexpensive (e.g. no memory allocation/deallocation is
 * required).
 *
 * \par
 * This template class can be instantiated with a data type for which the
 * following methods are defined:
 *  - operator<
 *  - operator>
 *  - operator==
 *  - operator<<(ostream& os)
 *
 * \sa GenericHashTable
 */
template <class T, class InfoT>
class SimpleHashTable : public AbstractHashTable<SimpleHashTableItem<T,InfoT>,T,InfoT>
{
public:

  /// The type of item stored in the hash table.
  typedef SimpleHashTableItem<T,InfoT> item_type;

  /// The iterator type for the hash table.
  typedef typename AbstractHashTable<item_type,T,InfoT>::iterator iterator;

  /// The const_iterator type for the hash table.
  typedef typename AbstractHashTable<item_type,T,InfoT>::const_iterator const_iterator;

  /// Constructor, which specifies the name for the hash table.
  explicit SimpleHashTable(const char* nameBuff = "Unnamed")
        : AbstractHashTable<SimpleHashTableItem<T,InfoT>,T,InfoT>(nameBuff) {}

  /// Constructor, which specifies the size and name for the hash table.
  explicit SimpleHashTable(size_type init_size, 
				const char* nameBuff = "Unnamed")
	: AbstractHashTable<SimpleHashTableItem<T,InfoT>,T,InfoT>(init_size, nameBuff) {}

  /// Copy constructor
  SimpleHashTable(const SimpleHashTable<T,InfoT>& table)
	{ *this = table; }

  /// Copy operator=
  SimpleHashTable<T,InfoT>& operator=(const SimpleHashTable<T,InfoT>& t)
	{
	AbstractHashTable<SimpleHashTableItem<T,InfoT>,T,InfoT>::operator=(t);
	return *this;
	}

};

} // namespace utilib

#endif
