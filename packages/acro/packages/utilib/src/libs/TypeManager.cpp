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
 * \file TypeManager.cpp
 * \author John Siirola
 *
 * Implementation for the utilib::Type_Manager class.  
 * 
 */

#include <utilib/TypeManager.h>
#include <utilib/ReferenceCounted.h>

#include <climits>

//#define DEBUG_TYPEMANAGER 1

using std::type_info;
using std::string;
using std::cerr;
using std::endl;
using std::make_pair;
using std::pair;
using std::vector;
using std::list;

namespace utilib {

/*====================================================================
 * Static constants for checking common special types
 *===================================================================*/

const type_info* Type_Manager::TypeInfo::Void =
   TypeManager()->getMainType(typeid(void));
const type_info* Type_Manager::TypeInfo::Any =
   TypeManager()->getMainType(typeid(utilib::Any));
const type_info* Type_Manager::TypeInfo::AnyRef =
   TypeManager()->getMainType(typeid(utilib::AnyRef));
const type_info* Type_Manager::TypeInfo::AnyFixedRef =
   TypeManager()->getMainType(typeid(utilib::AnyFixedRef));
const type_info* Type_Manager::TypeInfo::UntypedAnyContainer =
   TypeManager()->getMainType(typeid(utilib::UntypedAnyContainer));

/*====================================================================
 * Global functions for accessing the single Type_Manager instantiation
 *===================================================================*/

Type_Manager* 
TypeManager()
{
   static Type_Manager global_type_manager = Type_Manager();
   return &global_type_manager;
}


/*====================================================================
 * CASTS namespace: general casting functions for standard types (e.g. STL)
 *===================================================================*/
namespace LexicalCasts {

#define REGISTER_STATIC_CAST(FROM,TO)  register_lexical_cast    \
   ( typeid(FROM), typeid(TO),                                  \
     &LexicalCasts::cast_static< FROM, TO > )

#define REGISTER_SIGNED_CAST(TYPE)  register_lexical_cast       \
   ( typeid(TYPE), typeid(unsigned TYPE),                       \
     &LexicalCasts::cast_signed< TYPE, unsigned TYPE > )

#define REGISTER_UNSIGNED_CAST(TYPE)  register_lexical_cast     \
   ( typeid(unsigned TYPE), typeid(TYPE),                       \
     &LexicalCasts::cast_signed< unsigned TYPE, TYPE > )

#define REGISTER_TEMPLATE_CAST(FROM,TO,FCN)  register_lexical_cast      \
   ( typeid(FROM), typeid(TO),                                          \
     &LexicalCasts::FCN< FROM, TO > )

#define REGISTER_VAL2STL_CAST(TYPE,STL)  register_lexical_cast  \
   ( typeid(TYPE), typeid(std::STL< TYPE >),                    \
     &LexicalCasts::cast_val2stl< TYPE, std::STL< TYPE > > )

#define REGISTER_STL2VAL_CAST(TYPE,STL)  register_lexical_cast  \
   ( typeid(std::STL< TYPE >), typeid(TYPE),                    \
     &LexicalCasts::cast_stl2val< TYPE, std::STL< TYPE > >,     \
     10 )

#define REGISTER_STL2STL_CAST(FROM,TO,TYPE)  register_lexical_cast      \
   ( typeid(std::FROM< TYPE >), typeid(std::TO< TYPE >),                \
     &LexicalCasts::cast_stl2stl< std::FROM< TYPE >, std::TO< TYPE > > )

template<typename T>
bool is_negative(const T& x)
{ return x < 0; }

template<> bool is_negative(const unsigned char&) { return false; }
template<> bool is_negative(const unsigned short&){ return false; }
template<> bool is_negative(const unsigned int&)  { return false; }
template<> bool is_negative(const unsigned long&) { return false; }


/// A generic function for converting from signed to unsigned
template<typename SRC, typename DEST>
int cast_signed(const Any &src, Any &dest)
{
   // The following strange syntax [src.template expose<FROM>()] is a
   // workaround to a bug in g++ < 3.4.  It appears to be compatible /
   // accepted by icc and compilers on Solaris, IRIX, and IBM platforms.
   const SRC& tmp = src.template expose<SRC>();
   DEST& ans = dest.template set<DEST>() = static_cast<DEST>(tmp);
   if ( is_negative(tmp) || is_negative(ans) )
   {
      ans = 0;
      return Type_Manager::CastWarning::ValueOutOfRange;
   }
   return 0;
}

/// A function to cast doubles to ints with range checking
int cast_double2int(const Any &src, Any &dest)
{
   const double& d = src.expose<double>();
   int i = dest.set<int>() = static_cast<int>(d);

   if (( d > INT_MAX ) || ( d < INT_MIN ))
      return Type_Manager::CastWarning::ValueOutOfRange;
   if ( d != static_cast<double>(i) )
      return Type_Manager::CastWarning::ValueTruncated;
   return 0;
}

/// A function to cast a vector of doubles to ints with range checking
int cast_vectorDouble2vectorInt(const Any &src, Any &dest)
{
   const vector<double>& vd = src.expose<vector<double> >();
   vector<int> &vi = dest.set<vector<int> >();

   int ans = 0;
   size_t index = 0;
   size_t end = vd.size();
   vi.reserve(end);
  
   int i;
   double d;
   for ( ; index < end; ++index )
   {
      d = vd[index];
      i = static_cast<int>(d);

      if (( d > INT_MAX ) || ( d < INT_MIN ))
         ans |= Type_Manager::CastWarning::ValueOutOfRange;
      if ( d != static_cast<double>(i) )
         ans |= Type_Manager::CastWarning::ValueTruncated;

      vi.push_back(i);
   }

   return ans;
}

/// A function to cast a char into a string
int cast_char2string(const Any &src, Any &dest)
{
   dest.set<string>().assign(1, src.expose<char>());
   return 0;
}

} // namespace utilib::LexicalCasts


namespace {

Type_Manager::ChainLength 
operator+( const Type_Manager::ChainLength &a, 
           const Type_Manager::ChainLength &b )
{
   return Type_Manager::ChainLength(a.cost + b.cost, a.size + b.size);
}

struct CastFragment {
   CastFragment( Type_Manager::castFunctions_t::iterator cast_,
                 Any value_ )
      : cast(cast_),
        visited(),
        value(value_),
        length(),
        warnings()
   {}

