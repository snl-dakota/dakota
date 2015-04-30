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
 * \file sort.h
 *
 * Header for C++ sort routines.
 */

#ifndef utilib_sort_h
#define utilib_sort_h

#include <utilib/std_headers.h>
#include <utilib/BasicArray.h>

namespace utilib {
			
/// Sort a BasicArray using iterators
template <class T>
inline void sort(__normal_iterator<T*, BasicArray<T> > abegin,
	   	 __normal_iterator<T*, BasicArray<T> > aend)
{ std::sort(abegin.ptr(), aend.ptr()); }

/// Sort a BasicArray
template <class T, class CompareT>
inline void sort(__normal_iterator<T*, BasicArray<T> > abegin, 
	 	 __normal_iterator<T*, BasicArray<T> > aend,
		 CompareT compare)
{ std::sort(abegin.ptr(), aend.ptr(), compare); }

/// Sort a BasicArray
template <class T>
inline void sort(BasicArray<T>& array)
{ utilib::sort(array.begin(), array.end()); }

/// Sort a BasicArray
template <class T, class CompareT>
inline void sort(BasicArray<T>& array, CompareT compare)
{ utilib::sort(array.begin(), array.end(), compare); }

/// Sort a vector
template <class T>
inline void sort(std::vector<T>& array)
{ std::sort(array.begin(), array.end()); }

/// Sort a BasicArray
template <class T, class CompareT>
inline void sort(std::vector<T>& array, CompareT compare)
{ std::sort(array.begin(), array.end(), compare); }




/// Stable sort a BasicArray using iterators
template <class T>
inline void stable_sort(__normal_iterator<T*, BasicArray<T> > abegin, 
			__normal_iterator<T*, BasicArray<T> > aend)
{ std::stable_sort(abegin.ptr(), aend.ptr()); }

/// Stable sort a BasicArray
template <class T, class CompareT>
inline void stable_sort(__normal_iterator<T*, BasicArray<T> > abegin, 
			__normal_iterator<T*, BasicArray<T> > aend,
			CompareT compare)
{ std::stable_sort(abegin.ptr(), aend.ptr(), compare); }

/// Stable sort a BasicArray
template <class T>
inline void stable_sort(BasicArray<T>& array)
{ utilib::stable_sort(array.begin(), array.end()); }

/// Stable sort a BasicArray
template <class T, class CompareT>
inline void stable_sort(BasicArray<T>& array, CompareT compare)
{ utilib::stable_sort(array.begin(), array.end(), compare); }

/// Stable sort a vector
template <class T>
inline void stable_sort(std::vector<T>& array)
{ std::stable_sort(array.begin(), array.end()); }

/// Stable sort a vector
template <class T, class CompareT>
inline void stable_sort(std::vector<T>& array, CompareT compare)
{ std::stable_sort(array.begin(), array.end(), compare); }




/// Class used by the order function to compute the order of 
/// data in an array.
template <class ArrayT, class CompareT>
class OrderCompare
{
public:

  /// Constructor.
  OrderCompare(const ArrayT& array_, CompareT compare_)
	: array(array_), compare(compare_)
	{}

#if 0
  /// Constructor.
  OrderCompare(const ArrayT& array_)
	: array(array_)
	{}
#endif

  /// Copy Constructor.
  OrderCompare(const OrderCompare<ArrayT,CompareT>& comp)
	: array(comp.array), compare(comp.compare)
	{}
	
  /// Comparison operator.
  bool operator()(int a, int b)
	{ return compare(array[a],array[b]); }
 
  /// Reference array for comparison.
  const ArrayT& array;

