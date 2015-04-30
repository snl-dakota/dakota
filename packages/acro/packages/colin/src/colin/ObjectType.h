/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file ObjectType.h
 */

#ifndef colin_ObjectType_h
#define colin_ObjectType_h

#include <acro_config.h>
#include <utilib/exception_mngr.h>
#include <utilib/Any.h>

namespace colin
{

/** Helper class for managing object inheritance classes.  This
 *  class provides a set of template methods to ease getting the
 *  typeinfo for an intermediate class within the inheritance hierarchy
 *  and is necessary for managing the callback system.
 *
 *  Astute observers will note that this class is technically not safe
 *  when used with shared libraries (see utilib::Any::is_type()).
 *  However, as it is only ever referenced within the object that
 *  created it, we shouldn't have to worry about dynamic libraries.
 */
class ObjectType
{
public:

   /// Create an ObjectType object for a type specified by the template.
   template<typename T>
   static ObjectType get()
   { 
      if ( ObjectType::is_pointer<T>::value )
         return ObjectType(&typeid(T)); 
      else
         return ObjectType(&typeid(T*)); 
   }

   /// Create an ObjectType object for a type specified by the method argument.
   /// WEH: why perform the void cast?
   template<typename T>
   static ObjectType get(const T& ref)
   { 
      static_cast<void>(ref);
      return get<T>();
   }

   /// Return the type_info for this instance.
   const std::type_info* type() const
   { return data; }

   /// Return the demangled name of this type
   std::string name() const
   { return utilib::demangledName(data); }

   /// Return the mangledName of this type
   std::string mangledName() const
   { return utilib::mangledName(data); }

   /// Comparison operator
   bool operator<(const ObjectType& rhs) const
   { 
#ifdef _MSC_VER
      // NB: This silences a warning in MSVC it returns int and 
      // not the standard bool.
      return data->before(*rhs.data) != 0;
#else
      return data->before(*rhs.data);
#endif
   }

   /// Equals operator
   bool operator==(const ObjectType& rhs) const
   { return *data == *rhs.data; }

   /// Not equals operator
   bool operator!=(const ObjectType& rhs) const
   { return *data != *rhs.data; }

private:

   /// Returns false since this type is not a pointer
   template <typename T> 
   struct is_pointer {
      static const bool value = false; 
   };

   /// Returns true since this type is a pointer
   template <typename T> 
   struct is_pointer<T*> {
      static const bool value = true; 
   };

   /// Constructor, which initializes the type information
   ObjectType(const std::type_info* t)
      : data(t) 
   {}

   /// The type information managed by this class.
   const std::type_info* data;
};

} // namespace colin

namespace utilib {

DEFINE_DEFAULT_ANY_COMPARATOR(colin::ObjectType);

} // namespace utilib

#endif // defined colin_ObjectType_h

