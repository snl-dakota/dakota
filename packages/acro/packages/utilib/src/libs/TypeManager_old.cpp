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

#include <utilib/TypeManager_old.h>

#include <climits>

using std::cerr;
using std::endl;
using std::make_pair;

using utilib::Any;

namespace utilib {
namespace legacy {

/*====================================================================
 * Static constants for checking common special types
 *===================================================================*/

const std::type_info* Type_Manager::TypeInfo::Void =
   TypeManager()->getMainType(typeid(void));
const std::type_info* Type_Manager::TypeInfo::Any =
   TypeManager()->getMainType(typeid(utilib::Any));
const std::type_info* Type_Manager::TypeInfo::AnyRef =
   TypeManager()->getMainType(typeid(utilib::AnyRef));
const std::type_info* Type_Manager::TypeInfo::AnyFixedRef =
   TypeManager()->getMainType(typeid(utilib::AnyFixedRef));
const std::type_info* Type_Manager::TypeInfo::UntypedAnyContainer =
   TypeManager()->getMainType(typeid(utilib::UntypedAnyContainer));


/*====================================================================
 * Global functions for accessing the single Type_Manager instantiation
 *===================================================================*/

Type_Manager* 
TypeManager()
{
   static Type_Manager global_type_manager = Type_Manager(false);
   return &global_type_manager;
}


/*====================================================================
 * CASTS namespace: general casting functions for standard types (e.g. STL)
 *===================================================================*/
namespace LexicalCasts {

#define REGISTER_STATIC_CAST(FROM,TO)  register_lexical_cast       \
   ( typeid(FROM), typeid(TO),                                     \
     &LexicalCasts::cast_static< FROM, TO > )

#define REGISTER_SIGNED_CAST(TYPE)  register_lexical_cast          \
   ( typeid(TYPE), typeid(unsigned TYPE),                          \
     &LexicalCasts::cast_signed< TYPE, unsigned TYPE > )

#define REGISTER_UNSIGNED_CAST(TYPE)  register_lexical_cast        \
   ( typeid(unsigned TYPE), typeid(TYPE),                          \
     &LexicalCasts::cast_signed< unsigned TYPE, TYPE > )

#define REGISTER_TEMPLATE_CAST(FROM,TO,FCN)  register_lexical_cast \
   ( typeid(FROM), typeid(TO),                                     \
     &LexicalCasts::FCN< FROM, TO > )

#define REGISTER_VAL2STL_CAST(TYPE,STL)  register_lexical_cast     \
   ( typeid(TYPE), typeid(std::STL< TYPE >),                       \
     &LexicalCasts::cast_val2stl< TYPE, std::STL< TYPE > > )

#define REGISTER_STL2VAL_CAST(TYPE,STL)  register_lexical_cast     \
   ( typeid(std::STL< TYPE >), typeid(TYPE),                       \
     &LexicalCasts::cast_stl2val< TYPE, std::STL< TYPE > >,        \
     false )

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
    { return Type_Manager::CastWarning::ValueOutOfRange; }
  if ( d != static_cast<double>(i) )
    { return Type_Manager::CastWarning::ValueTruncated; }
  return 0;
  }

/// A function to cast a vector of doubles to ints with range checking
int cast_vectorDouble2vectorInt(const Any &src, Any &dest)
  {
  const std::vector<double>& vd = src.expose<std::vector<double> >();
  std::vector<int> &vi = dest.set<std::vector<int> >();

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
      { ans |= Type_Manager::CastWarning::ValueOutOfRange; }
    if ( d != static_cast<double>(i) )
      { ans |= Type_Manager::CastWarning::ValueTruncated; }

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

}; // namespace utilib::LexicalCasts


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
Type_Manager::Type_Manager(bool duplicate_singleton)
  : m_lastError(0),
    m_lexTableRebuildNeeded(true),
    m_ctxTableRebuildNeeded(true),
    m_throwErrors(true),
    m_throwWarnings(true),
    m_forceExact(true)
  {
  // By default, we will inherit *all* registered casts and contexts
  // from the global singleton.  This allows other packages to build
  // "private" casters that can still leverage the custom casts that get
  // registered with the main caster (like STL or Utilib classes).
  if ( duplicate_singleton )
    {
    Type_Manager &src = *TypeManager();

    // copy the data... but let this class rebuild its own cast tables
    m_equivalentTypes = src.m_equivalentTypes;
    m_typeByName      = src.m_typeByName;
    m_contextNames    = src.m_contextNames;
    m_lexCasts        = src.m_lexCasts;
    m_ctxCasts        = src.m_ctxCasts;
    return;
    }

  // casts for static_casting between types
  REGISTER_STATIC_CAST(unsigned char, unsigned short);
  REGISTER_STATIC_CAST(unsigned short, unsigned int);
  REGISTER_STATIC_CAST(unsigned int, unsigned long);
  REGISTER_STATIC_CAST(unsigned long, double);

  REGISTER_STATIC_CAST(char, short);
  REGISTER_STATIC_CAST(short, int);
  REGISTER_STATIC_CAST(int, long);
  REGISTER_STATIC_CAST(long, double);
  REGISTER_STATIC_CAST(short, float);

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
  REGISTER_TEMPLATE_CAST(std::list<bool>, std::list<short>, cast_stl2stl);
  REGISTER_TEMPLATE_CAST(std::list<short>, std::list<int>, cast_stl2stl);
  REGISTER_TEMPLATE_CAST(std::list<int>, std::list<long>, cast_stl2stl);
  REGISTER_TEMPLATE_CAST(std::list<long>, std::list<double>, cast_stl2stl);
  REGISTER_TEMPLATE_CAST(std::vector<bool>, std::vector<short>, cast_stl2stl);
  REGISTER_TEMPLATE_CAST(std::vector<short>, std::vector<int>, cast_stl2stl);
  REGISTER_TEMPLATE_CAST(std::vector<int>, std::vector<long>, cast_stl2stl);
  REGISTER_TEMPLATE_CAST(std::vector<long>, std::vector<double>, cast_stl2stl);

  // Right now, we are not supporting converting vector<char> into
  // vector<int>, because that can allow conversion of string ->
  // vector<int> (and ->vector<double>).
  //REGISTER_TEMPLATE_CAST(std::list<char>, std::list<int>, cast_stl2stl);
  //REGISTER_TEMPLATE_CAST(std::vector<char>, std::vector<int>, cast_stl2stl);

  // Some specialized string casts
  REGISTER_TEMPLATE_CAST(std::vector<char>, std::string, cast_stl2stl);
  REGISTER_TEMPLATE_CAST(std::string, std::vector<char>, cast_stl2stl);
  register_lexical_cast( typeid(char), typeid(std::string),
                         &LexicalCasts::cast_char2string );
  
  // -------------------------------------------------------------------
  // Some common "inexact" casts
  // -------------------------------------------------------------------

  // double -> int
  register_lexical_cast( typeid(double), typeid(int),
                         &LexicalCasts::cast_double2int, false );
  register_lexical_cast( typeid(std::vector<double>), typeid(std::vector<int>),
                         &LexicalCasts::cast_vectorDouble2vectorInt, false );

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
                            const std::type_info& toType,
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
                        "TypeManager::lexical_cast - empty source Any" ); }
    dest.clear();
    return m_lastError = error::TypeManager_EmptySrc; 
    }

