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
 * \file TypeManager.h
 *
 * Defines the utilib::Type_Manager class.  
 * 
 */
#ifndef utilib_TypeManager_h
#define utilib_TypeManager_h

#include <utilib/Any.h>

#include <cstddef>
#include <iostream>
#include <typeinfo>
#include <string>
#include <list>
#include <map>


//
// To add more debugging information, define the macro DEBUG_TYPEMANAGER
//

#ifndef TYPEMANAGER_THROW_EXCEPTIONS
/** \brief Throw exceptions on bad casts instead of (just) returning error code
 *  NB: while this will work with wither logical or numeric truth,
 *  Doxygen will only properly document if numerical truth (i.e. !0) is
 *  used. 
 */
# define TYPEMANAGER_THROW_EXCEPTIONS 1
#endif

namespace utilib {

namespace warning {
const int Typemanager_OverrideCastFcn    = 1;
const int Typemanager_NonexistentCastFcn = 2;
}; // namespace utilib::warning
namespace error {
const int TypeManager_EmptySrc           = -200;
const int TypeManager_NoCastRoute        = -201;
const int TypeManager_NoExactCastRoute   = -202;
const int TypeManager_UnknownContext     = -203;
const int TypeManager_DuplicateContext   = -204;
const int TypeManager_NoContextCastRoute = -205;
const int TypeManager_InvalidContext     = -206;
const int TypeManager_Exception          = -207;
}; // namespace utilib::error


/// An exception class used by Type_Manager when a lexical_cast fails or DNE
class bad_lexical_cast : public std::bad_cast
{
public:
   /// Constructor
   bad_lexical_cast(const std::string& msg_) : msg(msg_) {}
   /// Destructor
   virtual ~bad_lexical_cast() throw() {}

   /// Return the exception description
   virtual const char * what() const throw()
   { return msg.c_str(); }
  
protected:
   /// The exception description
   const std::string msg;
};


/// An exception class used by Type_Manager when a registration fails
class typeManager_error : public std::runtime_error
{
public:
   /// Constructor
   typeManager_error(const std::string& msg) : runtime_error(msg) {}
};



class UntypedAnyContainer
{
public:
   UntypedAnyContainer()
      : m_data()
   {}
   explicit UntypedAnyContainer(Any& referent)
      : m_data(referent)
   {}
   UntypedAnyContainer(const UntypedAnyContainer &rhs)
      : m_data(rhs.m_data)
   {}
   UntypedAnyContainer& operator=(const UntypedAnyContainer& rhs)
   {
      m_data = rhs.m_data; 
      return *this;
   }

   bool operator==(const UntypedAnyContainer& rhs) const
   { return m_data == rhs.m_data; }
   bool operator<(const UntypedAnyContainer& rhs) const
   { return m_data < rhs.m_data; }

