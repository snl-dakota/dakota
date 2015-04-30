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
 * \file LPHashTable.h
 *
 * Defines the utilib::LPHashTableItem and
 * utilib::LPHashTable classes
 */

#ifndef utilib_LPHashTable_h
#define utilib_LPHashTable_h

#include <utilib_config.h>
#include <utilib/_generic.h>
#include <utilib/_math.h>
#include <utilib/AbstractHashTable.h>
#include <utilib/SimpleHashTable.h>
#include <utilib/exception_mngr.h>
 
namespace utilib {


/**
 * An item in a LPHashTable.
 */
template <class ValueT, class InfoT>
class LPHashTableItem : public SimpleHashTableItem<ValueT,InfoT>
{

  #if !defined(DOXYGEN)
  friend class AbstractHashTable<LPHashTableItem<ValueT,InfoT>,ValueT,InfoT>;
  #endif

public:

  /// The value of the hash table item.
  ValueT array;

  /// Constructor, which requires a key.
  LPHashTableItem(const ValueT* array_, const InfoT* info_) 
		: SimpleHashTableItem<ValueT,InfoT>(array_,info_)
		{ array = this->Key; }

};


/**
 * A hash table class that defines a limited precision hash table.
 * The LPHashTable class is derived from AbstractHashTable,
 * which defines the basic operations of the hash table. The keys
 * are assumed to be classes for which the following operations are
 * defined:
 *	size_type hash(size_type tablesize, unsigned int precision) const
 *	int compare(KEY& key) const
 *	int write(ostream& os) const
 *	int read(istream& is)
 *
 * \sa AbstractHashTable
 */
template <class ValueT, class InfoT>
class LPHashTable : 
	public AbstractHashTable< LPHashTableItem<ValueT,InfoT>, ValueT, InfoT>
{
public:

  /// The type of item stored in the hash table.
  typedef LPHashTableItem<ValueT,InfoT> htitem;

  /// The base class type.
  typedef AbstractHashTable<htitem,ValueT,InfoT> baseclass;

  /// An iterator that represents the internal lists used by a hashtable.
  typedef typename baseclass::iterator iterator;

  /// An iterator that represents the internal lists used by a hashtable.
  typedef typename baseclass::const_iterator const_iterator;

  /// Constructor, which specifies the name for the hash table.
  explicit LPHashTable(const char* nameBuff = "Unnamed")
        : baseclass(nameBuff), hash_precision(6) {}

  /// Constructor, which specifies the size and name for the hash table.
  explicit LPHashTable(size_type init_size, const char* nameBuff = "Unnamed")
        : baseclass(init_size, nameBuff), hash_precision(6) {}

  /// Destructor.
  virtual ~LPHashTable() {}

  /// Find a rounded key value in the hash table.
  virtual iterator find(const ValueT& key)
	{
        d_round(key, hash_precision, 
	        const_cast<LPHashTable<ValueT,InfoT>*>(this)->tmp_array);
	return baseclass::find(tmp_array);
	}

  /// Find a rounded key value in the hash table.
  virtual const_iterator find(const ValueT& key) const
	{
        d_round(key, hash_precision, 
	        const_cast<LPHashTable<ValueT,InfoT>*>(this)->tmp_array);
	return baseclass::find(tmp_array);
	}

  /// Find the bucket for a rounded key value in the hash table.
  virtual void find(const ValueT& key, const_iterator& begin,
                              const_iterator& end)
	{
        d_round(key, hash_precision, 
	        const_cast<LPHashTable<ValueT,InfoT>*>(this)->tmp_array);
	baseclass::find(tmp_array, begin, end);
	}

  /// The precision used for rounding.
  unsigned int hash_precision;

protected:

  /// Returns true if the rounded key exists in the hash table.
  virtual bool exists_key(const ValueT& key)
	{
	d_round(key, hash_precision,
	        const_cast<LPHashTable<ValueT,InfoT>*>(this)->tmp_array);
	return exists_core(tmp_array);	
	}

  /// Create a hash table item.
  htitem* create_item(const ValueT* key, const InfoT* info)
	{
	htitem* item = baseclass::create_item(key,info);
	d_round(item->array, hash_precision, item->Key);
	return item;
	}

  /// A temporary array used to store a rounded point.
  ValueT tmp_array;

};

} // namespace utilib

#endif