#ifdef DEBUG_TYPEMANAGER
    { cerr << "lexical_cast: " << from->name() <<" -> "<< to->name() << endl; }
#endif

  // Special case 2: no casting needed...
  if ( from == to )
    { 
    dest = src; 
    return 0;
    }

  // Special case 3: the UntypedAnyContainer
  if ( to == TypeInfo::UntypedAnyContainer )
    { 
    if ( from == TypeInfo::UntypedAnyContainer )
      const_cast<Any&>(dest.expose<UntypedAnyContainer>().m_data) =
         src.expose<UntypedAnyContainer>().m_data; 
    else
      const_cast<Any&>(dest.expose<UntypedAnyContainer>().m_data) = src;

    return 0;
    }

  // Special case 4: casting out of the UntypedAnyContainer
  if ( from == TypeInfo::UntypedAnyContainer )
    { 
    return lexical_cast(src.expose<UntypedAnyContainer>().m_data,
                        dest, toType, force_exact);
    }


  // do we need to (re)generate the table of shortest paths?
  if ( m_lexTableRebuildNeeded )
    { generateLexicalCastingTable(); }

  // Look up the shortest castable path
  lexMatrix_t::iterator from_it = m_fwdLexCast.find(from);
  if ( from_it == m_fwdLexCast.end() )
    {
    if ( m_throwErrors )
      EXCEPTION_MNGR( utilib::bad_lexical_cast,
                      "TypeManager::lexical_cast - no castable route from " 
                      << demangledName(from) << " to " << demangledName(to) );
    dest.clear();
    return m_lastError = error::TypeManager_NoCastRoute; 
    }
  
  lexMap_t::iterator to_it = from_it->second.find(to);
  if ( to_it == from_it->second.end() )
    { 
    if ( m_throwErrors )
      EXCEPTION_MNGR( utilib::bad_lexical_cast, 
                      "TypeManager::lexical_cast - no castable route from "
                      << demangledName(from) << " to " << demangledName(to) );

    dest.clear();
    return m_lastError = error::TypeManager_NoCastRoute; 
    }

  if ( force_exact && ! to_it->second->exact )
    { 
    if ( m_throwErrors )
      EXCEPTION_MNGR( utilib::bad_lexical_cast,
                      "TypeManager::lexical_cast - no exact cast route from "
                      << demangledName(from) << " to " << demangledName(to) );

    dest.clear();
    return m_lastError = error::TypeManager_NoExactCastRoute; 
    }

  // Execute the cast chain
  std::stringstream warnings;
  fcnList_t::iterator cast_it = to_it->second->chain.begin();
  fcnList_t::iterator cast_itEnd = to_it->second->chain.end();
  fcnList_t::iterator cast_lastCast = to_it->second->chain.end();
  --cast_lastCast;
  Any tmpSrc;
  Any tmpDest = src;
  int ans = 0;
  int finalAns = 0;
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
      ans = (*cast_it)(tmpSrc, ( cast_it == cast_lastCast ? dest : tmpDest )); 
      } 
    catch ( std::exception &e ) 
      {
      EXCEPTION_MNGR( utilib::bad_lexical_cast,
                      "TypeManager::lexical_cast - cast function threw "
                      "exception at cast " << castNum << " during cast from "
                      << demangledName(from) << " to " << demangledName(to)
                      << ":" << std::endl << "     " << e.what() ); 
      }
    catch ( ... )
      {
      EXCEPTION_MNGR( utilib::bad_lexical_cast,
                      "TypeManager::lexical_cast - cast function threw "
                      "unknown exception at cast " << castNum << 
                      " during cast from " << demangledName(from) 
                      << " to " << demangledName(to) << "." );
      }

    if ( ans != 0 )
      {
      if ( ans < 0 )
        { 
        if ( m_throwErrors )
          EXCEPTION_MNGR( utilib::bad_lexical_cast, "TypeManager::"
                          "lexical_cast - cast function error at cast " 
                          << castNum << " during cast from "
                          << demangledName(from) << " to " << demangledName(to)
                          << ":" << ans);

        dest.clear();
        return m_lastError = ans; 
        }
      else
        { 
        finalAns |= ans; 
        warnings << "     cast from " << demangledName(tmpSrc.type()) << " to "
                 << demangledName( (cast_it == cast_lastCast ? dest : tmpDest )
                                   .type() ) << ": " << ans;
        }
      }
    }

  if ( finalAns != 0 )
    {
    m_lastError = finalAns;
    if ( m_throwWarnings )
      { EXCEPTION_MNGR( utilib::bad_lexical_cast, "TypeManager::"
                        "lexical_cast - cast function warning during cast from "
                        << demangledName(from) << " to " << demangledName(to)
                        << ":" << finalAns << endl << warnings.str() ); }
    }
  return finalAns;
  }


