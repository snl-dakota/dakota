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
 * \file GenericHashTable.h
 *
 * Defines the utilib::GenericHashTableItem and
 * utilib::GenericHashTable classes
 */

#ifndef utilib_GenericHashTable_h
#define utilib_GenericHashTable_h

#include <utilib_config.h>
#include <utilib/AbstractHashTable.h>
 
namespace utilib {


template <class T, class InfoT>
class GenericHashTable;



/**
 * An item in a GenericHashTable
 */
template <class T, class InfoT>
class GenericHashTableItem 
{
  friend class AbstractHashTable<GenericHashTableItem<T,InfoT>,T,InfoT>;

public:

  /// The type of the hash indeces.
  typedef size_t size_type;

  /// The type of the hash table item.
  typedef GenericHashTableItem<T,InfoT> hash_item;

  /// The type of a list of hash elements.
  typedef ListItem<hash_item*>* hash_element;

  /// Write the key to an output stream.
  void write(std::ostream& os) const
	{ Keyptr->write(os); Infoptr->write(os); }

  /// Write the key to an output stream.
  void write(PackBuffer& os) const
	{ Keyptr->write(os); Infoptr->write(os); }

  /// Write the key to an output stream.
  void read(std::istream& is)
	{ Keyptr->read(is); Infoptr->read(is); }

  /// Write the key to an output stream.
  void read(UnPackBuffer& is)
	{ Keyptr->read(is); Infoptr->read(is); }

  /// Return the key.
  T& key()
	{ return *Keyptr; }

  /// Return the item information.
  InfoT& info()
	{ return *Infoptr; }

  /// Return the key.
  const T& key() const
	{ return *Keyptr; }

  /// Return the item information.
  const InfoT& info() const
	{ return *Infoptr; }

  /// Compare the value of this item with the specified key
  int compare(const T& key) const
	{ return Keyptr->compare(key); }

  /// A pointer to a key object.
  T* Keyptr;

  /// A pointer to an information object.
  InfoT* Infoptr;

  /// Constructor, which requires a key.
  GenericHashTableItem(const T* Key_, const InfoT* info_) 
	{
	Keyptr  = const_cast<T*>(Key_);
	Infoptr = const_cast<InfoT*>(info_);
	}

  /// Constructor, which requires a key.
  GenericHashTableItem(T* Key_, InfoT* info_) 
					: Keyptr(Key_), Infoptr(info_) {}
};



/**
 * A hash table class that uses a general set of classes for keys.
 * The GenericHashTable class is derived from AbstractHashTable,
 * which defines the basic operations of the hash table. The keys
 * are assumed to be classes for which the following operations are
 * defined:
 *	size_type hash(size_type tablesize) const
 *	int compare(KEY& key) const
 *	int write(ostream& os) const
 *	int read(istream& is)
 *
 * \sa SimpleHashTable
 */
template <class T, class InfoT>
class GenericHashTable : public AbstractHashTable<GenericHashTableItem<T,InfoT>,T,InfoT>
{
public:

  /// The type of item stored in the hash table.
  typedef GenericHashTableItem<T,InfoT> item_type;
  

  /// The iterator type for the hash table.
  typedef typename AbstractHashTable<item_type,T,InfoT>::iterator iterator;

  /// The const_iterator type for the hash table.
  typedef typename AbstractHashTable<item_type,T,InfoT>::const_iterator const_iterator;

  /// Constructor, which specifies the name for the hash table.
  explicit GenericHashTable(const char* nameBuff = "Unnamed")
	: AbstractHashTable<item_type,T,InfoT>(nameBuff) {}

  /// Constructor, which specifies the size and name for the hash table.
  explicit GenericHashTable(size_type init_size, 
					const char* nameBuff = "Unnamed")
	: AbstractHashTable<item_type,T,InfoT>(init_size, nameBuff) {}

  /// Destructor.
  virtual ~GenericHashTable() {}

};

} // namespace utilib

#endif
