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
#ifndef utilib_TypeManager_old_h
#define utilib_TypeManager_old_h

#include <utilib/Any.h>
#include <utilib/TypeManager.h>

#include <cstddef>
#include <iostream>
#include <typeinfo>
#include <string>
#include <list>
#include <map>

namespace utilib {

/// An exception class used by Type_Manager when a lexical_cast fails or DNE
class bad_context_cast : public std::bad_cast
{
public:
  /// Constructor
  bad_context_cast(const std::string& msg_) : msg(msg_) {}
  /// Destructor
  virtual ~bad_context_cast() throw() {}
  
  /// Return the exception description
  virtual const char * what() const throw()
    { return msg.c_str(); }
  
protected:
  /// The exception description
  const std::string msg;
};

namespace legacy {

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
 *  casting functionality through two "new" casts:
 *    - lexical_cast
 *    - context_cast
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
 *
 *  <B>Context casts</B> support modeling extensions to the cast system.
 *  It is often desirable to convert from one variable or problem space
 *  to another - that is, to switch contexts.  A classic example is
 *  switching between a problem domain and a sub-problem domain.  The
 *  variable types in the two domains may be the same
 *  (e.g. vector<double>), but the variables themselves have separate
 *  and distinct meanings.  The context_cast functionality provides a
 *  standardized method for leveraging the lexical_cast infrastructure
 *  to also facilitate transparent conversions between spaces.  In this
 *  sense, a context_cast will typically first use a lexical_cast to
 *  convert the input type provided into the type necessary for the
 *  context_cast, then apply the context_cast function, and finally
 *  apply another lexical_cast to convert the output into the desired
 *  data type.  
 *
 *  Like lexical casts, context casts can be automatically chained -
 *  that is, if there exist context casts for "A" -> "B" and "B" -> "C",
 *  then you can usually cast directly from "A" -> "C".  However, unlike
 *  lexical casts, the segments that compose the context cast path need
 *  not be continuous: the TypeManager will transparently perform
 *  additional lexical casts as needed (e.g. if the "B" produced from
 *  the first cast and the "B" needed for the second cast are of
 *  different types, the context_cast() call will automatically perform
 *  the necessary lexical_cast() call).  In addition, you may specify
 *  multiple casting functions (with different input / output types) for
 *  any pair of input & output context IDs.
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
    typedef long int context_id_type;

    ///
    Type_Manager(bool duplicate_singleton = true);

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
        else if ( getMainType(d.type()) == TypeInfo::UntypedAnyContainer )
          { return lexical_cast( src, d, d.type(), force_exact); }
        else if ( getMainType(src.type()) == TypeInfo::UntypedAnyContainer )
          { return lexical_cast
                ( src, d, src.template expose<utilib::UntypedAnyContainer>()
                  .m_data.type(), force_exact ); }
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
    int register_lexical_cast( const std::type_info& fromType, 
                               const std::type_info& toType, 
                               const fptr_cast fcn,
                               const bool exact = true );