int
Type_Manager::register_lexical_cast( const std::type_info& fromType, 
                                     const std::type_info& toType, 
                                     const fptr_cast fcn,
                                     const bool exact )
  {
  assert(fcn!=NULL && "Type_Manager: registering NULL lexical cast function");
  type_t from = getMainType(fromType);
  type_t to   = getMainType(toType);

  // *any* change to the cast functions requires rebuilding the chains
  m_lexTableRebuildNeeded = true;
  m_ctxTableRebuildNeeded = true;

  lCastMap_t &toMap = m_lexCasts[from];

  lCastMap_t::iterator to_it = toMap.find(to);
  if ( to_it != toMap.end() )
    {
#ifdef DEBUG_TYPEMANAGER
      { cerr << "WARNING: TypeManager overriding lexical cast for "
             << demangledName(from) << " to " << demangledName(to) << endl; }
#endif
    to_it->second = lCast_t(fcn, exact);

    if ( m_throwWarnings ) 
      { EXCEPTION_MNGR( utilib::typeManager_error,
                        "TypeManager::register_lexical_cast - overriding "
                        "cast function from " << demangledName(from)
                        << " to " << demangledName(to) ); }
    return m_lastError = warning::Typemanager_OverrideCastFcn;
    }
  else
    { toMap[to] = lCast_t(fcn, exact); }

  return 0;
  }