  /// Comparison object
  CompareT compare;
};
 

/// Order an array.
template <class T, class CompareT>
void order(BasicArray<int>& ndx, const BasicArray<T>& v, CompareT compare)
{ 
ndx.resize(v.size());
for (unsigned i=0; i<v.size(); i++) ndx[i] = i;
sort(ndx, OrderCompare<BasicArray<T>,CompareT>(v,compare));
}

/// Order an array using a stable sort.
template <class T, class CompareT>
void stable_order(BasicArray<int>& ndx, const BasicArray<T>& v, 
		  CompareT compare)
{ 
ndx.resize(v.size());
for (unsigned i=0; i<v.size(); i++) ndx[i] = i;
stable_sort(ndx, OrderCompare<BasicArray<T>,CompareT>(v,compare));
}

/// Order an array.
template <class T>
void order(BasicArray<int>& ndx, const BasicArray<T>& v)
{ order(ndx,v,std::less<T>()); }

/// Order an array using a stable sort.
template <class T>
void stable_order(BasicArray<int>& ndx, const BasicArray<T>& v)
{ stable_order(ndx,v,std::less<T>()); }

/// Order an array.
template <class T, class CompareT>
void order(std::vector<int>& ndx, const std::vector<T>& v, CompareT compare)
{ 
ndx.resize(v.size());
for (unsigned i=0; i<v.size(); i++) ndx[i] = i;
sort(ndx, OrderCompare<std::vector<T>,CompareT>(v,compare));
}

/// Order an array using a stable sort.
template <class T, class CompareT>
void stable_order(std::vector<int>& ndx, const std::vector<T>& v, CompareT compare)
{ 
ndx.resize(v.size());
for (unsigned i=0; i<v.size(); i++) ndx[i] = i;
stable_sort(ndx, OrderCompare<std::vector<T>,CompareT>(v,compare));
}

/// Order an array.
template <class T>
void order(std::vector<int>& ndx, const std::vector<T>& v)
{ order(ndx,v,std::less<T>()); }

/// Order an array using a stable sort.
template <class T>
void stable_order(std::vector<int>& ndx, const std::vector<T>& v)
{ stable_order(ndx,v,std::less<T>()); }



/// Rank an array
template <class T>
inline void rank(BasicArray<int>& rvec, const BasicArray<T>& v)
{
BasicArray<int> ovec;
order(ovec, v);
rvec.resize(v.size());
for (unsigned int j=0; j<v.size(); j++)
  rvec[ ovec[j] ] = j;
}

/// Rank an array using a stable sort.
template <class T>
inline void stable_rank(BasicArray<int>& rvec, const BasicArray<T>& v)
{
BasicArray<int> ovec;
stable_order(ovec, v);
rvec.resize(v.size());
for (unsigned int j=0; j<v.size(); j++)
  rvec[ ovec[j] ] = j;
}

/// Rank an array
template <class T, class CompareT>
inline void rank(BasicArray<int>& rvec, const BasicArray<T>& v, CompareT compare)
{
BasicArray<int> ovec;
order(ovec, v, compare);
rvec.resize(v.size());
for (unsigned int j=0; j<v.size(); j++)
  rvec[ ovec[j] ] = j;
}

/// Rank an array using a stable sort.
template <class T, class CompareT>
inline void stable_rank(BasicArray<int>& rvec, const BasicArray<T>& v, 
			CompareT compare)
{
BasicArray<int> ovec;
stable_order(ovec, v, compare);
rvec.resize(v.size());
for (unsigned int j=0; j<v.size(); j++)
  rvec[ ovec[j] ] = j;
}

/// Rank an array
template <class T>
inline void rank(std::vector<int>& rvec, const std::vector<T>& v)
{
std::vector<int> ovec;
order(ovec, v);
rvec.resize(v.size());
for (unsigned int j=0; j<v.size(); j++)
  rvec[ ovec[j] ] = j;
}

/// Rank an array using a stable sort.
template <class T>
inline void stable_rank(std::vector<int>& rvec, const std::vector<T>& v)
{
std::vector<int> ovec;
stable_order(ovec, v);
rvec.resize(v.size());
for (unsigned int j=0; j<v.size(); j++)
  rvec[ ovec[j] ] = j;
}

/// Rank an array
template <class T, class CompareT>
inline void rank(std::vector<int>& rvec, std::vector<T>& v, CompareT compare)
{
std::vector<int> ovec;
order(ovec, v, compare);
rvec.resize(v.size());
for (unsigned int j=0; j<v.size(); j++)
  rvec[ ovec[j] ] = j;
}

/// Rank an array using a stable sort.
template <class T, class CompareT>
inline void stable_rank(std::vector<int>& rvec, const std::vector<T>& v, 
			CompareT compare)
{
std::vector<int> ovec;
stable_order(ovec, v, compare);
rvec.resize(v.size());
for (unsigned int j=0; j<v.size(); j++)
  rvec[ ovec[j] ] = j;
}

}

#endif