    /// Remove a lexical cast from the cast table
    int clear_lexical_cast( const std::type_info& fromType, 
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


    // NB: context_cast uses "const Any& src" instead of the "Any src"
    // that lexical_cast uses so that the overloads can all be inlined
    // without having to formally #include the Any class
    // definition... oh well... the "convenience" template functions now
    // require the Any class definition to be included...  nevermind.
    // We will now reverse ourselves and use "const Any src" so that the
    // compiler can automagically (implicitly) convert from a hard
    // source variable to an Any.

    /// Perform a context_cast (context IDs)
    inline int context_cast( const Any src, 
                             Any &dest, 
                             const context_id_type fromContext, 
                             const context_id_type toContext, 
                             const std::type_info& toType = typeid(void) )
      { return context_cast_impl( src, dest, fromContext, toContext, 
                                  m_forceExact, toType != typeid(void), 
                                  toType ); }
    /// Perform a context_cast (context IDs), specifying force_exact
    inline int context_cast( const Any src, 
                             Any &dest, 
                             const context_id_type fromContext, 
                             const context_id_type toContext, 
                             const bool force_exact,
                             const std::type_info& toType = typeid(void) )
      { return context_cast_impl( src, dest, fromContext, toContext, 
                                  force_exact, toType != typeid(void), 
                                  toType ); }
    /// Perform a context_cast (context names)
    inline int context_cast( const Any src, 
                             Any &dest, 
                             const std::string &fromContext, 
                             const std::string &toContext, 
                             const std::type_info& toType = typeid(void) )
      { return context_cast_impl( src, dest, 
                                  contextID(fromContext), contextID(toContext),
                                  m_forceExact, toType != typeid(void), 
                                  toType ); }
    /// Perform a context_cast (context names), specifying force_exact
    inline int context_cast( const Any src, 
                             Any &dest, 
                             const std::string &fromContext, 
                             const std::string &toContext, 
                             const bool force_exact,
                             const std::type_info& toType = typeid(void) )
      { return context_cast_impl( src, dest, 
                                  contextID(fromContext), contextID(toContext),
                                  force_exact, toType != typeid(void), 
                                  toType ); }

    /// Convenience function for those too lazy to declare local Any's ;-)
    template<typename TYPE>
    inline int context_cast( const Any src, TYPE &dest,
                             const context_id_type fromContext, 
                             const context_id_type toContext, 
                             const bool force_exact )
      {
      Any tmp(dest, true, true);
      return context_cast_impl( src, tmp, fromContext, toContext, 
                                force_exact, true, typeid(TYPE) );
      }
    /// Convenience function for those too lazy to declare local Any's ;-)
    template<typename TYPE>
    inline int context_cast( const Any src, TYPE &dest,
                             const context_id_type fromContext, 
                             const context_id_type toContext )
      { return context_cast<TYPE>( src, dest, fromContext, toContext, 
                                   m_forceExact, true, typeid(TYPE) ); }
    /// Convenience function for those too lazy to declare local Any's ;-)
    template<typename TYPE>
    inline int context_cast( const Any src, TYPE &dest,
                             const std::string fromContext, 
                             const std::string toContext, 
                             const bool force_exact )
      { return context_cast<TYPE>( src, dest, contextID(fromContext), 
                                   contextID(toContext),
                                   force_exact ); }
    /// Convenience function for those too lazy to declare local Any's ;-)
    template<typename TYPE>
    inline int context_cast( const Any src, TYPE &dest,
                             const std::string fromContext, 
                             const std::string toContext )
      { return context_cast<TYPE>( src, dest, fromContext, toContext, 
                                   m_forceExact ); }


    /// Get the contextID for a given name
    context_id_type contextID(const std::string &name) const;
    /// Register a new context name (generates new contextID)
    context_id_type register_context( const std::string &name );
    /// Register a new function for converting between from & to context IDs
    int register_context_cast( const context_id_type fromContext,
                               const std::type_info& fromType, 
                               const context_id_type toContext,
                               const std::type_info& toType, 
                               const fptr_cast fcn,
                               const bool exact = true );
    /// Register a new function for converting between from & to context names
    int register_context_cast( const std::string& fromContext,
                               const std::type_info& fromType, 
                               const std::string& toContext,
                               const std::type_info& toType, 
                               const fptr_cast fcn,
                               const bool exact = true )
      { return register_context_cast(contextID(fromContext), fromType,
                                     contextID(toContext), toType, fcn, 
                                     exact); }

    /// Print a (pretty) pseudo-table showing all valid context_casts
    void printContextCastingTable(std::ostream &out = std::cerr);

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

  private:
    typedef const std::type_info*   type_t;
    typedef std::list<fptr_cast>    fcnList_t;

    /// A simple structure for storing lexical_cast function pointers
    struct lCast_t {
      lCast_t() : fcn(NULL), exact(true) {}
      lCast_t(const fptr_cast Fcn, const bool Exact)
        : fcn(Fcn),
          exact(Exact)
        {}

      fptr_cast  fcn;
      bool       exact;
      };
    typedef std::map<type_t, lCast_t>     lCastMap_t;
    typedef std::map<type_t, lCastMap_t>  lCastMatrix_t;


    /// A simple structure for storing context_cast function pointers
    struct cCast_t {
      cCast_t() : in(NULL), out(NULL), fcn(NULL), exact(true) {}
      cCast_t(const type_t In, const type_t Out, 
              const fptr_cast Fcn, const bool Exact)
        : in(In), 
          out(Out),
          fcn(Fcn),
          exact(Exact)
        {}

      type_t     in;
      type_t     out;
      fptr_cast  fcn;
      bool       exact;
      };
    typedef std::list<cCast_t>           cCastList_t;
    typedef cCastList_t::iterator        cCastRef_t;
    typedef std::list<cCastRef_t>        cCastRefList_t;
    typedef std::map<context_id_type, cCastList_t>   cCastMap_t;
    typedef std::map<context_id_type, cCastMap_t>    cCastMatrix_t;


    /// A lexical cast chain: the list of casts to get from one type to another
    struct lCastChain_t {
      lCastChain_t() : size(0), exact(true) {}
      lCastChain_t(const lCastChain_t& rhs, const lCast_t& cast) 
        : size(rhs.size + 1),
          chain(rhs.chain),
          exact(rhs.exact && cast.exact)
        { chain.push_back(cast.fcn); }
      
      size_t     size;
      fcnList_t  chain;
      bool       exact;
      };
    typedef std::list<lCastChain_t>::iterator  lChainRef_t;

    /**\brief A context cast chain: the context cast sequence to get
     *  from one context to another (intermediate lexical_casts may be
     *  necessary) */
    struct cCastChain_t {
      cCastChain_t() : size(0), exact(true) {}
      cCastChain_t(const cCastChain_t& rhs, const cCastRef_t& cast,
                   const size_t lexSize = 0, const bool lexExact = true) 
        : size(rhs.size + 1 + lexSize),
          chain(rhs.chain),
          exact(rhs.exact && cast->exact && lexExact)
        { chain.push_back(cast); }
      
      size_t          size;
      cCastRefList_t  chain;
      bool            exact;
      };
    typedef std::list<cCastChain_t>::iterator  cChainRef_t;
    typedef std::list<cChainRef_t>             cChainRefList_t;


    struct lNode_t {
      lNode_t()                   : tail(NULL) {}
      lNode_t(const type_t& Tail) : tail(Tail) {}
      lNode_t(const type_t& Tail, const lCastChain_t& Chain) 
        : tail(Tail),
          chain(Chain)
        {}
      
      type_t       tail;
      lCastChain_t chain;
      };

    struct context_t {
      context_t() : context(0), type(NULL) {}
      context_t(context_id_type Context, type_t Type) 
        : context(Context), type(Type)
        {}
      bool operator<(const context_t& rhs) const
        { return ( context == rhs.context 
                   ? type < rhs.type 
                   : context < rhs.context ); }

      context_id_type     context;
      type_t  type;
      };
    struct cNode_t {
      cNode_t()                      : tail() {}
      cNode_t(const context_t& Tail) : tail(Tail) {}
      cNode_t(const context_t& Tail, const cCastChain_t& Chain) 
        : tail(Tail),
          chain(Chain)
        {}
      
      context_t     tail;
      cCastChain_t  chain;
      };


    typedef std::map<type_t, lChainRef_t>  lexMap_t;
    typedef std::map<type_t, lexMap_t>     lexMatrix_t;

    typedef std::map<context_id_type, cChainRefList_t> ctxMap_t;
    typedef std::map<context_id_type, ctxMap_t>        ctxMatrix_t;



    /// mapping of all known types to the "official" type_info for that type
    std::map<type_t, type_t>       m_equivalentTypes;
    /// mapping of mangled type name to "official" type_info for that type
    std::map<std::string, type_t>  m_typeByName;
    /// map of context names to context id's
    std::map<std::string, context_id_type>     m_contextNames;

    /// all known lexical casting functions
    lCastMatrix_t           m_lexCasts;
    /// all known context casting functions
    cCastMatrix_t           m_ctxCasts;      

    std::list<lCastChain_t> m_lexCastChains; ///< computed lexical cast chains
    lexMatrix_t             m_fwdLexCast;    ///< lexical chains sorted by FROM
    lexMatrix_t             m_revLexCast;    ///< lexical chains sorted by TO

    std::list<cCastChain_t> m_ctxCastChains; ///< computed context cast chains
    ctxMatrix_t             m_fwdCtxCast;    ///< context chains sorted by FROM

    int m_lastError;  ///< last non-zero return code generated

    bool m_lexTableRebuildNeeded; ///< indicates out-of-date lexical_cast table
    bool m_ctxTableRebuildNeeded; ///< indicates out-of-date context_cast table
 
    bool m_throwErrors;    ///< if true, errors will generate exceptions
    bool m_throwWarnings;  ///< if true, warnings will generate exceptions
    bool m_forceExact;     ///< default value for cast's force_exact parameter

    /// Perform a context_cast, possibly returning the result as a toType
    int context_cast_impl( const Any &src, 
                           Any &dest, 
                           const context_id_type fromContext, 
                           const context_id_type toContext, 
                           const bool force_exact,
                           const bool userdefType,
                           const std::type_info& toType );
    /// Get standardized (unique) type_info pointer (see detailed note)
    type_t getMainType(const std::type_info& type);
    /// Perform repeated BFS to generate the complete lexical_cast table
    void generateLexicalCastingTable();
    /// Perform repeated BFS to generate the complete context_cast table
    void generateContextCastingTable();
    /// Private implementation that assumes type* returned by getMainType()
    bool lexical_castable( const std::type_info* srcType, 
                           const std::type_info* destType,
                           bool &isExact, size_t &pathlength );
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

} // namespace utilib::legacy::LexicalCasts

} // namespace utilib::legacy

} // namespace utilib

#endif // utilib_TypeManager_old_h