int
Type_Manager::clear_lexical_cast( const std::type_info& fromType, 
                                  const std::type_info& toType )
{
  type_t from = getMainType(fromType);
  type_t to   = getMainType(toType);

   lCastMatrix_t::iterator fromIt = m_lexCasts.find(from);
   if ( fromIt != m_lexCasts.end() )
   {
      lCastMap_t::iterator toIt = fromIt->second.find(to);
      if ( toIt != fromIt->second.end() )
      {
         fromIt->second.erase(toIt);
         if ( fromIt->second.empty() )
            m_lexCasts.erase(fromIt);

         // *any* change to the cast functions requires rebuilding the chains
         m_lexTableRebuildNeeded = true;
         m_ctxTableRebuildNeeded = true;
         return 0;
      }
   }

#ifdef DEBUG_TYPEMANAGER
   cerr << "WARNING: TypeManager: attempt to clear nonexistent lexical cast "
      "for " << demangledName(from) << " to " << demangledName(to) << endl;
#endif

   if ( m_throwWarnings ) 
      EXCEPTION_MNGR( utilib::typeManager_error,
                      "TypeManager::clear_lexical_cast - attempt to clear "
                      "nonexistent cast function from " << demangledName(from)
                      << " to " << demangledName(to) ); 
   return m_lastError = warning::Typemanager_NonexistentCastFcn;
}


void
Type_Manager::clear_lexical_casts()
{
   m_equivalentTypes.clear();
   m_typeByName.clear();
   m_lexCasts.clear();
   m_lexTableRebuildNeeded = true;
   m_ctxTableRebuildNeeded = true;
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
    { generateLexicalCastingTable(); }

  // field & header widths
  const size_t fw 
      = 2 + (static_cast<unsigned long>(m_typeByName.size()) / 10);
  const size_t hw = ( fw > 2 ? fw : 2 );

  std::map<std::string, type_t>::iterator name_it = m_typeByName.begin();
  std::map<std::string, type_t>::iterator name_itEnd = m_typeByName.end();

  bool setLeft = (( out.flags() & std::ios::left ) != 0);
  bool setRight = (( out.flags() & std::ios::right ) != 0);

  // Print out the "key"
  int i = 1;
  out << std::left;
  while ( name_it != name_itEnd )
    {
    out << std::setw(fw) << i << "= " 
        << demangledName(name_it->second) << endl;
    ++i;
    ++name_it;
    }

  // Print out the "table" heading
  out << endl << std::right << std::setw(hw+2) << "\\TO";
  int j = 1;
  for( ; i != j; ++j )
    { out << std::setw(fw) << j << " "; }
  out << endl << std::right << std::setw(hw+2) << "FR\\.";
  std::string line(fw+1, '-');
  while ( --i > 0 )
    { out << line; }
  out << endl;

  // Print out the "table" rows
  i = 0;
  lexMatrix_t::iterator from_it;
  lexMap_t::iterator    to_it;

  std::map<std::string, type_t>::iterator name2_it;
  for( name_it = m_typeByName.begin(); name_it != name_itEnd; ++name_it )
    {
    out << std::setw(hw) << ++i << " |";

    for( name2_it = m_typeByName.begin(); name2_it != name_itEnd; ++name2_it )
      {
      if ( name_it == name2_it )
        {
        out << std::setw(fw) << 0 << " ";
        continue;
        }

      from_it = m_fwdLexCast.find(name_it->second);
      if ( from_it == m_fwdLexCast.end() )
        {
        out << std::setw(fw) << "-" << " ";
        continue;
        }

      to_it = from_it->second.find(name2_it->second);
      if ( to_it == from_it->second.end() )
        {
        out << std::setw(fw) << "-" << " ";
        continue;
        }

      out << std::setw(fw) << to_it->second->size 
          << ( to_it->second->exact ? " " : "*" );
      }
    out << endl;
    }

  if ( setLeft )
    { out.setf(std::ios::left); }
  if ( setRight )
    { out.setf(std::ios::right); }
  }



/** Get the contextID for a given name
 */
Type_Manager::context_id_type
Type_Manager::contextID(const std::string &name) const
  {
  std::map<std::string, context_id_type>::const_iterator it 
      = m_contextNames.find(name);
  return ( it == m_contextNames.end() ? 0 : it->second ); 
  }


/** Register a new context name (generates new contextID)
 *
 *  Returns new ID [>0 on success, <0 on error (duplicate name)]
 */
Type_Manager::context_id_type
Type_Manager::register_context( const std::string &name )
  {
  std::map<std::string, context_id_type>::const_iterator it
      = m_contextNames.find(name);
  if ( it == m_contextNames.end() )
    {
    int id = 1 + static_cast<int>(m_contextNames.size());
    m_contextNames[name] = id;
    return id;
    }

  // Special case, per Bill: if we are using exceptions for error
  // trapping, then do not return the error code.
  if ( m_throwErrors )
    { 
    EXCEPTION_MNGR( utilib::typeManager_error,
                    "TypeManager::register_context - duplicate context" ); 
    return 0;
    }

  return m_lastError = error::TypeManager_DuplicateContext; 
  }


