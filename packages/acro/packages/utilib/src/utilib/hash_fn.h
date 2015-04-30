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
 * \file hash_fn.h
 *
 * Defines commonly used hash functions
 */

#ifndef utilib_hash_fn_h
#define utilib_hash_fn_h

#include <utilib/std_headers.h>
#include <utilib/ValuedContainer.h>
#include <utilib/BasicArray.h>
#include <utilib/CharString.h>

namespace utilib {

//
// Declarations for the Bob Jenkins hash function (which is quite good!)
//
/// unsigned 4-byte quantities
typedef unsigned long int  ub4;
/// unsigned 1-byte quantities 
typedef unsigned      char ub1;
/// The Bob Jenkins hash function
ub4 bjlookup( register ub1* k, register ub4 length, register ub4 level);

/// Hash a long int
inline unsigned long int hash_bj(const long int& val, unsigned long int level=0)
	{return bjlookup((ub1*)&val,sizeof(long int),level);}

/// Hash a double
inline unsigned long int hash_bj(const double& val, unsigned long int level=0)
	{return bjlookup((ub1*)&val,sizeof(double),level);}

/// Hash an integer
inline unsigned long int hash_bj(const int& val, unsigned long int level=0)
	{return bjlookup((ub1*)&val,sizeof(int),level);}

/// Hash a vector of ints
inline unsigned long int hash_bj(const std::vector<int>& array, 
						unsigned long int level=0)
	{return bjlookup((ub1*)(&(array[0])),array.size()*sizeof(int),level);}

/// Hash a BasicArray of ints
inline unsigned long int hash_bj(const BasicArray<int>& array, 
						unsigned long int level=0)
	{return bjlookup((ub1*)(array.data()),array.size()*sizeof(int),level);}

/// Hash an array of ints
inline unsigned long int hash_bj(const int* array, const int arraylen, 
						unsigned long int level=0)
	{return bjlookup((ub1*)(array),arraylen*sizeof(int),level);}

/// Hash a vector of doubles
inline unsigned long int hash_bj(const std::vector<double>& array, 
						unsigned long int level=0)
	{return bjlookup((ub1*)(&(array[0])),array.size()*sizeof(double),level);}

/// Hash a BasicArray of doubles
inline unsigned long int hash_bj(const BasicArray<double>& array, 
						unsigned long int level=0)
	{return bjlookup((ub1*)(array.data()),array.size()*sizeof(double),level);}

/// Hash an array of doubles
inline unsigned long int hash_bj(const double* array, const int arraylen, 
						unsigned long int level=0)
	{return bjlookup((ub1*)(array),arraylen*sizeof(double),level);}



/// A default hash function declaration.
template <class T>
size_type hash_fn(const T& key, size_type table_size)
{return key.hash(table_size);}

/// Default hash function for BasicArray objects
template<class T>
inline size_type hash_fn(const BasicArray<T>& key, size_type table_size)
	{return hash_fn1(*((const BasicArray<T>*)&key),table_size);}

/// A default hash function for ValuedContainer objects
template <class V, class T>
size_type hash_fn(const ValuedContainer<V,T>& key, size_type table_size)
{return hash_fn(key.info,table_size);}



/// Hash a CharString
size_type hash_fn1(const CharString& key, size_type table_size);
/// Hash a CharString
size_type hash_fn2(const CharString& key, size_type table_size);
/// Hash a CharString
size_type hash_fn3(const CharString& key, size_type table_size);
/// Default hash function for CharString objects
template<>
inline size_type hash_fn(const CharString& key, size_type table_size)
	{return hash_fn3(key,table_size);}

/// Hash a string
size_type hash_fn3(const std::string& key, size_type table_size);
/// Default hash function for strings
template<>
inline size_type hash_fn(const std::string& key, size_type table_size)
	{return hash_fn3(key,table_size);}

/// Hash an integer
size_type hash_fn1(const int& key, size_type table_size);
/// Hash an integer
inline size_type hash_fn2(const int& key, size_type table_size)
	{return hash_bj(key) % table_size;}
/// Default hash function for integers
template<>
inline size_type hash_fn(const int& key, size_type table_size)
	{return hash_fn2(key,table_size);}

/// Hash a long integer
template<>
inline size_type hash_fn(const long int& key, size_type table_size)
	{return hash_bj(key) % table_size;}

/// Hash a double
size_type hash_fn1(const double& key, size_type table_size);
/// Hash a double
inline size_type hash_fn2(const double& key, size_type table_size)
	{return hash_bj(key) % table_size;}
/// Default hash function for doubles
template<>
inline size_type hash_fn(const double& key, size_type table_size)
	{return hash_fn2(key,table_size);}

/// Hash a BasicArray of ints
size_type hash_fn1(const BasicArray<int>& key, size_type table_size);
/// Hash a BasicArray of ints
inline size_type hash_fn2(const BasicArray<int>& key, size_type table_size)
	{return hash_bj(key) % table_size;}
/// Default hash function for BasicArray<int> objects
template<>
inline size_type hash_fn(const BasicArray<int>& key, size_type table_size)
	{return hash_fn2(key,table_size);}

/// Hash a BasicArray of doubles
size_type hash_fn1(const BasicArray<double>& key, size_type table_size);
/// Hash a BasicArray of doubles
size_type hash_fn2(const BasicArray<double>& key, size_type table_size);
/// Hash a BasicArray of doubles
inline size_type hash_fn3(const BasicArray<double>& key, size_type table_size)
	{return hash_bj(key) % table_size;}
/// Default hash function for BasicArray<doubles> objects
template<>
inline size_type hash_fn(const BasicArray<double>& key, size_type table_size)
	{return hash_fn3(key,table_size);}

/// Hash a vector of doubles
size_type hash_fn1(const std::vector<int>& key, size_type table_size);
/// Hash a vector of doubles
inline size_type hash_fn2(const std::vector<int>& key, size_type table_size)
	{return hash_bj(key) % table_size;}
/// Default hash function for std::vector<int> objects
template<>
inline size_type hash_fn(const std::vector<int>& key, size_type table_size)
	{return hash_fn2(key,table_size);}

/// Hash a vector of doubles
size_type hash_fn1(const std::vector<double>& key, size_type table_size);
/// Hash a vector of doubles
size_type hash_fn2(const std::vector<double>& key, size_type table_size);
/// Hash a vector of doubles
inline size_type hash_fn3(const std::vector<double>& key, size_type table_size)
	{return hash_bj(key) % table_size;}
/// Default hash function for std::vector<doubles> objects
template<>
inline size_type hash_fn(const std::vector<double>& key, size_type table_size)
	{return hash_fn3(key,table_size);}

} // namespace utilib

#endif
