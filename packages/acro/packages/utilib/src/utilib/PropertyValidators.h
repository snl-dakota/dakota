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
 * \file PropertyValidators.h
 * 
 * Defines common validators for use with the utilib::Property class.
 */

#ifndef utilib_PropertyValidators_h
#define utilib_PropertyValidators_h

#include <utilib/Property.h>

namespace utilib {
namespace PropertyValidators {

template<typename T>
bool Nonnegative( const ReadOnly_Property&, const Any& new_value )
{
   Any tmp;
   utilib::TypeManager()->lexical_cast(new_value, tmp, typeid(T));
   return ! ( tmp.template expose<T>() < 0 );
}

template<typename T>
bool Negative( const ReadOnly_Property&, const Any& new_value )
{
   Any tmp;
   utilib::TypeManager()->lexical_cast(new_value, tmp, typeid(T));
   return tmp.expose<T>() < 0;
}


template<typename T>
bool Positive( const ReadOnly_Property&, const Any& new_value )
{
   Any tmp;
   utilib::TypeManager()->lexical_cast(new_value, tmp, typeid(T));
   return 0 < tmp.expose<T>();
}


template<typename T>
class LowerBound
{
public:
   LowerBound(T value_) : value(value_) {}

   bool operator()( const ReadOnly_Property&, const Any& new_value )
   {
      Any tmp;
      utilib::TypeManager()->lexical_cast(new_value, tmp, typeid(T));
      return ! ( tmp.expose<T> < value );
   }
private:
   T value;
};


template<typename T>
class UpperBound
{
public:
   UpperBound(T value_) : value(value_) {}

   bool operator()( const ReadOnly_Property&, const Any& new_value )
   {
      Any tmp;
      utilib::TypeManager()->lexical_cast(new_value, tmp, typeid(T));
      return ! ( value < tmp.expose<T> );
   }
private:
   T value;
};


template<typename T>
class Bounds
{
public:
   Bounds(T lower_, T upper_) : lower(lower_), upper(upper_) {}

   bool operator()( const ReadOnly_Property&, const Any& new_value )
   {
      Any tmp;
      utilib::TypeManager()->lexical_cast(new_value, tmp, typeid(T));
      return ! ( upper < tmp.expose<T> || tmp.expose<T>() < lower );
   }
private:
   T lower;
   T upper;
};


} // namespace PropertyValidators
} // namespace utilib

#endif // defined utilib_PropertyValidators_h