/** Register a new function for converting between from & to types
 */
int
Type_Manager::register_context_cast
    ( const Type_Manager::context_id_type fromContext,
      const std::type_info& fromType, 
      const Type_Manager::context_id_type toContext,
      const std::type_info& toType, 
      const fptr_cast fcn,
      const bool exact )
  {
  assert(fcn!=NULL && "Type_Manager: registering NULL context cast function");
  if (( fromContext <= 0 ) || ( toContext <= 0 ))
    { 
    if ( m_throwErrors )
      { EXCEPTION_MNGR( utilib::typeManager_error, "TypeManager::"
                        "register_context_cast - invalid context" ); }
    return m_lastError = error::TypeManager_InvalidContext; 
    }
  int maxContext = static_cast<int>(m_contextNames.size());
  if (( fromContext > maxContext ) || ( toContext > maxContext ))
    { 
    if ( m_throwErrors )
      { EXCEPTION_MNGR( utilib::typeManager_error, "TypeManager::"
                        "register_context_cast - unknown context" ); }
    return m_lastError = error::TypeManager_UnknownContext; 
    }

  type_t from = getMainType(fromType);
  type_t to   = getMainType(toType);
  
  cCastList_t &castList =  m_ctxCasts[fromContext][toContext];

  cCastRef_t cast_it = castList.begin();
  cCastRef_t cast_itEnd = castList.end();
  for ( ; cast_it != cast_itEnd; ++cast_it )
    {
    if (( cast_it->in == from ) && ( cast_it->out == to ))
      { 
#ifdef DEBUG_TYPEMANAGER
        { cerr << "WARNING: TypeManager overriding context cast for "
               << fromContext << "[" << fromType.name() << "] -> " 
               << toContext << "[" << toType.name() << "]" << endl; }
#endif

      if ( cast_it->exact != exact )
        { m_ctxTableRebuildNeeded = true; }
      cast_it->fcn   = fcn;
      cast_it->exact = exact;

      if ( m_throwWarnings ) 
        { EXCEPTION_MNGR( utilib::typeManager_error,
                          "TypeManager::register_context_cast - "
                          "overriding cast function" ); }
      return m_lastError = warning::Typemanager_OverrideCastFcn;
      }
    }

  castList.push_back(cCast_t(from, to, fcn, exact)); 
  m_ctxTableRebuildNeeded = true;
  return 0;
  }


void
Type_Manager::printContextCastingTable(std::ostream &out)
  {
  if ( m_ctxTableRebuildNeeded )
    { generateContextCastingTable(); }

  // field & header widths
  const size_t fw 
      = 1 + (static_cast<unsigned long>(m_contextNames.size()) / 10);

  size_t nw = 0;
  std::map<std::string, type_t>::iterator nw_it = m_typeByName.begin();
  std::map<std::string, type_t>::iterator nw_itEnd = m_typeByName.end();
  for( ; nw_it != nw_itEnd; ++nw_it )
    {
    size_t tmp = nw_it->first.size();
    if ( tmp > nw )
      { nw = tmp; }
    }
    
  std::map<std::string, context_id_type>::iterator name_it
      = m_contextNames.begin();
  std::map<std::string, context_id_type>::iterator name_itEnd 
      = m_contextNames.end();

  bool setLeft = (( out.flags() & std::ios::left ) != 0);
  bool setRight = (( out.flags() & std::ios::right ) != 0);

  // Print out the "key"
  out << std::left;
  while ( name_it != name_itEnd )
    {
    out << std::setw(fw) << name_it->second << " = " << name_it->first << endl;
    ++name_it;
    }
  out << endl;

  // Print out the known casting routes
  cChainRefList_t::iterator cast_it;
  cChainRefList_t::iterator cast_itEnd;
  ctxMap_t::iterator    to_it;
  ctxMap_t::iterator    to_itEnd;
  ctxMatrix_t::iterator from_it = m_fwdCtxCast.begin();
  ctxMatrix_t::iterator from_itEnd = m_fwdCtxCast.end();
  out << std::right;
  for( ; from_it != from_itEnd; ++from_it )
    {
    to_itEnd = from_it->second.end();
    for( to_it = from_it->second.begin(); to_it != to_itEnd; ++to_it )
      {
      out << std::setw(fw) << from_it->first << " -> " 
          << std::setw(fw) << to_it->first << " : ";
      cast_it    = to_it->second.begin(); 
      cast_itEnd = to_it->second.end();
      while ( cast_it != cast_itEnd )
        {
        out << std::setw(fw) << (*cast_it)->size 
            << ( (*cast_it)->exact ? " " : "*" ) << ": " 
            << std::setw(nw) << (*cast_it)->chain.front()->in->name() << " -> "
            << (*cast_it)->chain.back()->out->name() << endl;
        if ( ++cast_it != cast_itEnd )
          { out << std::setw(2*fw+7) << " "; }
        }
      }
    }

  if ( setLeft )
    { out.setf(std::ios::left); }
  if ( setRight )
    { out.setf(std::ios::right); }
  }