   Any m_data;
};

#if 0
// This was added in r1799, but I am no longer convinced this is
// actually needed
//
/// Redefine copying UntypedAnyContainer to be a deep (clone) copy
template<>
class Any::Copier<UntypedAnyContainer>
{
public:
   static const bool CopyConstructable = true;
   static void copy(UntypedAnyContainer& lhs, const UntypedAnyContainer& rhs)
   { lhs.m_data = rhs.m_data.clone(); }
};
#endif

class Type_Manager;

/// Global function for accessing single copy of Type_Manager
Type_Manager* TypeManager();

/** \brief A central class for managing & facilitating type conversions
 * 
 *  The Type_Manager class facilitates generic type conversions.  To
 *  understand why this is necessary, a brief word about casting is in
 *  order.
 * 
 *  C++ supports 4 basic types of casting: static, dynamic, const, and
 *  reinterpret.  Although static_cast is widely used for converting
 *  among types, it suffers from 2 fundamental limitations:
 *    -# the compiler must know a priori both the starting and 
 *       destination types.
 *    -# there must be a function that directly converts from the 
 *       starting type to the destination type (the compiler will not
 *       deduce chaining rules)
 *
 *  This means that it is impossible to create a non-template function
 *  that can take any user-defined data type as an argument (see
 *  utilib::Any) and convert it to the actual data type it wants to work
 *  with without knowing all possible incoming data types at compile
 *  time.  Even for the case of template functions / classes, you will
 *  need to generate a potentially large number of casting functions to
 *  facilitate the automatic conversion process.
 *
 *  Type_Manager attempts to fix this by creating a run-time casting
 *  system.  A series of casting functions (each of which take a
 *  utilib::Any and produce a utilib::Any) are registered with the
 *  Manager.  It then computes the complete reachable type space through
 *  a series of Breadth First Searches.  Later, when it is provided with
 *  a starting Any and a desired destination type, it locates and
 *  applies the sequence of casts that can convert the Any to the
 *  destination type (returning an error if the destination type is
 *  unreachable).  The benefit of this structure is that the caller NEED
 *  NOT KNOW the actual type of the starting utilib::Any.
 *
 *  Finally, a word on terminology.  The Type_Manager provides this
 *  casting functionality through a "new" <tt>lexical_cast</tt> cast.
 *
 *  <B>Lexical casts</B> provide seamless conversion among data types in
 *  the same manner as the traditional static_cast (although
 *  lexical_cast supports cast chaining).  The data stored into the
 *  destination utilib::Any should be the same in both size and value.
 *  The exception to this are cast chains that are "inexact."  An
 *  inexact cast chain is any chain that contains a step that
 *  potentially modifies the sensible value of the casted data.  As an
 *  example, casting int -> double is an example of an exact cast: the
 *  complete input space of int is representable as a double.  However,
 *  the reverse cast of double -> int is an inexact cast (5.5 will be
 *  truncated to 5).  The Type_Manager::lexical_cast() will always
 *  follow an exact cast chain if one is available.  If no exact cast
 *  chain is available, it will then follow an inexact chain (unless the
 *  force_exact parameter to lexical_cast is set to true, in which case
 *  the cast fails and an error is returned).  If no value is provided
 *  for the force_exact parameter, the Type_Manager will use its
 *  internal default (which defaults to true, but may be overridden
 *  through the setDefaultForceExact() method).
 */
class Type_Manager
{
private:
   /// Common type_info's used in the TypeManager
   struct TypeInfo {
      static const std::type_info* Void;
      static const std::type_info* Any;
      static const std::type_info* AnyRef;
      static const std::type_info* AnyFixedRef;
      static const std::type_info* UntypedAnyContainer;
   };

public:
   /// The prototype for all casting functions
   typedef int(*fptr_cast)(const Any &from, Any &to);

   ///
   Type_Manager(Type_Manager& src, bool reference);

   ///
   Type_Manager(bool registerDefaultCasts = true);

   ///
   ~Type_Manager()
   {}

   /// return the last error code thrown by Type_Manager
   int lastError() const
   { return m_lastError; }
   /// set whether Errors generate exceptions [default = true]
   void setErrorExceptions(bool value)
   { m_throwErrors = value; }
   /// set whether Warnings generate exceptions [default = false]
   void setWarningExceptions(bool value)
   { m_throwWarnings = value; }
   /// set default value for force_exact [default = true]
   void setDefaultForceExact(bool value)
   { m_forceExact = value; }
   /// get whether Errors generate exceptions
   bool errorExceptions()
   { return m_throwErrors; }
   /// get whether Warnings generate exceptions
   bool warningExceptions()
   { return m_throwWarnings; }
   /// get default value for force_exact in casts
   bool defaultForceExact()
   { return m_forceExact; }

   /// Perform a lexical_cast, converting src to the toType
   int lexical_cast( const Any src, Any &dest, const std::type_info& toType, 
                     const bool force_exact );
   /// Perform a lexical_cast, converting src to the toType
   inline 
   int lexical_cast( const Any src, Any &dest, const std::type_info& toType) 
   { return lexical_cast(src, dest, toType, m_forceExact); }

   /// Convenience function for those too lazy to declare local Any's ;-)
   /** Ideally, I would do explicit specializations for Any dests, but
    *  that seems to cause a link error (at least on cygwin).  SO, we
    *  will do the specialization here.
    */
   template<typename TYPE>
   int lexical_cast( const Any src, TYPE &dest, 
                     const bool force_exact )
   {
      type_t dest_type = getMainType(typeid(TYPE));
      if (( dest_type == TypeInfo::AnyFixedRef ) ||
          ( dest_type == TypeInfo::Any ) ||
          ( dest_type == TypeInfo::AnyRef ))
      {
         Any& d = reinterpret_cast<Any&>(dest);
         if ( d.is_immutable() )
         { return lexical_cast( src, d, d.type(), force_exact ); }
         else
         { return lexical_cast( src, d, src.type(), force_exact); }
      }

      Any tmp(dest, true, true);
      return lexical_cast(src, tmp, typeid(TYPE), force_exact);
   }