   CastFragment( Type_Manager::castFunctions_t::iterator cast_,
                 CastFragment &base )
      : cast(cast_),
        visited(base.visited),
        value(base.value),
        length(base.length),
        warnings(base.warnings)
   {}

   CastFragment( Type_Manager::castFunctions_t::iterator cast_,
                 CastFragment &base, Any& value_ )
      : cast(cast_),
        visited(),
        value(value_),
        length(base.length),
        warnings(base.warnings)
   { 
      // deep copy the visited set and add the new type
      *visited = *base.visited;
      visited->insert(base.cast->first.to);
      // record the cast "length" as part of the evaluated length
      length.cost += base.cast->second.cost;
      length.size++;
   }

   Type_Manager::castFunctions_t::iterator  cast;
   ReferenceCounted<std::set<Type_Manager::type_t> >  visited;
   Any  value;
   Type_Manager::ChainLength  length;
   ReferenceCounted<std::ostringstream>  warnings;
};

struct CastStatus;
typedef std::multimap<CastStatus, CastFragment> alternate_route_queue_t;

struct CastStatus {
   enum Evaluated { FINAL, EVALUATED, PENDING };

   CastStatus()
      : length(),
        errorLevel(0),
        evaluated(EVALUATED)
   {}

   CastStatus( const CastStatus &base, int error, bool final )
      : length(base.length),
        errorLevel(base.errorLevel | error),
        evaluated( final ? FINAL : EVALUATED)
   {}

   CastStatus( alternate_route_queue_t::iterator node,
               size_t cast_cost,
               Type_Manager::ChainLength route = Type_Manager::ChainLength() )
      : length( node->second.length 
                + Type_Manager::ChainLength(cast_cost,1)
                + route ),
        errorLevel(node->first.errorLevel),
        evaluated(PENDING)
   {}

   bool operator<(const CastStatus &rhs) const
   {
      if ( errorLevel < rhs.errorLevel )
         return true;
      else if ( rhs.errorLevel < errorLevel )
         return false;
      else if ( length < rhs.length )
         return true;
      else if ( rhs.length < length )
         return false;
      else
         return evaluated < rhs.evaluated;
   }

   bool operator==(const CastStatus &rhs) const
   {
      return (errorLevel == rhs.errorLevel) && (length == rhs.length) && 
         (evaluated == rhs.evaluated);
   }