/*====================================================================
 * Type_Manager Private functions
 *===================================================================*/


/** Perform a context cast.  Cast the data contained in src into the
 *  context (and optionally, the type specified by toType) and return
 *  the result in dest.
 *
 *  Returns 0 on success, <0 on error, >0 on warning.
 */
int
Type_Manager::context_cast_impl
    ( const Any &src, 
      Any &dest, 
      const Type_Manager::context_id_type fromContext, 
      const Type_Manager::context_id_type toContext, 
      const bool force_exact,
      const bool userdefType,
      const std::type_info& toType )
  {
  // Special case 1: no src
  if ( src.empty() )
    { 
    if ( m_throwErrors )
      { EXCEPTION_MNGR( utilib::bad_context_cast,
                        "TypeManager::context_cast - empty source Any" ); }
    dest.clear();
    return m_lastError = error::TypeManager_EmptySrc; 
    }

  if (( fromContext <= 0 ) || ( toContext <= 0 ))
    { 
    if ( m_throwErrors )
      { EXCEPTION_MNGR( utilib::bad_context_cast,
                        "TypeManager::context_cast - invalid context" ); }
    dest.clear();
    return m_lastError = error::TypeManager_InvalidContext; 
    }

  int maxContext = static_cast<int>(m_contextNames.size());
  if (( fromContext > maxContext ) || ( toContext > maxContext ))
    {
    if ( m_throwErrors )
      { EXCEPTION_MNGR( utilib::bad_context_cast,
                        "TypeManager::context_cast - unknown context" ); }
    dest.clear();
    return m_lastError = error::TypeManager_UnknownContext; 
    }

  type_t from = getMainType(src.type());
  type_t to = ( userdefType ? getMainType(toType) : 
                ( dest.is_immutable() ? getMainType(dest.type()) : NULL ));
#ifdef DEBUG_TYPEMANAGER
  cerr << "context_cast: " << fromContext << " [ " << from->name() <<" ] -> " 
       << toContext << (to == NULL ? "" : string(" [ ") + to->name() + " ]") 
       << endl;
#endif

  Any tmpIn = src;
  Any tmpOut;

  int ans      = 0;
  int finalAns = 0;

  // Is context_casting needed?
  if (( fromContext != toContext ) && ( fromContext > 0 ))
    {
    if ( m_ctxTableRebuildNeeded )
      { generateContextCastingTable(); }

    // Find the cast paths that connect the two contextID's
    ctxMatrix_t::iterator from_it = m_fwdCtxCast.find(fromContext);
    if ( from_it == m_fwdCtxCast.end() )
      { 
      if ( m_throwErrors )
        { EXCEPTION_MNGR( utilib::bad_context_cast,
                          "TypeManager::context_cast - no cast route" ); }
      dest.clear();
      return m_lastError = error::TypeManager_NoContextCastRoute; 
      }
    
    ctxMap_t::iterator to_it = from_it->second.find(toContext);
    if ( to_it == from_it->second.end() )
      { 
      if ( m_throwErrors )
        { EXCEPTION_MNGR( utilib::bad_context_cast,
                          "TypeManager::context_cast - no cast route" ); }
      dest.clear();
      return m_lastError = error::TypeManager_NoContextCastRoute; 
      }

    // Find the shortest specific cast that can actually connect the types
    cChainRefList_t::iterator  cast_it    = to_it->second.begin();
    cChainRefList_t::iterator  cast_itEnd = to_it->second.end();
    cChainRefList_t::iterator  cast       = cast_itEnd;
    size_t                     castLen    = 0;
    bool                       castExact  = true;
    size_t  tmpLen;
    bool    tmpExact;
    size_t  lexSize;
    bool    lexExact;
    for( ; cast_it != cast_itEnd; ++cast_it )
      {
      // does this path satisfy the exactness requirement?
      if ( force_exact && ( ! (*cast_it)->exact ) )
        { continue; }

      // Check if we can actually "connect" to this cast
      tmpLen   = (*cast_it)->size;
      tmpExact = (*cast_it)->exact;
      if ( ! lexical_castable( from, (*cast_it)->chain.front()->in, 
                               lexExact, lexSize ) )
        { continue; }
      if ( force_exact && ( ! lexExact ) )
        { continue; }
      tmpLen += lexSize;
      tmpExact = (tmpExact && lexExact);
      if ( to != NULL )
        {
        if ( ! lexical_castable( (*cast_it)->chain.back()->out, to, 
                                 lexExact, lexSize ) )
          { continue; }
        if ( force_exact && ( ! lexExact ) )
          { continue; }
        tmpLen += lexSize;
        tmpExact = (tmpExact && lexExact);
        }
      
      // Check if this cast is actually "better": valid when we didn't
      // have a valid solution, OR more exact, OR shorter for the same
      // "exactness"
      if (( cast == cast_itEnd ) || 
          ( tmpExact && ! castExact ) ||
          ( (castLen > tmpLen) && ( castExact == tmpExact ) ))
        {
        cast      = cast_it;
        castLen   = tmpLen;
        castExact = tmpExact;
        }
      }

    // did we find a cast path?
    if ( cast == cast_itEnd )
      { 
      if ( m_throwErrors )
        { EXCEPTION_MNGR( utilib::bad_context_cast,
                          "TypeManager::context_cast - no cast route" ); }
      dest.clear();
      return m_lastError = error::TypeManager_NoCastRoute; 
      }
#ifdef DEBUG_TYPEMANAGER
      { cerr << "       using: " << (*cast)->chain.front()->in->name()
             << " -> " << (*cast)->chain.back()->out->name() << endl; }
#endif
    
    // apply the cast path...
    cCastRefList_t::iterator castFcn_it = (*cast)->chain.begin();
    cCastRefList_t::iterator castFcn_itEnd = (*cast)->chain.end();
    while ( castFcn_it != castFcn_itEnd )
      {
      ans = lexical_cast(tmpIn, tmpOut, *(*castFcn_it)->in, force_exact);
      if ( ans != 0 )
        {
        // NB: not throwing an exception because the lexical_cast already did.
        if ( ans < 0 )
          { return m_lastError = ans; }
        else
          { finalAns |= ans; }
        }
      tmpIn = tmpOut;

      ans = (*castFcn_it)->fcn(tmpIn, tmpOut);
      if ( ans != 0 )
        {
        if ( ans < 0 )
          { 
          if ( m_throwErrors )
            { EXCEPTION_MNGR( utilib::bad_context_cast, "TypeManager::"
                              "context_cast - cast function error" ); }
          dest.clear();
          return m_lastError = ans; 
          }
        else
          { finalAns |= ans; }
        }
      tmpIn = tmpOut;

      ++castFcn_it;
      }
    }


  // convert the final answer into the desired output type
  if ( to != NULL )
    {
    ans = lexical_cast(tmpIn, dest, toType, force_exact);
    if ( ans != 0 )
      {
      // NB: not throwing an exception because the lexical_cast already did
      if ( ans < 0 )
        { return m_lastError = ans; }
      else
        { finalAns |= ans; }
      }

    tmpIn = tmpOut;
    }
  else
    { dest = tmpIn; }


  if ( finalAns != 0 )
    {
    m_lastError = finalAns;
    if ( m_throwWarnings ) 
      { EXCEPTION_MNGR( utilib::bad_context_cast, "TypeManager::"
                        "context_cast - cast function warning" ); }
    }
    
  return finalAns;
  }



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
  std::map<type_t, type_t>::iterator type_it = m_equivalentTypes.find(&type);
  if ( type_it != m_equivalentTypes.end() )
    { return type_it->second; }

  return m_equivalentTypes[&type] = 
     m_typeByName.insert(make_pair(mangledName(type), &type)).first->second;
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
  m_lexCastChains.clear();
  m_fwdLexCast.clear();
  m_revLexCast.clear();


  std::set<type_t>   exactVisited;
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

  m_lexTableRebuildNeeded = false;
  }