   /// Convenience for those too lazy to declare local Any's ;-)
   /** While in theory, we should be able to implement this with an Any
    *  as the first argument, some platforms (notably Solaris and AIX)
    *  have difficulty picking up implicit type conversions for
    *  template arguments.
    */
   template<typename FROM, typename TO>
   inline int lexical_cast( const FROM& src, TO &dest )
   { return lexical_cast<TO>(Any(src), dest, m_forceExact); }

   /// Register a new function for converting between from & to types
   bool register_lexical_cast( const std::type_info& fromType, 
                               const std::type_info& toType, 
                               const fptr_cast fcn,
                               const size_t cost = 0 );

   /// Remove a lexical cast from the cast table
   int clear_lexical_casts( const std::type_info& fromType, 
                            const std::type_info& toType );

   /// Delete all registered casts and reset the cast table
   void clear_lexical_casts();

   /// returns true if cast path exists
   bool lexical_castable( const std::type_info& srcType, 
                          const std::type_info& destType,
                          bool force_exact );
   /// returns true if cast path exists
   bool lexical_castable( const std::type_info& srcType, 
                          const std::type_info& destType )
   { return lexical_castable(srcType, destType, m_forceExact); }

   /// returns true if cast path exists (plus the cast length and "exactness")
   bool lexical_castable( const std::type_info& srcType, 
                          const std::type_info& destType,
                          bool &isExact, size_t &pathlength );

   /// Print a (pretty) table showing all valid lexical_casts
   void printLexicalCastingTable(std::ostream &out = std::cerr);


   /// Common warnings thrown by casting functions
   /** These are common warnings thrown by casting functions.  Note
    *  that all warnings are 2^n.  As a warning does not stop the
    *  casting process, all positive cast returns are bitwise or'ed
    *  together to produce the cast return code.
    *
    *  While casting functions are not strictly required to follow this
    *  convention (or even use these codes), I highly recommend that
    *  they do.
    */
   struct CastWarning {
      static const int UserDefined     = 1; ///< generic catch-all
      static const int ValueOutOfRange = 2; ///< value exceeded MAX / MIN value
      static const int ValueTruncated  = 4; ///< decimal value truncated
      static const int ValueLost       = 8; ///< (duplicate?) item omitted
      static const int ValueMissing    =16; ///< required data missing
   };

   /// Common errors thrown by casting functions
   struct CastError {
      static const int GeneralFailure  = -1; ///< generic catch-all
      static const int InvalidSource   = -2; ///< Input data not valid for cast
   };

public: // datatypes

   typedef const std::type_info*   type_t;

   struct CastKey {
      CastKey() : from(NULL), to(NULL) {}
      CastKey(const type_t &from_, const type_t &to_) : from(from_), to(to_) {}

      bool operator<(const CastKey& rhs) const
      { 
         if ( from < rhs.from )
            return true;
         else if ( rhs.from < from )
            return false;
         else
            return to < rhs.to;
      }
      bool operator==(const CastKey& rhs) const
      { return from == rhs.from && to == rhs.to; }

      type_t from;
      type_t to;
   };

   struct ChainLength {
      ChainLength() : cost(0), size(0) {}
      ChainLength(size_t cost_, size_t size_) : cost(cost_), size(size_) {}
      ChainLength(const ChainLength &rhs) : cost(rhs.cost), size(rhs.size) {}

      bool operator<(const ChainLength &rhs) const
      {
         if ( cost < rhs.cost )
            return true;
         else if ( rhs.cost < cost )
            return false;
         else
            return size < rhs.size;
      }
      bool operator==(const ChainLength &rhs) const
      { return cost == rhs.cost && size == rhs.size; }

      size_t cost;
      size_t size;
   };

   struct CastData {
      CastData() : fcn(NULL), cost(0) {}
      CastData(fptr_cast fcn_, size_t cost_) : fcn(fcn_), cost(cost_) {}

      fptr_cast  fcn;
      size_t     cost;
   };

