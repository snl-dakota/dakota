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
 * \file compare.h
 *
 * Comparison routines useful for sorting.
 */

#ifndef utilib_compare_h
#define utilib_compare_h

#include <utilib_config.h>

namespace utilib {

/**
 * A generic comparison function, which uses the < and > operators.
 * Returns zero is equal, negative is x<y and positive otherwise.
 */
template <class T>
int compare(const T& x, const T& y)
{
if (x<y) return -1;
if (x>y) return 1;
return 0;
}


/// A struct that is used to encapsulate the definition of the \a compare_type.
struct Comparison
{
  /// Comparison values
  enum compare_type {
	better,
	worse,
	equal
	};
};


/// A base class for utilib comparison objects
template <class Type>
struct ComparisonBase
{
  /// The argument type used for comparison
  typedef Type argument_type;
};


/// A simple comparison class
template <class Type>
struct SimpleCompare : public ComparisonBase<Type>, public Comparison
{
public:

  /// Comparison operator
  compare_type operator()(const Type& _arg1, const Type& _arg2) const
	{
	if (_arg1 < _arg2) return better;
	if (_arg1 > _arg2) return worse;
	return equal;
	}
};


/// A class that reverses the encapsulated comparison
template <class ComparisonType>
class Reverse : public ComparisonBase<typename ComparisonType::argument_type>,
		public Comparison
{
public:

  /// Constructor
  Reverse() {}

  /// Constructor
  Reverse(const ComparisonType& _compare) : compare(_compare) {}

  /// Comparison operator
  compare_type operator()(
		const typename ComparisonType::argument_type& _arg1, 
		const typename ComparisonType::argument_type& _arg2) const
	{ return compare(_arg2,_arg1); }

protected:

  /// Comparison object
  ComparisonType compare;
};

} // namespace utilib

#endif