/** This generates the full table of shortest castable paths between all
 *  known contexts.  This algorithm uses a simple breadth-first search
 *  of the adjacency matrix (m_ctxCasts), along with the lexical casting
 *  table generated by generateLexicalCastingTable().
 *
 *  I suppose I could have used the Boost Graph Library... somehow.  But
 *  seeing as a valid context_cast path may require intermediate
 *  lexical_casts, I just don't see how to do it easily in Boost.
 *
 *  That said, this algorithm is not the 100% best algorithm possible.
 *  It determines the minimum number of *context_cast* function calls to
 *  get from any context (contextID + type) to any other context.
 *  Although it does tabulate the number (and exactness) of lexical_cast
 *  function calls, they do not factor into the shortest path
 *  calculation.  To do so would require using something like Dijkstra's
 *  algorithm... and IMHO, the potential to save a few function calls
 *  just isn't worth the effort.  If you disagree, go for it.
 */
void 
Type_Manager::generateContextCastingTable()
  {
  // Clear out the old tables
  m_ctxCastChains.clear();
  m_fwdCtxCast.clear();

  cCastRef_t               cast_it;
  cCastRef_t               cast_itEnd;
  cCastMap_t::iterator     to_it;
  cCastMap_t::iterator     to_itEnd;
  cCastMatrix_t::iterator  from_it;
  cCastMatrix_t::iterator  src_it = m_ctxCasts.begin();
  cCastMatrix_t::iterator  src_itEnd = m_ctxCasts.end();

  // generate the list of valid source full contexts (type + contextID)
  std::list<context_t> srcContexts;
  for( ; src_it != src_itEnd; ++src_it )
    { 
    to_itEnd = src_it->second.end();
    for( to_it = src_it->second.begin(); to_it != to_itEnd; ++to_it )
      {
      std::set<type_t> srcTypes;
      cast_itEnd = to_it->second.end();
      for( cast_it = to_it->second.begin(); cast_it != cast_itEnd; ++cast_it )
        { 
        if ( srcTypes.insert(cast_it->in).second )
          { srcContexts.push_back(context_t(src_it->first, cast_it->in)); }
        }
      }
    }
  
  std::set<context_t>  exactVisited;
  std::set<context_t>  approxVisited;
  std::list<cNode_t>   visitQ;

  // foreach source context
  for ( ; ! srcContexts.empty(); srcContexts.pop_front() )
    {
    exactVisited.clear();
    approxVisited.clear();
    visitQ.clear();

    // Perform a BFS from the src node to all nodes
    context_t src(srcContexts.front());
    exactVisited.insert(src);
    visitQ.push_back(cNode_t(src));
    // This is really a while loop, but with the 'continue's, it is
    // easier to write it as a for loop.
    for ( ; ! visitQ.empty(); visitQ.pop_front() )
      {
      cNode_t &node = visitQ.front();
      from_it = m_ctxCasts.find(node.tail.context);
      if ( from_it == m_ctxCasts.end() )
        { continue; }

      to_itEnd = from_it->second.end();
      for (to_it = from_it->second.begin(); to_it != to_itEnd; ++to_it )
        {
        cast_itEnd = to_it->second.end();
        for(cast_it = to_it->second.begin(); cast_it != cast_itEnd; ++cast_it)
          {
          // Is this a cast that starts from the src context, but was
          // not the type that started this trace?
          if (( from_it->first == src.context ) && ( cast_it->in != src.type ))
            { continue; }

          // Have we been here before?
          context_t toContext(to_it->first, cast_it->out);
          if ( exactVisited.count(toContext) != 0 )
            { continue; }

          // *can* we get here?
          size_t lexSize;
          bool   lexExact;
          if ( ! lexical_castable( node.tail.type, cast_it->in, 
                                   lexExact, lexSize ) )
            { continue; }

          // Is this an approx chain, and has another approx chain been here?
          cCastChain_t tmpChain(node.chain, cast_it, lexSize, lexExact);
          bool approxExists = ( approxVisited.count(toContext) != 0 );
          if (( !tmpChain.exact ) && ( approxExists ))
            { continue; }

          if ( approxExists )
            {
            // OK - we are replacing a shorter approximate chain with
            // this (longer) exact chain
            cChainRefList_t &chainList 
                = m_fwdCtxCast[src.context][to_it->first];
            cChainRefList_t::iterator chainList_it = chainList.begin();
            cChainRefList_t::iterator chainList_itEnd = chainList.end();
            while (( (*chainList_it)->chain.front()->in != src.type ) ||
                   ( (*chainList_it)->chain.back()->out != cast_it->out ))
              { ++chainList_it; }
            (**chainList_it) = tmpChain;
            }
          else
            {
            // New chain... just insert it
            m_fwdCtxCast[src.context][to_it->first].push_back 
                (m_ctxCastChains.insert(m_ctxCastChains.end(), tmpChain));
            }
        
          if ( tmpChain.exact )
            { exactVisited.insert(toContext); }
          else
            { approxVisited.insert(toContext); }
          visitQ.push_back(cNode_t(toContext, tmpChain));
          }
        }
      }
    }

  m_ctxTableRebuildNeeded = false;
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
    { generateLexicalCastingTable(); }

  // Look up the shortest castable path
  lexMatrix_t::iterator from_it = m_fwdLexCast.find(srcType);
  if ( from_it == m_fwdLexCast.end() )
    { return false; }
  
  lexMap_t::iterator to_it = from_it->second.find(destType);
  if ( to_it == from_it->second.end() )
    { return false; }

  isExact    = to_it->second->exact;
  pathLength = to_it->second->size;
  return true;
  }

} // namespace utilib::legacy
} // namespace utilib