   Type_Manager::ChainLength  length;
   int   errorLevel;
   Evaluated   evaluated;
};



} // namespace utilib::(local)


/*====================================================================
 * Type_Manager Public functions
 *===================================================================*/

/** Constructor.  This constructor operates in two modes: if
 *  duplicate_singleton == true (the default), the new Type_Manager will
 *  copy all contexts and casting functions (lexical and context) from
 *  the global TypeManager() singleton.  If duplicate_singleton ==
 *  false, then the new Type_Manager is populated with a standard set
 *  of default casts for C++ POD data types and STL container types.
 *
 *  In general, you want to create private Type_Manager instances by
 *  copying all casting functions from the global singleton in order to
 *  take advantage of casts that are automatically-registered by other
 *  types (including many of the Utilib ADT classes).
 */
Type_Manager::Type_Manager(Type_Manager& src, bool reference)
   : m_lastError(0),
     m_lexTableRebuildNeeded(true),
     m_throwErrors(true),
     m_throwWarnings(true),
     m_forceExact(false)
{
   // By default, we will inherit *all* registered casts and contexts
   // from the global singleton.  This allows other packages to build
   // "private" casters that can still leverage the custom casts that get
   // registered with the main caster (like STL or Utilib classes).
   if ( ! reference )
   {
      // copy the data... but let this class rebuild its own cast tables
      equivalentTypes = src.equivalentTypes;
      typeByName      = src.typeByName;
      castFunctions   = src.castFunctions;
   }
}


Type_Manager::Type_Manager(bool registerDefaultCasts)
   : m_lastError(0),
     m_lexTableRebuildNeeded(true),
     m_throwErrors(true),
     m_throwWarnings(true),
     m_forceExact(false)
{
   if ( registerDefaultCasts == false )
      return;

   // casts for static_casting between types
   REGISTER_STATIC_CAST(unsigned char, unsigned short);
   REGISTER_STATIC_CAST(unsigned short, unsigned int);
   REGISTER_STATIC_CAST(unsigned int, unsigned long);
   REGISTER_STATIC_CAST(unsigned long, double);

   REGISTER_STATIC_CAST(char, short);
   REGISTER_STATIC_CAST(short, int);
   REGISTER_STATIC_CAST(int, long);

   REGISTER_STATIC_CAST(short, float);
   REGISTER_STATIC_CAST(long, double);

   REGISTER_STATIC_CAST(float, double);

   // casts from signed <-> unsigned
   REGISTER_SIGNED_CAST(char);
   REGISTER_SIGNED_CAST(short);
   REGISTER_SIGNED_CAST(int);
   REGISTER_SIGNED_CAST(long);

   REGISTER_UNSIGNED_CAST(char);
   REGISTER_UNSIGNED_CAST(short);
   REGISTER_UNSIGNED_CAST(int);
   REGISTER_UNSIGNED_CAST(long);

   // casts for going from single values to STL containers
   REGISTER_VAL2STL_CAST(char, set);
   REGISTER_VAL2STL_CAST(bool, set);
   REGISTER_VAL2STL_CAST(short, set);
   REGISTER_VAL2STL_CAST(int, set);
   REGISTER_VAL2STL_CAST(long, set);
   REGISTER_VAL2STL_CAST(double, set);
   REGISTER_VAL2STL_CAST(float, set);

   REGISTER_VAL2STL_CAST(int, vector);
   REGISTER_VAL2STL_CAST(double, vector);
   REGISTER_VAL2STL_CAST(float, vector);

   // casts for converting among common STL containers
   REGISTER_STL2STL_CAST(list, vector, char);
   REGISTER_STL2STL_CAST(list, vector, bool);
   REGISTER_STL2STL_CAST(list, vector, short);
   REGISTER_STL2STL_CAST(list, vector, int);
   REGISTER_STL2STL_CAST(list, vector, long);
   REGISTER_STL2STL_CAST(list, vector, float);
   REGISTER_STL2STL_CAST(list, vector, double);

   REGISTER_STL2STL_CAST(vector, list, char);
   REGISTER_STL2STL_CAST(vector, list, bool);
   REGISTER_STL2STL_CAST(vector, list, short);
   REGISTER_STL2STL_CAST(vector, list, int);
   REGISTER_STL2STL_CAST(vector, list, long);
   REGISTER_STL2STL_CAST(vector, list, float);
   REGISTER_STL2STL_CAST(vector, list, double);

   REGISTER_STL2STL_CAST(set, vector, char);
   REGISTER_STL2STL_CAST(set, vector, bool);
   REGISTER_STL2STL_CAST(set, vector, short);
   REGISTER_STL2STL_CAST(set, vector, int);
   REGISTER_STL2STL_CAST(set, vector, long);
   REGISTER_STL2STL_CAST(set, vector, float);
   REGISTER_STL2STL_CAST(set, vector, double);

   // "promotion" casts for casting between types within STL containers
   REGISTER_TEMPLATE_CAST(list<bool>, list<short>, cast_stl2stl);
   REGISTER_TEMPLATE_CAST(list<short>, list<int>, cast_stl2stl);
   REGISTER_TEMPLATE_CAST(list<int>, list<long>, cast_stl2stl);
   REGISTER_TEMPLATE_CAST(list<long>, list<double>, cast_stl2stl);
   REGISTER_TEMPLATE_CAST(vector<bool>, vector<short>, cast_stl2stl);
   REGISTER_TEMPLATE_CAST(vector<short>, vector<int>, cast_stl2stl);
   REGISTER_TEMPLATE_CAST(vector<int>, vector<long>, cast_stl2stl);
   REGISTER_TEMPLATE_CAST(vector<long>, vector<double>, cast_stl2stl);

   // Right now, we are not supporting converting vector<char> into
   // vector<int>, because that can allow conversion of string ->
   // vector<int> (and ->vector<double>).
   //REGISTER_TEMPLATE_CAST(list<char>, list<int>, cast_stl2stl);
   //REGISTER_TEMPLATE_CAST(vector<char>, vector<int>, cast_stl2stl);

   // Some specialized string casts
   REGISTER_TEMPLATE_CAST(vector<char>, string, cast_stl2stl);
   REGISTER_TEMPLATE_CAST(string, vector<char>, cast_stl2stl);
   register_lexical_cast( typeid(char), typeid(string),
                          &LexicalCasts::cast_char2string );
  
   // -------------------------------------------------------------------
   // Some common "inexact" casts
   // -------------------------------------------------------------------

   // double -> int
   register_lexical_cast( typeid(double), typeid(int),
                          &LexicalCasts::cast_double2int, 1 );
   register_lexical_cast( typeid(vector<double>), typeid(vector<int>),
                          &LexicalCasts::cast_vectorDouble2vectorInt, 1 );

   // casts for going from STL containers to single values
   REGISTER_STL2VAL_CAST(char, vector);
   REGISTER_STL2VAL_CAST(bool, vector);
   REGISTER_STL2VAL_CAST(short, vector);
   REGISTER_STL2VAL_CAST(int, vector);
   REGISTER_STL2VAL_CAST(long, vector);
   REGISTER_STL2VAL_CAST(double, vector);
}



/** Perform a lexical cast.  Cast the data contained in src into the
 *  type specified by toType and return the result in dest.
 *
 *  Returns 0 on success, <0 on error, >0 on warning.
 */
int
Type_Manager::lexical_cast( const Any src, Any &dest, 
                            const type_info& toType,
                            const bool force_exact )
{
   type_t from = getMainType(src.type());
   type_t to = getMainType(toType);

   // Special case 1: no src
   if ( src.empty() )
   { 
      // Special case 1a: intentionally casting an empty Any into another
      // empty Any is OK.
      if ( ! dest.is_immutable() && to == TypeInfo::Void )
      {
         dest = src;
         return 0;
      }
       
      if ( m_throwErrors )
      { EXCEPTION_MNGR( utilib::bad_lexical_cast,
                        "Type_Manager::lexical_cast(): empty source Any" ); }
      dest.clear();
      return m_lastError = error::TypeManager_EmptySrc; 
   }

#ifdef DEBUG_TYPEMANAGER
   { cerr << "lexical_cast: " << from->name() <<" -> "<< to->name() << endl; }
#endif

   // Special case 2: the UntypedAnyContainer
   if ( to == TypeInfo::UntypedAnyContainer )
   { 
      if ( from == TypeInfo::UntypedAnyContainer )
         const_cast<UntypedAnyContainer&>(dest.expose<UntypedAnyContainer>())
            = src.expose<UntypedAnyContainer>(); 
      else
         const_cast<Any&>(dest.expose<UntypedAnyContainer>().m_data) = src;

      return 0;
   }

   // Special case 3: casting out of the UntypedAnyContainer
   if ( from == TypeInfo::UntypedAnyContainer )
   { 
      return lexical_cast(src.expose<UntypedAnyContainer>().m_data,
                          dest, toType, force_exact);
   }

   // Special case 4: no casting needed...
   if ( from == to )
   { 
      dest = src; 
      return 0;
   }

   // do we need to (re)generate the table of shortest paths?
   if ( m_lexTableRebuildNeeded )
      generateLexicalCastingTable();

   // Look up the shortest castable path
   castTable_t::iterator cast = castTable.find(CastKey(from, to));
   if ( cast == castTable.end() )
   {
      if ( m_throwErrors )
         EXCEPTION_MNGR( bad_lexical_cast,"Type_Manager::lexical_cast(): "
                         "no castable route from " << demangledName(from) 
                         << " to " << demangledName(to) );
      dest.clear();
      return m_lastError = error::TypeManager_NoCastRoute; 
   }

   if ( force_exact && cast->second.length.cost )
   { 
      if ( m_throwErrors )
         EXCEPTION_MNGR( bad_lexical_cast, "Type_Manager::lexical_cast(): "
                         "no exact cast route from " << demangledName(from)
                         << " to " << demangledName(to) );

      dest.clear();
      return m_lastError = error::TypeManager_NoExactCastRoute; 
   }


   //
   // Run the precomputed cast chain
   //
   string message = "";
   int ans = execute_cast_chain(src, dest, cast, message);
   if ( ans == 0 )
      return ans;

#if DEBUG_TYPEMANAGER
   cerr << "Stored cast chain returned " << ans 
        << "; running second chance system" << endl;
#endif

   string secondMsg = "";
   int secondAns = 
      explore_alternate_routes( src, dest, to, force_exact, secondMsg );
   if ( secondAns < 0 )
   {
      dest.clear();
      if ( ans < 0 )
      {
         secondAns = ans;
         secondMsg = message;
      }
      m_lastError = secondAns;
      if ( m_throwErrors )
         EXCEPTION_MNGR( bad_lexical_cast, "Type_Manager::lexical_cast(): "
                         "cast function error during cast from "
                         << demangledName(from) << " to " 
                         << demangledName(to) << ": " << m_lastError << endl
                         << secondMsg );
   } 
   else if ( secondAns > 0 )
   {
      m_lastError = secondAns;
      if ( m_throwWarnings )
         EXCEPTION_MNGR( bad_lexical_cast, "Type_Manager::lexical_cast(): "
                         "cast function warning during cast from "
                         << demangledName(from) << " to " << demangledName(to)
                         << ": " << m_lastError << endl << secondMsg );
   }
   return secondAns;
}


bool
Type_Manager::register_lexical_cast( const std::type_info& fromType, 
                                     const std::type_info& toType, 
                                     const fptr_cast fcn,
                                     const size_t cost )
{
   if ( fcn == NULL )
      EXCEPTION_MNGR( typeManager_error, "Type_Manager::"
                      "register_lexical_cast(): NULL cast function for cast { "
                      << demangledName(fromType) << " -> "
                      << demangledName(toType) << " }");

   type_t from = getMainType(fromType);
   type_t to   = getMainType(toType);
   if ( from == to )
      EXCEPTION_MNGR( typeManager_error, "Type_Manager::"
                      "register_lexical_cast(): cannot register cast to "
                      "the same type { " << demangledName(from) << " }" );
   //cerr << "register " << demangledName(from) << " -> " << demangledName(to)
   //     << "; cost = " << cost << endl;

   // *any* change to the cast functions requires rebuilding the chains
   m_lexTableRebuildNeeded = true;
   castFunctions.insert( make_pair(CastKey(from, to), CastData(fcn, cost)) );
   return true;
}


int
Type_Manager::clear_lexical_casts( const std::type_info& fromType, 
                                   const std::type_info& toType )
{
   type_t from = getMainType(fromType);
   type_t to   = getMainType(toType);

   int ans = 0;
   CastKey key = CastKey(from, to);
   castFunctions_t::iterator it = castFunctions.lower_bound(key);
   while ( it->first == key )
   {
      castFunctions.erase(it++);
      ans++;
   }

   // *any* change to the cast functions requires rebuilding the chains
   if ( ans )
      m_lexTableRebuildNeeded = true;
   else
   {
#ifdef DEBUG_TYPEMANAGER
      cerr << "WARNING: TypeManager: attempt to clear nonexistent lexical "
         "cast for " << demangledName(from) << " to " << demangledName(to) 
           << endl;
#endif
      if ( m_throwWarnings ) 
         EXCEPTION_MNGR( typeManager_error, "Type_Manager::"
                         "clear_lexical_cast(): attempt to clear "
                         "nonexistent cast function from " <<
                         demangledName(from) << " to " << demangledName(to) ); 
      m_lastError = warning::Typemanager_NonexistentCastFcn;
   }

   return ans;
}


void
Type_Manager::clear_lexical_casts()
{
   typeByName.clear();
   equivalentTypes.clear();
   castFunctions.clear();
   m_lexTableRebuildNeeded = true;
}


bool
Type_Manager::lexical_castable( const std::type_info& srcType, 
                                const std::type_info& destType,
                                bool    force_exact )
{
   type_t from = getMainType(srcType);
   type_t to   = getMainType(destType);

  
   bool    isExact;
   size_t  len;
   if ( ! lexical_castable(from, to, isExact, len) )
      return false;
   else
      return isExact || ( ! force_exact );
};


bool
Type_Manager::lexical_castable( const std::type_info& srcType, 
                                const std::type_info& destType,
                                bool    &isExact,
                                size_t  &pathLength)
{
   type_t from = getMainType(srcType);
   type_t to   = getMainType(destType);

   return lexical_castable(from, to, isExact, pathLength);
};



void
Type_Manager::printLexicalCastingTable(std::ostream &out)
{
   if ( m_lexTableRebuildNeeded )
      generateLexicalCastingTable();

   // field & header widths
   const size_t fw = 2 + 
      static_cast<size_t>(std::log10(static_cast<double>(typeByName.size())));
   const size_t hw = fw;

   std::map<std::string, type_t>::iterator name_it = typeByName.begin();
   std::map<std::string, type_t>::iterator name_itEnd = typeByName.end();

   bool setLeft = (( out.flags() & std::ios::left ) != 0);
   bool setRight = (( out.flags() & std::ios::right ) != 0);

   // Print out the "key"
   size_t i = 0;
   out << std::left;
   while ( name_it != name_itEnd )
   {
      out << std::setw(fw) << ++i << "= " 
          << demangledName(name_it->second) << endl;
      ++name_it;
   }

   // Print out the "table" heading
   out << endl << std::right << std::setw(hw+2) << "\\TO";
   for(size_t j = 0; j < i; out << std::setw(fw) << ++j << " ");
   out << endl << std::right << std::setw(hw+2) << "FR\\.";
   if ( i )
      out << std::string(i*(fw+1), '-');
   out << endl;

   // Print out the "table" rows
   i = 0;
   std::map<std::string, type_t>::iterator to_it;
   for( name_it = typeByName.begin(); name_it != name_itEnd; ++name_it )
   {
      out << std::setw(hw) << ++i << " |";

      for( to_it = typeByName.begin(); to_it != name_itEnd; ++to_it )
      {
         if ( name_it == to_it )
         {
            out << std::setw(fw) << 0 << " ";
            continue;
         }

         castTable_t::iterator chain_it
            = castTable.find(CastKey(name_it->second, to_it->second));
         if ( chain_it == castTable.end() )
            out << std::setw(fw) << "-" << " ";
         else
            out << std::setw(fw) << chain_it->second.length.size 
                << ( chain_it->second.length.cost ? "*" : " " );
      }
      out << endl;
   }

   if ( setLeft )
      out.setf(std::ios::left);
   if ( setRight )
      out.setf(std::ios::right);
}



/*====================================================================
 * Type_Manager Private functions
 *===================================================================*/

/** This function manages coordination between equivalent types that are
 *  instantiated in separate compilation units.  When user-defined
 *  template types are used in dynamically-linked libraries, the default
 *  behavior is for the compiler to emit the templated object code in
 *  *each* shared object.  As a result, an instance of the templated
 *  object created in one shared library will have a type_info reference
 *  that is different from an instance of the object created in a
 *  different library, even though the two objects are the same type.
 *
 *  This code makes sure that we use the same type_info* (NB, not
 *  type_info&) for ALL instances of a type.
 *
 *  Incidentally, duplicate template emissions occurs for static
 *  libraries as well, but the linker is smart enough to remove all the
 *  duplicate symbols/code.
 *
 *  Additional note: this problem is even worse for Windows DLLs, where
 *  ALL derived objects defined within a DLL must contain the code for
 *  the full class ancestry.  To make matters worse, DLLs use separate
 *  memory spaces from the main executable and other DLLs.  Unless you
 *  take special steps, you will crash the program if you have objects
 *  where a function in one DLL causes memory to be allocated, and then
 *  that memory is freed by a function in a separate space.  The most
 *  common case where this comes into play is with passing objects
 *  across DLL boundaries (which is why I mention it here).
 */
Type_Manager::type_t 
Type_Manager::getMainType(const std::type_info& type)
{
   std::map<type_t, type_t>::iterator type_it = equivalentTypes.find(&type);
   if ( type_it != equivalentTypes.end() )
      return type_it->second;

   return equivalentTypes[&type] = 
      typeByName.insert(make_pair(mangledName(type), &type)).first->second;
}


/** This generates the full table of shortest castable paths between all
 *  known types.  This algorithm uses a simple breadth-first search of
 *  the adjacency matrix (m_lexCasts).
 *
 *  I could have used the Boost Graph Library, but this functionality
 *  supports both exact (e.g. int -> double) and inexact (e.g. double ->
 *  int) casts, with a preference for exact casting routes when
 *  available.  That requires a slightly more complicated traversal
 *  algorithm that I don't think is terribly easy to implement in the
 *  BGL.
 */
void 
Type_Manager::generateLexicalCastingTable()
{
   // Clear out the old tables
   castTable.clear();

   typedef std::multimap<ChainLength, pair<type_t, CastChain> > 
      pending_t;

   for( castFunctions_t::iterator src = castFunctions.begin();
        src != castFunctions.end(); 
        src = castFunctions.lower_bound(CastKey(src->first.from + 1, 0)) )
   {
      std::set<type_t>   visited;
      pending_t  pending;
      pending.insert(make_pair( ChainLength(), 
                                make_pair(src->first.from, CastChain()) ));
      for ( pending_t::iterator node = pending.begin();
            node != pending.end();
            ++node )
      {
         type_t nodeType = node->second.first;
         if ( ! visited.insert(nodeType).second )
            continue;

         CastChain chain = node->second.second;
         if ( chain.length.size )
            castTable.insert(make_pair( CastKey(src->first.from, nodeType), 
                                        chain ));

         // ALL extended chains will increment size (which guarantees
         // that new nodes will be inserted *after* the active node)
         chain.length.size++;

         for ( castFunctions_t::iterator cast_it 
                  = castFunctions.lower_bound(CastKey(nodeType, 0));
               cast_it != castFunctions.end() && 
                  cast_it->first.from == nodeType;
               ++cast_it )
         {
            //cerr << "  " << demangledName(cast_it->first.from) << " -> " 
            //     << demangledName(cast_it->first.to) 
            //     << " [" << cast_it->second.length.cost << "]" << endl;
            if ( visited.count(cast_it->first.to) )
               continue;
            //cerr << "    OK" << endl;
            CastChain tmp = chain;
            tmp.length.cost += cast_it->second.cost;
            tmp.chain.push_back(cast_it);
            pending.insert(make_pair( tmp.length,
                                      make_pair(cast_it->first.to, tmp) ));
         }
      }
   }

#if 0
   std::set<type_t>   approxVisited;
   std::list<lNode_t> visitQ;

   lCastMatrix_t::iterator  from_it;
   lCastMap_t::iterator     to_it;
   lCastMap_t::iterator     to_itEnd;
   lCastMatrix_t::iterator  src_it = m_lexCasts.begin();
   lCastMatrix_t::iterator  src_itEnd = m_lexCasts.end();
   for ( ; src_it != src_itEnd; ++src_it )
   {
      exactVisited.clear();
      approxVisited.clear();
      visitQ.clear();

      // Perform a BFS from the src node to all nodes
      exactVisited.insert(src_it->first);
      approxVisited.insert(src_it->first);
      visitQ.push_back(src_it->first);
      // This is really a while loop, but with the 'continue's, it is
      // easier to write it as a for loop.
      for ( ; ! visitQ.empty(); visitQ.pop_front() )
      {
         lNode_t &node = visitQ.front();
         from_it = m_lexCasts.find(node.tail);
         if ( from_it == m_lexCasts.end() )
         { continue; }

         to_itEnd = from_it->second.end();
         for ( to_it = from_it->second.begin(); to_it != to_itEnd; ++to_it )
         {
            // Have we been here before?
            if ( exactVisited.count(to_it->first) != 0 )
            { continue; }

            // Is this an approx chain, and has another approx chain been here?
            lCastChain_t tmpChain(node.chain, to_it->second);
            if (( !tmpChain.exact ) && ( approxVisited.count(to_it->first) != 0 ))
            { continue; }

            m_fwdLexCast[src_it->first][to_it->first] 
               = m_revLexCast[to_it->first][src_it->first] 
               = m_lexCastChains.insert(m_lexCastChains.end(), tmpChain);
        
            if ( tmpChain.exact )
            { exactVisited.insert(to_it->first); }
            else
            { approxVisited.insert(to_it->first); }
            visitQ.push_back(lNode_t(to_it->first, tmpChain));
         }
      }
   }

#endif
   m_lexTableRebuildNeeded = false;
}


bool
Type_Manager::lexical_castable( const std::type_info* srcType, 
                                const std::type_info* destType,
                                bool    &isExact,
                                size_t  &pathLength)
{
   // Special case 2: no casting needed...
   if ( srcType == destType )
   { 
      isExact = true;
      pathLength = 0;
      return true;
   }

   // Special case 3: casting to an UntypedAnyContainer
   if ( destType == TypeInfo::UntypedAnyContainer )
   { 
      isExact = true;
      pathLength = 0;
      return true;
   }

   // Special case 4: casting from an UntypedAnyContainer
   if ( srcType == TypeInfo::UntypedAnyContainer )
   {
      // Casting out of UntypedAnyContainers is not yet supported.
      return false;
   }

   // do we need to (re)generate the table of shortest paths?
   if ( m_lexTableRebuildNeeded )
      generateLexicalCastingTable();

   castTable_t::iterator chain = castTable.find(CastKey(srcType, destType));
   if ( chain == castTable.end() )
      return false;

   isExact    = chain->second.length.cost == 0;
   pathLength = chain->second.length.size;
   return true;
}


int
Type_Manager::execute_cast_chain( const Any src, Any &dest, 
                                  castTable_t::iterator cast,
                                  std::string &message )
{
   // Execute the cast chain
   fcnList_t::iterator cast_it = cast->second.chain.begin();
   fcnList_t::iterator cast_itEnd = cast->second.chain.end();
   fcnList_t::iterator cast_lastCast = cast->second.chain.end();
   --cast_lastCast;
   Any tmpSrc;
   Any tmpDest = src;
   int ans = 0;
   int castNum = 0;
   for ( ; cast_it != cast_itEnd; ++cast_it )
   { 
      //
      // To prepare for the next iteration, we move the result
      // into tmpSrc and clear the tmpDest object.
      //
      tmpSrc.clear();  // We clear in case the incoming src was immutable.
      tmpSrc = tmpDest;
      tmpDest.clear();

      ++castNum;
      try 
      { 
         // If this is the last cast, we will cast directly into the dest
         // Any, in case that Any was immutable (this saves a copy).
         ans = (*cast_it)->second.fcn
            ( tmpSrc, ( cast_it == cast_lastCast ? dest : tmpDest ) ); 
      } 
      catch ( std::exception &e ) 
      {
         std::ostringstream msg;
         msg << "Cast function threw exception at cast " << castNum 
             << ": { " << demangledName(cast->first.from) << " -> " 
             << demangledName(cast->first.to) << " }:" << endl << e.what();
         message = msg.str();
         return error::TypeManager_Exception;
      }
      catch ( ... )
      {
         std::ostringstream msg;
         msg << "Cast function threw unknown exception at cast " << castNum 
             << ": { " << demangledName(cast->first.from) << " -> " 
             << demangledName(cast->first.to) << " }.";
         message = msg.str();
         return error::TypeManager_Exception;
      }

      if ( ans != 0 )
      {
         if ( ans < 0 )
         {
            std::ostringstream msg;
            msg << "Cast function error (" << ans << ") at cast " << castNum 
                << ": { " << demangledName(cast->first.from) << " -> " 
                << demangledName(cast->first.to) << " }";
            message = msg.str();
         }
         return ans;
      }
   }
   return ans; // always 0
}


int
Type_Manager::explore_alternate_routes( const Any src, Any &dest, 
                                        type_t toType,
                                        bool force_exact,
                                        std::string &message )
{
   alternate_route_queue_t pending;
   pending.insert( pending.end(), make_pair
                   ( CastStatus(), CastFragment(castFunctions.end(), src) ) );

   while ( ! pending.empty() )
   {
      alternate_route_queue_t::iterator node = pending.begin();
      if ( node->first.evaluated != CastStatus::PENDING )
      {
         //
         // expand the node
         //
#if DEBUG_TYPEMANAGER
         cerr << "expanding node " << demangledName(node->second.value.type());
#endif

         // If this was a successful route to the destination type, this
         // is the best we can do (because of the sorting order we use
         // for CastStatus: error then length then evaluated: first
         // evaluated route is guaranteed to be the lowest errorlevel)
         if ( node->first.evaluated == CastStatus::FINAL )
         {
            dest = node->second.value;
            message = node->second.warnings->str();
            return node->first.errorLevel;
         }

         type_t type = ( node->second.cast == castFunctions.end()
                         ? getMainType(node->second.value.type())
                         : node->second.cast->first.to );
         castFunctions_t::iterator cast
            = castFunctions.lower_bound(CastKey(type, 0));
         for ( ; cast->first.from == type; ++cast )
         {
#if DEBUG_TYPEMANAGER
            cerr << endl << "  ...testing " << demangledName(cast->first.to);
#endif
            // Honor the force_exact flag
            if ( force_exact && cast->second.cost )
               continue;
            
            // If this gets us to the destination type, then there is no
            // additional route to validate.
            if ( cast->first.to == toType )
            {
#if DEBUG_TYPEMANAGER
               cerr << " ... OK (end)";
#endif
               pending.insert
                  ( make_pair( CastStatus( node, cast->second.cost ),
                               CastFragment( cast, node->second ) ) );
               continue;
            }

            // Can we get to our destination through this type?
            castTable_t::iterator route 
               = castTable.find(CastKey(cast->first.to, toType));
            if ( route == castTable.end() )
               continue;

            // Honor the force_exact flag
            if ( force_exact && route->second.length.cost )
               continue;
            
            // Is this a loop back to a type we have already passed through?
            if ( node->second.visited->count(cast->first.to) )
               continue;

#if DEBUG_TYPEMANAGER
            cerr << " ... OK";
#endif
            pending.insert
               ( make_pair
                 ( CastStatus( node, cast->second.cost, route->second.length ),
                   CastFragment( cast, node->second ) ) );
         }
         
         // We are done with this node...
         pending.erase(node);
#if DEBUG_TYPEMANAGER
         cerr << endl << "   [done]" << endl;
#endif
      }
      else
      {
         // evaluate the node
         Any d;
         int ans;
         try {
#if DEBUG_TYPEMANAGER
            cerr << "casting from " 
                 << demangledName(node->second.cast->first.from) << " to " 
                 << demangledName(node->second.cast->first.to) << endl;
#endif
            ans = node->second.cast->second.fcn(node->second.value, d);
         } catch ( ... ) {
            // silently eat the error and give up on this route
            ans = error::TypeManager_Exception;
         }
         
         if ( ans >= 0 )
         {
            alternate_route_queue_t::iterator new_node = pending.insert
               ( node, 
                 make_pair
                 ( CastStatus( node->first, ans, 
                               node->second.cast->first.to == toType ),
                   CastFragment( castFunctions.end(), node->second, d ) ) );
            if ( ans != 0 )
            {
               new_node->second.warnings
                  = ReferenceCounted<std::ostringstream>();
               *new_node->second.warnings 
                  << node->second.warnings->str()
                  << "     cast from "
                  << demangledName(node->second.cast->first.from) << " to " 
                  << demangledName(node->second.cast->first.to) << ": " << ans
                  << endl;

            }
         }
         pending.erase(node);
      }
   }

   // No route to the destination!
   return error::TypeManager_NoCastRoute;
}

} // namespace utilib