   typedef std::multimap<CastKey, CastData> castFunctions_t;

   typedef std::list<castFunctions_t::iterator> fcnList_t;
   struct CastChain {
      CastChain() : chain(), length() {}

      fcnList_t  chain;
      ChainLength length;
   };

   typedef std::map<CastKey, CastChain> castTable_t;

private:

   int m_lastError;  ///< last non-zero return code generated

   bool m_lexTableRebuildNeeded; ///< indicates out-of-date lexical_cast table
 
   bool m_throwErrors;    ///< if true, errors will generate exceptions
   bool m_throwWarnings;  ///< if true, warnings will generate exceptions
   bool m_forceExact;     ///< default value for cast's force_exact parameter

   /// mapping of all known types to the "official" type_info for that type
   std::map<type_t, type_t>       equivalentTypes;
   /// mapping of mangled type name to "official" type_info for that type
   std::map<std::string, type_t>  typeByName;

   castFunctions_t  castFunctions;

   castTable_t  castTable;

   /// Get standardized (unique) type_info pointer (see detailed note)
   type_t getMainType(const std::type_info& type);
   /// Perform repeated BFS to generate the complete lexical_cast table
   void generateLexicalCastingTable();
   /// Private implementation that assumes type* returned by getMainType()
   bool lexical_castable( const std::type_info* srcType, 
                          const std::type_info* destType,
                          bool &isExact, size_t &pathlength );
   ///
   int execute_cast_chain( const Any src, Any &dest, 
                           castTable_t::iterator cast, std::string &message );
   ///
   int explore_alternate_routes( const Any src, Any &dest, type_t toType,
                                 bool force_exact, std::string &message );
};


namespace LexicalCasts {

/// A generic function for converting one STL class to another.
/** A generic function for converting one STL class to another.  BE VERY
 *  CAREFUL when using this function.  IT IS NOT SAFE in all cases.
 */
template<typename FROM, typename TO>
int cast_stl2stl(const Any &src, Any &dest)
{ 
   // The following strange syntax [src.template expose<FROM>()] is a
   // workaround to a bug in g++ < 3.4.  It appears to be compatible /
   // accepted by icc and compilers on Solaris, IRIX, and IBM platforms.
   const FROM& tmp = src.template expose<FROM>();
   dest.template set<TO>().assign(tmp.begin(), tmp.end());
   return 0;
}

/// A generic function for converting a single value into a STL container
template<typename TYPE, typename STL>
int cast_val2stl(const Any &src, Any &dest)
{ 
   // The following strange syntax [src.template expose<FROM>()] is a
   // workaround to a bug in g++ < 3.4.  It appears to be compatible /
   // accepted by icc and compilers on Solaris, IRIX, and IBM platforms.
   const TYPE& tmp = src.template expose<TYPE>();
   STL& stl = dest.template set<STL>();
   stl.insert(stl.end(), tmp);
   return 0;
}


/// A generic function for converting an STL container with size==1 to a scalar
template<typename TYPE, typename STL>
int cast_stl2val(const Any &src, Any &dest)
{ 
   // The following strange syntax [src.template expose<FROM>()] is a
   // workaround to a bug in g++ < 3.4.  It appears to be compatible /
   // accepted by icc and compilers on Solaris, IRIX, and IBM platforms.
   const STL& tmp = src.template expose<STL>();
   TYPE& ans = dest.template set<TYPE>();
   if ( tmp.empty() )
      return Type_Manager::CastWarning::ValueMissing;

   ans = *tmp.begin();
   return tmp.size() == 1 ? 0 : Type_Manager::CastWarning::ValueLost;
}


/// A generic function for converting through static_cast
template<typename FROM, typename TO>
int cast_static(const Any &src, Any &dest)
{ 
   // The following strange syntax [src.template expose<FROM>()] is a
   // workaround to a bug in g++ < 3.4.  It appears to be compatible /
   // accepted by icc and compilers on Solaris, IRIX, and IBM platforms.
   const FROM& tmp = src.template expose<FROM>();
   TO& ans = dest.template set<TO>() = static_cast<TO>(tmp);
   return ( static_cast<FROM>(ans) == tmp 
            ? 0 : Type_Manager::CastWarning::ValueTruncated );
}

} // namespace utilib::LexicalCasts

} // namespace utilib

#endif // utilib_TypeManager_h
